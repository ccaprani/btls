# Design Review — 待讨论事项

在推进主线（并行切块+合并）过程中发现的代码设计问题。**均未改动**——逐项与
用户讨论后再决定是否处理。按发现时间排列。

## DR-1: FlowData 小时分箱在静默小时会错位（疑似 C++ bug）

[VehicleBuffer.cpp:108-110](../cpp/src/VehicleBuffer.cpp#L108-L110)：
`updateFlowData` 用 `if (curRelTime > m_CurHour*3600) flushFlowData();`
——是 `if` 不是 `while`。如果某个小时完全没有车辆到达，`m_CurHour` 只会
追赶 1 个小时，导致后续车辆被计入滞后的小时行，FlowData 的 Hour 列与真实
小时错位。流量饱和场景不受影响（每小时都有车），但低流量/夜间空窗会触发。
建议：改为 `while` 循环补齐空小时行。影响输出数值（修复=行为变化），需确认。

## DR-2: C++ 无条件向 stdout 打印

跑完模拟后 stdout 出现 "Bridge 30 m: Flushing AllEvents buffer: ..." 等
（来源 EventManager/Bridge 的 `std::cout`）。库代码不应无条件打印；多进程
并行时 30 个 worker 的输出会交错刷屏。建议：加 verbosity 开关或经由
Python logging 路由。

## DR-3: `_OutputManager` 按目录 glob 重建状态，且依赖 `os.chdir`

[simulation.py:323-325](../py/pybtls/simulation.py#L323-L325) 每个 sim 用
`os.chdir` 切换工作目录来控制 C++ 写文件的位置——进程级全局状态，与
多线程不兼容，也是 `multiprocessing spawn` 之外无法并行的根因之一。
C++ 侧若能接受输出目录参数（或 Python 侧传绝对路径前缀），可去掉 chdir。

## DR-4: 输出持久化用 pickle（用户已点名质疑）

`pb.save_output(...)` 将 `_OutputManager` pickle 到 .pkl。问题：
(a) pickle 跨版本/跨环境脆弱，pandas/pybtls 升级即可能读不回；
(b) `_OutputManager` 本质是「路径集合 + 配置」，pickle 的是路径快照，
目录挪动后失效（虽有 relocate() 补救）；
(c) 真正的数据仍在 txt 文件里，pkl 只是壳。
候选方向：元数据走 JSON/YAML（人类可读、版本稳健）；若要持久化解析后的
DataFrame，用 parquet（跨语言、压缩、类型安全）。待与用户讨论取舍。

## DR-5: Python wrapper 与 C++ 状态重复（用户已点名质疑）

例：`OutputConfig` 在 Python 侧持有 `_ConfigDataCore` 的镜像设置方法；
`Bridge`/`TrafficGenerator` 等包装类同时在 Python 和 C++ 各存一份参数。
风险：两边可能漂移；pickle/spawn 时要靠 `__getstate__` 同步。
方向：Python 层只做「构建器+校验」，单一事实源放 C++（或反之），减少镜像。
范围大，需单独立项讨论。

## DR-6: read 层细节问题（小）

- `read_POT_S` 读完后无条件重写 Peak Index 为 1..N——掩盖了文件原始索引
  （也说明该索引本身无信息量，C++ 写它的意义存疑）。
- `read_E_CS` 直接丢弃 9 列之后的 truck-presence 数据（注释说易误导），
  数据写了又不读，写入本身是否还有必要？
- 多个 read 函数用 `sep="[\s\t]+"` + `engine="python"`，大文件（TH）解析
  慢；C++ 输出本是固定宽度，可用 `delim_whitespace`/`sep="\s+"` + C engine。
- `flushFlowData`/`m_FirstHour` 语义：FlowData 的 Hour 从首车所在小时起算
  （`m_FirstHour`），合并时按「绝对小时」偏移需注意首小时为空的边角。

## DR-8: AllEvents/POT 时间列用 C++ 默认 6 位有效数字写盘（长模拟精度劣化）

[EventBuffer.cpp:80](../cpp/src/EventBuffer.cpp#L80) `m_OutFile << Ev.getStartTime()`
——无 `setprecision`，默认 6 位有效数字。t≈86400 时只剩 1 位小数；
**100 年模拟 t≈3.15e9 时精度只剩 ±1000 秒**，AllEvents 的事件时间在长
模拟下基本不可用（纯串行跑也一样）。对比：Fatigue buffer 用
`fixed << setprecision(2)`（[EventBuffer.cpp:107](../cpp/src/EventBuffer.cpp#L107)）。
建议：统一为 `fixed << setprecision(2~3)`。改动会轻微改变输出文件格式
（数值更精确），需确认。split-replay 等价性测试因此被迫用 rtol=1e-5。

## DR-9: 每次模拟丢失最后一个统计区间（C++ bug，**已修复**）

[StatsManager.cpp:63](../cpp/src/StatsManager.cpp#L63) 原 `CheckBuffer(true)`
（Finish 路径）先 `WriteBuffer()` 再把当前区间 push 进缓冲——最后一个
区间永远写不出去。实测：48h 模拟的 SS_S 只有 47 行。**已在本分支修复**
（forced 路径先 push 再写）：这是数据丢失型缺陷，且阻塞 chunk 合并精确性
（N 个 chunk 丢 N 个区间）。注意：修复后所有模拟的 SS_S 都会多出最后
一行（即从前丢失的那行）——输出行为变化，请知悉。split-replay 等价性
测试可证明修复的正确性。

## DR-10: 静默区间导致 SS_S 区间错位（同 DR-1 类，未修复）

[StatsManager.cpp:46-47](../cpp/src/StatsManager.cpp#L46-L47)：区间翻转
用 `if` 只补一个区间。若某区间完全无事件（低流量/夜间），下一个事件会
被归入滞后的区间，后续区间 ID 与时间错位。饱和车流不触发。与 DR-1
（FlowData 同模式）一起讨论后统一修复。

## DR-7: 测试遗留的 stray print（极小）

测试套件结束后 stdout 出现 C++ 缓冲 flush 消息（与 DR-2 同源），且
`test_sim_run.py` 的 try/except+pytest.fail 模式吞掉了 traceback 细节，
直接裸调用断言更可调试。
