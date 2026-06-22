# GPU 引擎的采样计算（原理详解）

> 配套代码：[py/pybtls/gpu/engine.py](py/pybtls/gpu/engine.py)（`prepare_axles` / `compute_from_axles`）。
> 适用：`Simulation.add_sim(..., engine="gpu")`。

---

## 0. 核心思想

CPU 引擎是**串行时间机**：推进时钟、维护"当前在桥车辆"、每步求和（`CBridge::Update`）。

GPU 引擎换了根本视角——荷载效应对荷载是**线性叠加**的：

```
E(t) = Σ_(所有在桥的轴)  w · IL(pos(t))
```

所以不按时间推进，而是把整个仿真**摊平成一大堆相互独立的"采样点"**：每个采样点是一个三元组
`(时间样本索引 sidx, 轴位置 pos, 轴重 w)`。共有 **P 个**（可达数百万）。然后：

1. 对每个点**并行**求 `w·IL(pos)`；
2. 把同一时间样本的所有点**散射累加**到 `E[sidx]`（这一步实现"对在桥的轴求和"）；
3. 对 E 做**并行归约**取 block-max。

GPU 的威力：第 1 步对 P 个点同时算，第 2/3 步并行累加 + 归约。

> 前提：每辆车**所有轴等速**（刚体，同 `speed`，仅 `datum` 按轴距错开）。

---

## 1. 数据形态的四次变换

```
车辆对象        →   每轴数组          →   每采样点(pair)数组       →   E 矩阵(分块)        →  结果
N_veh 个         N_axles 个            P = Σ每轴样本数              [block_len, n_eff]      [n_eff]
(Vehicle)       datum/sign/speed/     sidx/pos/w[+lane/trans/      每块一个                global_max
                weight                 track]                                              = max over blocks
   ↑CPU(getter)    ↑CPU(numpy向量化)      ↑GPU(展开)                 ↑GPU(散射+归约)
```

关键：**主机只到"每轴"规模（N_axles）**；"每采样点"的大数组（P，百万级）**只在 GPU 上存在**
（这是 C4 优化——避免 CPU↔GPU 传大数组）。

---

## 2. 每轴 → 在桥采样窗口（解析判断"上桥/离桥"）

轴位置 `pos(t) = sign · speed · (t − datum)`，在桥 ⟺ `0 ≤ pos ≤ L`。解出时间窗，换算成采样索引区间
（采样点 k 对应时刻 k·ts）：

```python
tlo = where(sign>0, datum, datum-L/speed);  thi = where(sign>0, datum+L/speed, datum)
nlo = clamp(ceil(tlo/ts), 0, n_total-1);    nhi = clamp(floor(thi/ts), 0, n_total-1)
count = nhi - nlo + 1          # 这根轴贡献几个采样点
```

**不需要逐时步维护"桥上有哪些车"**——每根轴**独立地**解析算出自己覆盖哪些时间样本。这是 GPU 能并行的根本。

例：L=20m，speed=10m/s，dir=1，datum=5.0s，ts=0.1s → 窗 [5.0,7.0] → nlo=50, nhi=70 → **count=21**。

---

## 3. ragged 展开（最精妙的一步）

不同轴 count 不同（21,18,25,…），要摊平成长度 P 的扁平数组。用经典"ragged range"向量化：

```python
counts = [21, 18, 25, ...]; P = sum(counts)
off  = arange(P) - repeat_interleave(cumsum(counts)-counts, counts)   # 每轴内部 0..count-1
sidx = repeat_interleave(nlo, counts) + off
```

摊开看：
```
arange(P)                       = [0,1,...,20,  21,...,38,  39,...]
cumsum(counts)-counts           = [0,           21,         39, ...]   # 每轴起始
repeat_interleave(↑, counts)    = [0×21,        21×18,      39×25]
off = arange - ↑                = [0,1,...,20,  0,...,17,   0,...,24]  ← 每轴内部局部计数
sidx = repeat(nlo)+off          = [50..70,      ...,        ...]       ← 全局采样索引
```

