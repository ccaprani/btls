# Design Review — 发现的问题与处理状态

在推进主线（并行切块+合并）过程中发现的问题。**bug 类已经用户授权修复**
（2026-06-12「把发现的bug都修了」）；设计类仍待逐项讨论。

状态总览：
- **已修复（bug）**：DR-1、DR-8、DR-9、DR-10、DR-11、DR-12
- **待讨论（设计）**：DR-2、DR-3、DR-4、DR-5、DR-6、DR-7

## DR-1: FlowData 小时分箱在静默小时会错位（C++ bug，**已修复**）

[VehicleBuffer.cpp:108-110](../cpp/src/VehicleBuffer.cpp#L108-L110)：
`updateFlowData` 用 `if` 不是 `while`。某小时完全无车时 `m_CurHour` 只追
1 小时，后续车辆计入滞后小时行。**已改 `while` 补齐空小时行。**
同款模式同时存在于 Stats/POT/BM 三个 manager（见 DR-10），已一并修复。

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

## DR-8: AllEvents 时间列用默认 6 位有效数字写盘（**已修复**）

[EventBuffer.cpp:80](../cpp/src/EventBuffer.cpp#L80) 原 `m_OutFile <<
Ev.getStartTime()` 无 `setprecision`——**100 年模拟 t≈3.15e9 时只剩
±1000 秒精度**。已改 `fixed << setprecision(3)`（与 TH 文件一致）。
已核查其余写出器：TH=fixed(3)、BM_V/POT_V=fixed(1)、POT_S=fixed(1)、
Fatigue=fixed(2)，均为 fixed 模式（精度不随量级劣化），不改。

## DR-11: read_E_CS / read_E_IS 统计列名整体错位（Python bug，**已修复**）

SS 文件真实列序（`CEventStatistics::outputString`）：`#Events, #Ev Vehs,
#Ev Trucks, Min, Max, Mean, StdDev, Variance, Skew, Kurt`。原 read 函数
漏了 Min/Max 两列，导致从第 5 列起全部错位：用户拿到的 "Mean" 实为 Min、
"Std Dev" 实为 Max、"Variance" 实为 Mean……真正的 Skew/Kurt 被丢弃。
已修正列名并保留 Min/Max 列（DataFrame 多两列，属 schema 修复）。
全仓无下游依赖旧错位列名。

## DR-12: 零事件区间的统计 finalize 产生 NaN/size_t 下溢（C++ bug，**已修复**）

[EventStatistics.cpp:71](../cpp/src/EventStatistics.cpp#L71)：N=0 时
`m_M2/(m_N-1)` 触发 size_t 下溢，偏度/峰度 0/0 → NaN 写盘（Windows 下
为 "-nan(ind)"，会破坏读取）。N<2 或 M2=0 时各阶矩现在输出 0.0。
该路径在 DR-9/DR-10 修复后（空区间会被写出）才可触达。

## DR-9: 每次模拟丢失最后一个统计区间（C++ bug，**已修复**）

[StatsManager.cpp:63](../cpp/src/StatsManager.cpp#L63) 原 `CheckBuffer(true)`
（Finish 路径）先 `WriteBuffer()` 再把当前区间 push 进缓冲——最后一个
区间永远写不出去。实测：48h 模拟的 SS_S 只有 47 行。**已在本分支修复**
（forced 路径先 push 再写）：这是数据丢失型缺陷，且阻塞 chunk 合并精确性
（N 个 chunk 丢 N 个区间）。注意：修复后所有模拟的 SS_S 都会多出最后
一行（即从前丢失的那行）——输出行为变化，请知悉。split-replay 等价性
测试可证明修复的正确性。

## DR-10: 静默块导致 Stats/POT/BM 块错位（同 DR-1 类，**已修复**）

同款 `if` 只补一块的模式存在于三处，已全部改 `while`：
- [StatsManager.cpp:46](../cpp/src/StatsManager.cpp#L46)（SS_S 区间）
- [POTManager.cpp:49](../cpp/src/POTManager.cpp#L49)（POT counter 块）
- [BlockMaxManager.cpp:67](../cpp/src/BlockMaxManager.cpp#L67)（BM 块）

修复后语义：无事件的块/区间会以零值行写出，块索引始终与时间对齐。
**行为变化提示**：低流量场景的输出会比从前多出空块行（从前是错位+缺行）。

## DR-7: 测试遗留的 stray print（极小）

测试套件结束后 stdout 出现 C++ 缓冲 flush 消息（与 DR-2 同源），且
`test_sim_run.py` 的 try/except+pytest.fail 模式吞掉了 traceback 细节，
直接裸调用断言更可调试。
