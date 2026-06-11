# Phase 1 — 自动切块 API 与全输出合并

状态：**完成**（2026-06-12）

## 用户可见变化

一次 100 年模拟从单核 8–12 小时 → 自动铺满多核：

```python
sim.add_sim(
    bridge=bridge, traffic=traffic_gen,
    no_day=36500, output_config=config,
    seed=42,          # master seed；chunk i 用 seed+i
    no_chunk=30,      # 新参数：切成 30 个独立天块
    tag="100yr",
)
sim.run(no_core=30)
out = sim.get_output()["100yr"]   # 单个合并视图，接口与 _OutputManager 一致
out.read_data("BM_summary")       # 与连续单跑同构的 DataFrame
```

- 不切块（`no_chunk=None`）与多个独立模拟并发的行为完全不变。
- `seed=None` 时自动抽取 master seed，可经 `out.master_seed` 取回复现。

## 实现

1. **`merge_cumulative_stats`**（[_merge.py](../py/pybtls/output/_merge.py)）：
   SS_C 合并。从每 chunk 的 (N, Mean, Var, Skew, Kurt) **反解原始矩
   M2/M3/M4**（精确逆 `CEventStatistics::finalize`），用 Chan 并行公式
   合并，再 finalize 回去。数学精确；唯一误差源是输入文件的 0.01 定点
   量化（等价性测试 rtol=1e-3/atol=0.03 内通过）。**Phase 2 原计划的
   「C++ 吐原始矩」由此免除**；若将来要消除量化误差，只需提高 SS_C
   写盘精度（小改动，列入 Phase 2 可选项）。
2. **`merge_vehicle_traffic`**：车辆记录文件合并——模拟日历
   （25 天/月 × 10 月/年，ConfigData::Time 默认）字段平移 + Head 偏移。
3. **`_ChunkedOutputManager`**（[chunked_manager.py](../py/pybtls/output/chunked_manager.py)）：
   读取时合并的视图，镜像 `_OutputManager` 接口（get_summary /
   read_data / relocate / tag），另提供 `read_chunk_data()`（调试用）、
   `master_seed`、`no_chunk`、`chunks`。
4. **`Simulation`**（[simulation.py](../py/pybtls/simulation.py)）：
   - `add_sim(no_chunk=N)`：校验（TrafficGenerator-only、no_day 整除、
     chunk 长度对齐 BM 块/POT counter 块/SS 区间）后展开为 N 个
     `tag/chunk_iii` 子任务（嵌套输出目录），seed = master+i。
   - `run()` 末尾 `_reduce_chunk_groups()`：chunk 输出归并为单个
     `_ChunkedOutputManager` 挂在父 tag 下。

## 测试

- [test_auto_chunk.py](../tests/test_auto_chunk.py)（5 个）：端到端
  （13 种输出全部可读且合并）、时间线连续性（块/区间索引跨界连续、
  事件时间单调且覆盖末日）、同 seed 可复现（逐值相等）、5 种校验报错。
- [test_parallel_equivalence.py](../tests/test_parallel_equivalence.py)
  新增 SS_C 矩合并等价性（vs 连续回放）。
- 全套件通过（见提交信息）。

## 提速基准（2 车道、500 卡车/h/车道、0.1s 步长、BM+POT+Stats+FR 输出）

| 规模 | 单核 | 8 chunk × 8 核 | 提速 | 并行效率 |
|---|---|---|---|---|
| 16 天 | 2.5 s | 0.8 s | 3.1× | 39%（spawn 开销主导） |
| 96 天 | 15.0 s | 2.1 s | 7.1× | 89% |

残差为固定的进程 spawn/import 开销（~0.5s/worker）。真实 100 年规模下
每 chunk 为分钟～小时级，开销占比趋零 → 30 chunk/30 核预期接近 30×，
即 8–12 小时 → **~20–25 分钟**。

## 精确性边界（与用户的约定一致）

- concat/bin-sum/moment 类输出：与连续单跑**等同至文本格式量子**。
- rainflow：v1 为各 chunk 闭合后按 bin 求和，边界残余可能移动少量循环
  （等价性测试界：≤max(4, 1%)）。**Phase 2 做精确残余拼接（需 C++ 吐
  残余反转序列）。**
- 切块本身的统计语义：含义 A（独立随机流，统计等价），已与用户确认。
