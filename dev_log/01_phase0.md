# Phase 0 — 地基与验证骨架

状态：**完成**（2026-06-12，全套件 52 passed）

## 已完成

### 1. 合并原语 + 注册表（`py/pybtls/output/_merge.py`）

- `MergeSpec` dataclass：每种输出声明 `category` + 列角色（时间列 fnmatch
  模式、连续计数列、重编号列、bin 求和键/值列）。
- `MERGE_REGISTRY`：覆盖 `_OutputManager` 全部 14 个输出 key。
- 已实现原语：`merge_concat`（拼接+时间偏移+计数偏移+重编号）、
  `merge_bin_sum`（按键求和）。
- Phase 2 待实现：`moment_merge`（SS_C 全程矩）、`vehicles_concat`（车辆
  记录文件）。
- 单元测试 `tests/test_merge_primitives.py`（11 个，合成数据，全过）。

### 2. 格式语义核实（决定 registry 声明的事实）

- FlowData `Hour`：从模拟起点单调递增（VehicleBuffer.cpp `m_CurHour`）
  → 合并 = 拼接+小时偏移（不是按 bin 求和）。
- SS_S：`Time = WRITE_SS_INTERVAL_SIZE × ID`（StatsManager.cpp
  WriteSummaryFiles）→ ID 偏移 + Time 当作时间列偏移即自洽；区间不跨
  chunk（边界对齐时）→ **SS_S 拼接即精确，无需矩合并**。
- 需要矩合并的只有 SS_C（全程累计）。且 SS_C 文件含 N/Mean/Variance/
  Skewness/Kurtosis，数学上可反解 M2/M3/M4 —— Phase 2 可能不需要改 C++。
- 默认尺寸：BM 块=1 天、POT counter=天级、区间=3600s → 天对齐切块对默认
  配置精确。自动切块 API 须校验整除性。

### 3. 开发环境

- pybtls 改装为 editable install（pybtls-dev env，build isolation）。
- 既有测试基线：40 passed（~19s）。

### 4. split-replay 等价性验证（`tests/test_parallel_equivalence.py`）— 完成

策略：录 2 天车流（午夜±15s 挖保护空窗）→ 完整回放 vs 两半分别回放后
merge → 应精确相等（紧容差）。回放无 RNG，比较的是引擎确定性语义。

**首轮失败 → 根因定位 → 已修复（测试侧 bug，非 pybtls bug）：**
合并结果比完整回放多 ~3% 事件（1204 vs 1171），且 full 回放的 day-2
事件时间「回绕」到当天内。排查路径：分割文件行数守恒 ✓ → Vehicle
setTime/getTime 日历往返 ✓ → MON 读写链（LaneFileTraffic /
VehicleTrafficFile / VehicleBuffer 均不动时间）✓ → 双路径读入实验发现
**重写后的 full.txt 日期字段全为 1**。根因：`_split_traffic` 中
`half_b` 与 `kept` 共享同一批 Vehicle 对象（列表推导不拷贝），先
`set_time(-86400)` 再写 full.txt 导致 full.txt 的 day-2 车辆被改写为
day-1。修复：先写 full/half_a，再平移并写 half_b。

修复别名 bug 后的三轮收敛：
1. 行数/形状全部一致，剩格式量化差（0.01 定点量子 + AllEvents 的 6 位
   有效数字浮动量子）→ 容差定为 rtol=1e-5, atol=0.011（仅覆盖文本格式
   量子，仍能抓住任何真实合并错误）。
2. SS_S 形状不匹配 → 发现并修复 C++「末区间丢失」bug（DR-9，
   StatsManager.cpp CheckBuffer 强制路径先写后存）。
3. PT_V 的 Index 是「每事件多行（每效应一行）」的连续事件计数器 →
   registry 从 renumber 改为 offset 类。

最终：**等价性测试 12 项全过**——11 种 concat 类输出与连续单跑精确一致
（至格式量子），rainflow bin 求和在宽松界内（Phase 2 做精确）。全套件
52 passed（40 既有 + 12 新增）。

调试中顺带确认的工程事实：
- AllEvents/事件文件的时间列在引擎正常运行时**是绝对时间**（事件时间
  来自 `vehicle.get_time()`，全链无 mod 86400）——registry 的时间偏移
  声明正确。
- 用 multiprocessing spawn 的调试脚本必须包 `if __name__ == "__main__"`
  guard，否则 worker 重新 import `__main__` 会递归执行模块顶层的模拟
  （第一版调试脚本因此跑飞，已杀）。
- MON 往返量化：速度 1 km/h、秒 1 ms、轴重 kg 级取整——录制→回放的
  量化对三个回放一致，不影响等价性验证。

## 设计决定记录

- **合并层级**：在「读取层」合并（chunked manager 读各 chunk 的
  DataFrame 后按 registry 变换合并），不在文件层合并 → 避免为 14 种
  古怪的定宽文本格式写往返写入器。每 chunk 的原始文件保留在磁盘。
- **精确性条件**：chunk 时长必须是 BM 块/POT counter 块/SS 区间的公倍数
  （天对齐 + 默认配置自动满足）；自动切块 API 负责校验并报错。
- **rainflow v1**：bin 求和（边界残余可能差几个循环），Phase 2 用残余
  反转拼接做到精确。
