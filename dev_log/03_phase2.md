# Phase 2 — Rainflow 精确合并(residue splicing)

状态:**完成**(2026-06-12,全套件 62 passed)

## 结果

最后一个非精确输出(fatigue_rainflow)升级为**精确合并**:
chunked 模拟的雨流直方图 = 连续单跑的结果(等价性测试 rtol=1e-9 通过)。
至此 **13 种输出全部精确合并**(至文本格式量子)。

## 过程中发现并修复的算法问题

### DR-13:雨流结果依赖 IO 缓冲尺寸(已修复)

原 `CRainflow::calcCycles(false)` 在每次缓冲 flush 时把未闭合点链全部
折算成半循环、只携带最后 2 个反转点——跨 flush 的大循环被切碎,
**同一模拟换 buffer_size 会得到不同直方图**。

### 第一版修复尝试的教训:「尾三点」规则不满足拼接结合律

最初仅去掉 S-半循环规则、保留尾三点闭合(`X>=Y` 看最后三点)。
cadence 无关性通过了,但随机序列的拼接恒等式失败(总循环数守恒、
bin 分布重排)。原因:尾三点判据隐式依赖队列前部是否存在点
(`size>3` 条件),分块时局部起点不同导致闭合时机不同、改变后续配对。

### 最终实现:真四点法 + 末端 ASTM 闭合

[Rainflow.cpp](../cpp/src/Rainflow.cpp):检查最后四个点,当
`|x3-x2| <= |x2-x1| && |x3-x2| <= |x4-x3|` 时闭合中间对 (x2,x3) 为全循环;
residual 跨 flush 携带;最终(或合并闭合时)按 ASTM E1049-85 规则 5
把 residual 逐对折半。性质(均有测试):
- 与 ASTM E1049-85 X3.1 教科书算例完全一致;
- 输出与缓冲 cadence 无关;
- **拼接结合**:`closed(S1)+closed(S2)+closure(res1⊕res2) == 一次性整列计数`。

另修复 `extractReversals` 对 0/1 点序列的越界 UB(单时间步事件可触发,
潜在老 bug)。

## 行为变化提示(用户须知)

- 旧实现的雨流输出本身受 buffer 尺寸影响(每 ~10000 事件被强制切割
  一次),新实现是确定性的整列 ASTM 计数。**新旧 FR 直方图会有少量
  bin 级差异,新值更接近理论雨流计数。**

## 机制

- C++:`Fatigue.WRITE_RAINFLOW_RESIDUALS` 配置(默认 false,不影响
  现有用户)。开启后模拟结束时不闭合 residual,写入侧车文件
  `FRR_{length}_{effect}.txt`(首行 decimal+cutoff,随后 %.17g 反转值)。
- `Simulation.add_sim(no_chunk=N)` 自动对 chunk 开启该模式(pickle
  深拷贝 config,不污染用户对象)。
- pybind11 暴露 `_Rainflow`(processData / calcCycles / getRainflowOutput /
  getResiduals),Python 合并时用**同一份 C++ 算法**闭合拼接后的
  residual——零算法复制。
- `merge_rainflow`([_merge.py](../py/pybtls/output/_merge.py)):
  闭合循环 bin 求和 + residual 拼接闭合;无侧车文件时退化为 bin 求和。

## 测试

- [test_rainflow.py](../tests/test_rainflow.py)(4 个):ASTM 教科书值、
  cadence 无关性(2/7/50 段切分)、拼接恒等式(随机序列)、
  cutoff/decimal 生效。
- 等价性测试重构为统一走生产路径(`_ChunkedOutputManager`),
  13 种输出 × 容差矩阵,FR 用 rtol=1e-9(本质精确)。
