# Phase 4 — 测试补强、文档、构建/CI

状态:**完成(本轮范围)**(2026-06-12,全套件 68 passed)

## 4a. 测试补强(针对本项目暴露的失效模式)

新增 [test_output_integrity.py](../tests/test_output_integrity.py)(6 个):
- 静默块/区间索引完整性(busy + sparse 工况,600s 小区间锻炼空块路径)
  ——回归保护 DR-9/DR-10 修复
- 空区间内容(零事件区间必须存在且各阶矩为 0)——保护 DR-12
- SS_C 与原始事件数据的数值一致性(count/min/max/mean/std/var/skew)
  ——回归保护 DR-11 列错位
- 区间事件数总和 = 累计事件数
- garage 文件往返保真(时间/GVW/轴数/速度)

加上此前各阶段:test_merge_primitives(11)、test_parallel_equivalence
(13,生产路径)、test_auto_chunk(5)、test_rainflow(4)。
**套件从 40 → 68。**

### 测试驱动发现并修复的新缺陷

**模拟末尾静默块丢失**(本阶段修复,commit 484aa26):输出 manager 不知道
模拟结束时间,输出止于最后一个事件——稀疏车流下尾部空区间/块/小时缺失,
且会令 chunk 合并的索引偏移错位。`Bridge::Finish` /
`EventManager::Finish` / `VehicleBuffer::FlushBuffer` 增加带
`sim_end_time` 的重载(Python 传入 `no_day*86400`),BM/POT/Stats 在强制
flush 前滚到末尾。busy 工况输出逐字节不变(golden 哈希验证)。

**DR-14(未修,待讨论)**:小时流量为 0 会令车道永久静默。

## 4b. 文档

- [parallel.rst](../docs/source/parallel.rst) **全部重写**:旧版描述手工
  recipe + 未实现的 warmup buffer(半虚构);新版以 `no_chunk` 真实 API
  为中心——快速上手、统计有效性论证、各输出的精确合并语义、chunk 尺寸
  校验规则、可复现性矩阵、实测性能表。
- [IO_formats.rst](../docs/source/IO_formats.rst):新增 FRR_* 侧车文件
  格式说明。
- 新增/改动 API 的 docstring 完整(add_sim 的 seed/no_chunk、
  set_fatigue_output 的 write_residuals、_ChunkedOutputManager、
  _merge 模块、_Rainflow 绑定、C++ 新增函数的 doxygen 注释)。
- RST 语法经 docutils 校验(sphinx 角色除外)。

### 未尽事项(需用户决定/配合)

- **完整 docs 本地构建**:pybtls-dev env 缺 sphinx/breathe/nbsphinx
  (environment.yml 声明了但未装)。按约不擅自安装——经用户允许
  `pip install -r docs/requirements.txt` 后可本地 `make html` 验证;
  否则由 CI(sphinx-docs.yml)验证。
- **教程「由简到繁」整体重构**(startup/tutorial/notebooks):工作量大且
  需文档构建环境预览,建议作为独立任务在 design review 讨论后推进。

## 4c. 构建/CI

- pyproject 钉死 Release(Phase 3);editable/常规安装均验证可用。
- C++ 改动均为可移植标准写法(overload_cast、(std::max) 防 Windows
  宏、shared_ptr);无新构建依赖;cibuildwheel/ci.yml 无需改动。
- 全套件 68 passed,将由 CI 在 3 OS × 4 Python 上复验。

## 项目总览(Phase 0–4)

| 指标 | 值 |
|---|---|
| 提交 | 7 个(1ea7cf1 … 484aa26 + 本提交) |
| 修复 bug | DR-1,8,9,10,11,12,13 + 末尾静默块(8 项) |
| 新功能 | `add_sim(no_chunk=N)` 全自动并行 + 13 种输出精确合并 |
| 单核性能 | 1.8×(逐字节一致) |
| 测试 | 40 → 68 |
| 待讨论 | DR-2~7、DR-14(design_review.md) |

用户 100 年模拟预期:8–12h → **~10–15 分钟**(30 核 + 单核 1.8×)。