`repeat_interleave` 把每轴常量"铺开"到它每个采样点；`arange − 段起点` 造出每轴内部局部偏移。
**全程无 Python 循环、纯向量算子** → GPU 一次算完。

位置与轴重同理：
```python
pos = repeat(sign,counts) * repeat(speed,counts) * (sidx*ts - repeat(datum,counts))
w   = repeat(weight, counts)
m = (pos>=0) & (pos<=L)        # 边界掩码，修整 ceil/floor 舍入
sidx, pos, w = sidx[m], pos[m], w[m]
```

接上例：pos = 10·(k·0.1−5.0) = 0,1,…,20（轴从桥头匀速走到桥尾）。

---

## 4. 影响线求值（per-effect，向量化）

对每个效应 e，在**全部 P 个点的位置上一次性**求 IL 纵坐标：
- **discrete**：`searchsorted` + 线性插值（二分，向量化）
- **built-in**：`builtin_ordinate`（`torch.where` 分段公式，9 个 id）
- **surface**：双线性 + 双轮 `0.5·(IL(y_left)+IL(y_right))`

得 `ordn`（长度 P）。每点贡献 = `w · ordn · 该效应权重`。

---

## 5. scatter-add —— 叠加就发生在这里

```python
E[:, e].index_add_(0, sidx, w * ordn * weight_e)
```

`index_add_` 把每点贡献**累加**到 `E[sidx]`。**两根轴在同一时间样本 k 都在桥上时**，它们 sidx 都=k →
两份贡献都加到 `E[k]` → **自动实现"对在桥所有轴求和"**。

不需要显式判断"此刻桥上有哪几辆车"——散射累加替你做了求和。这就是 superposition 的 GPU 实现：
**不按时间组织"哪些轴在一起"，而让每根轴独立地撒到它覆盖的时间样本上，靠累加自然汇总。**

---

## 6. 分块 tiling + 归约

E 对整个仿真物化会爆显存，故**按时间块（默认每天）**处理：

```python
for b in 每块:
    msk = (sidx 落在本块范围)
    E = zeros(block_len, n_eff)        # 只为本块分配
    for e in 效应: ... index_add_ ...
    bm[b] = E.max(dim=0).values        # GPU 并行归约
    del E                              # 丢弃
global_max = bm.max(over blocks)
```

E 只存"一块"大小 → 显存有界（实测 ~0.8G）。

---

## 7. 并行性总结

| 步骤 | 并行维度 |
|---|---|
| 展开 pairs | P 个点全并行 |
| IL 求值 | P 个点全并行 |
| scatter-add | P 个点并行写入（原子累加处理冲突）|
| block-max | 并行归约 |

**唯一串行**：外层"按块、按效应"的 Python 循环（几十次量级），每次循环内的 kernel 都对**百万点**大并行。
瓶颈是 kernel 启动次数（少），而非数据规模——GPU 擅长的形态。

---

## 8. 与 CPU 引擎的差异（为何不逐位相等）

两个引擎在**不同时间点**采样：CPU 网格从车辆到达时刻起步、事件边界有部分步（非均匀）；GPU 是
t=0 起的均匀网格 k·ts。同一条连续 E(t) 用两套网格取极值会差一点点：
- **光滑 IL（弯矩/面）**：峰附近平坦 → <1%；
- **剪力 IL**：峰在支座**不连续跳变**处 → 2-4%，且不随 ts 收敛。

**给定同一时间 t + 同一轴位置，两者算出的 E 完全相同**（C0 已验证）；差异**纯来自"评估哪些时间点"**。
这是 Phase C 的有意取舍（统计容差换 GPU 并行效率），见 [refactor_convolution_proposal.md](refactor_convolution_proposal.md) §1.1。
