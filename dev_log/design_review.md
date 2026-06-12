# Design Review — 发现的问题与处理状态

在推进主线（并行切块+合并）过程中发现的问题。**bug 类已经用户授权修复**
（2026-06-12「把发现的bug都修了」）；设计类仍待逐项讨论。

状态总览：
- **已修复**：DR-1、DR-2、DR-4、DR-6、DR-7、DR-8、DR-9、DR-10、DR-11、
  DR-12、DR-13、DR-14 + 末尾静默块（共 13 项）
- **已出方案、待立项**：DR-3、DR-5（同一重构的两面，建议独立分支，见下）

## DR-13: 雨流计数依赖 IO 缓冲尺寸（C++ bug，**已修复**）

原 `CRainflow::calcCycles(false)` 每次 flush 把未闭合点折成半循环、只带
2 个反转点过界——直方图随 buffer_size 变化、跨 flush 大循环被切碎。
已重写为真四点法 + residual 跨 flush 携带 + 末端 ASTM 闭合：确定性、
与教科书 ASTM E1049-85 一致、且满足 chunk 拼接结合律。详见
[03_phase2.md](03_phase2.md)。**行为变化**：新 FR 直方图与旧值有少量
bin 级差异（新值是正确的整列计数）。另修 extractReversals 对 0/1 点
序列的越界 UB。

## DR-1: FlowData 小时分箱在静默小时会错位（C++ bug，**已修复**）

[VehicleBuffer.cpp:108-110](../cpp/src/VehicleBuffer.cpp#L108-L110)：
`updateFlowData` 用 `if` 不是 `while`。某小时完全无车时 `m_CurHour` 只追
1 小时，后续车辆计入滞后小时行。**已改 `while` 补齐空小时行。**
同款模式同时存在于 Stats/POT/BM 三个 manager（见 DR-10），已一并修复。

## DR-2: C++ 无条件向 stdout 打印（**已修复**）

例行 flush 消息（VehicleBuffer/EventBuffer）现由全局开关
`btls::console_output`（[ConsoleOutput.h](../cpp/include/ConsoleOutput.h)）
控制，**默认关闭**；Python 经 `libbtls.set_console_output(True)` 开启。
`***` 类警告/错误不受影响，始终打印。顺带修复了空缓冲 flush 消息对
`m_vEvents[-1]` 的越界读。**行为变化**：默认不再打印 flush 进度。

## DR-3: 输出目录依赖 `os.chdir`（已出方案，待立项）

进程级全局状态，多线程不兼容。**方案**：`ConfigDataCore.Output` 增加
`OUTPUT_DIR` 字段，`COutputManagerBase` 及其余 ~8 处文件打开点统一加
前缀（OutputManagerBase/EventBuffer/VehicleBuffer/TH/FlowData/Rainflow/
residual），Python 侧去掉 `_single_*_sim` 与 `garage/write.py` 的
chdir。机械性改动约一天，触面广（每个写文件点），建议与 DR-5 同分支做、
配合现有 split-replay 金标准测试验证输出不变。本轮未动以免膨胀当前分支。

## DR-4: 输出持久化用 pickle（**已修复**）

[output_pickle.py](../py/pybtls/utils/output_pickle.py) 重写：
`save_output` 现写**人类可读的 JSON 清单**（格式版本号 + 路径 + 配置
状态字典；数据本体始终在输出 txt 文件里），跨版本稳健；`load_output`
读 JSON，**旧 .pkl 仍可加载**（带弃用警告）。同时修复了 isinstance
检查拒收 `_ChunkedOutputManager` 的问题（chunked 结果现在可保存/恢复，
含 master_seed）。如需持久化解析后的 DataFrame（parquet），可后续按需加。

## DR-5: Python wrapper 与 C++ 状态重复（已出方案，待立项）

核实后修正认知：`OutputConfig` 其实**直接继承** `_ConfigDataCore`（无
镜像，没问题）；真正的重复在 `Bridge`/`TrafficGenerator`/`TrafficLoader`
——Python 持有参数副本 + 手写 `__getstate__/__setstate__`，spawn 时靠
其同步，新增字段时两处易漂移（本轮 WRITE_RAINFLOW_RESIDUALS 就必须同时
改三处 pickle 代码）。**方案**：Python 层定型为「声明式参数容器」
（dataclass 化、自动派生 state），C++ 对象仅在 worker 内由参数即时构建
（现架构已大体如此，需收尾归一）。触面是全部 wrapper 类，建议与 DR-3
同分支独立立项，行为不变、以现有 69 个测试为门禁。

## DR-6: read 层细节问题（**已修复（解析部分）**）

- `sep="[\s\t]+"` + python engine → `sep=r"\s+"` + C engine
  （all_events/time_history/POT_counter）：同语义，大文件（TH）解析显著
  提速。
- `read_POT_S` 的 Peak Index 重写、truck-presence 列丢弃：维持现状
  （前者是文件索引本身无信息量的合理补救，后者为有意为之并有注释）。

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

## DR-14: 小时流量为 0 会永久杀死车道（**已修复**）

[FlowGenerator.cpp](../cpp/src/FlowGenerator.cpp)
`skipZeroFlowBlocks()`：当前块总流量为 0 时，跳到下一个有流量的块起点
重启到达过程（一个周期内全零则警告并维持旧行为）。语义统一为
「零流量块 = 该块无车」，对所有 headway 模型一致。回归测试
`test_zero_flow_hours_recover` 验证静默窗保持静默、两天的恢复窗均有
事件。**行为变化**：从前零流量小时后车道永久静默（明显错误）。

## DR-7: 测试风格（**已修复**）

`test_sim_run.py` 重写：去掉全部 try/except+pytest.fail（失败现在直接
给出完整 traceback），输出读取从「逐个访问」改为断言非空，save/load
round-trip 改用 JSON 清单。stray print 随 DR-2 一并消失（套件耗时
17s → 10.6s）。
