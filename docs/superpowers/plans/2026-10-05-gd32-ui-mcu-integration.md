# GD32-UI 与 GD32F5HCR-EVAL 集成实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 让 GD32-UI 的八个页面、资源和交互逻辑同时构建到 Windows 模拟器与 GD32F5HCR-EVAL MCU 固件。

**Architecture:** GD32-UI 的 `generated/` 和 `custom/gui_app.c` 作为唯一共享 UI 源；新增 `ui_platform.h` 为时间、毫秒节拍和三个逻辑按键提供最小接口。模拟器与 MCU 各自实现平台接口，并保留各自的 LVGL 显示/输入设备。

**Tech Stack:** C11、CMake、LVGL 8.3.11、FreeRTOS、GD32W51x_F5HC 标准外设库、Windows Win32 Simulator。

**Spec:** `docs/superpowers/specs/2026-10-05-gd32-ui-mcu-integration-design.md`

## Global Constraints

- UI 唯一源码目录是 `C:/Users/86135/Desktop/GD32-UI/generated` 和 `C:/Users/86135/Desktop/GD32-UI/custom`。
- MCU 与 UI 工程保持相邻目录；MCU 侧使用 `GD32_UI_DIR`，默认 `${CMAKE_SOURCE_DIR}/../GD32-UI`。
- 不把 GD32-UI 的 `simulator/` 源码编入 MCU 固件。
- LVGL 版本固定为 8.3.11，颜色深度固定为 RGB565/16-bit，屏幕固定为 320x240。
- MCU 的 `LV_MEM_SIZE` 初始值固定为 `128U * 1024U`。
- 逻辑按键固定为 `UI_KEY_FOCUS_NEXT`、`UI_KEY_ACTIVATE`、`UI_KEY_BACK`。
- GD32F5HCR-EVAL 按键均为低电平有效：`UI_KEY_FOCUS_NEXT=PD2/USER_1`、`UI_KEY_ACTIVATE=PA3/USER_2`、`UI_KEY_BACK=PA2/TAMPER_WAKEUP`。
- GD32-UI 当前不是 Git 仓库；不得为了本计划擅自初始化其 Git 仓库。只有 MCU 工程中的文件按任务提交。
- 所有 CMake 集成代码必须放在 `[mcudeva:user]` 标记区域内。

## Review Focus

1. **mcudeva 重新生成 CMake：** 自动区覆盖后，用户区的 GD32_UI_DIR、UI 源收集和 include 路径必须保留；Task 2 检查标记区并重新配置。
2. **GIF、字体和图片资源：** `LV_USE_GIF`、自定义字体声明或资源遗漏会导致链接失败；Task 2 检查链接符号、Flash/SRAM 报告。
3. **八个页面同时驻留的 LVGL 内存压力：** 128 KB 池可能在 GIF 解码或页面切换时不足；Task 3 在模拟器截图和 MCU 实机中重点观察 LVGL 内存告警。
4. **RTC BCD 转换和首次上电兜底：** RTC 失败时页面时间不能崩溃或显示非法值；Task 2 检查 `ui_platform_time_get()` 返回值处理，Task 3 实机观察时间刷新。
5. **低电平按键和去抖：** USER1/USER2/TAMPER 映射错误会导致页面导航错乱；Task 3 实机逐键验证焦点、激活和返回。

---

## File Structure

### GD32-UI（外部工程，修改但不初始化 Git）

- Create `C:/Users/86135/Desktop/GD32-UI/custom/ui_platform.h`  
  共享平台类型和接口，禁止包含 Win32、GD32 寄存器头文件。
- Create `C:/Users/86135/Desktop/GD32-UI/simulator/ui_platform_sim.c`  
  Win32 时间、毫秒节拍和按键状态实现。
- Modify `C:/Users/86135/Desktop/GD32-UI/custom/gui_app.c`  
  删除 `_WIN32`/`gd32f5hcr_eval.h` 直接依赖，统一走 `ui_platform.h`。
- Modify `C:/Users/86135/Desktop/GD32-UI/simulator/CMakeLists.txt`  
  将 `ui_platform_sim.c` 加入 simulator 目标，不加入共享源码。

### GD32F5HCR-EVAL（MCU Git 工程）

- Create `src/ui_platform_mcu.c`  
  RTC、三个按键 GPIO、FreeRTOS 毫秒时间实现。
- Modify `CMakeLists.txt`  
  只在用户自定义区引用 GD32-UI、收集共享源码和 include 路径。
- Modify `src/main.c`  
  使用 `ui_init()`/`custom_init()` 替换旧测试 UI，并运行 LVGL 定时器循环。
- Modify `lvgl/lv_conf.h`  
  启用 GIF、注册 UI 字体、设置 `LV_MEM_SIZE (128U * 1024U)`。
- Test `build/Debug/gd32f5hcr_eval_lvgl.elf`、`.hex`、`.map`。

---

### Task 1: 共享平台接口与模拟器适配

**Files:**
- Create `C:/Users/86135/Desktop/GD32-UI/custom/ui_platform.h`
- Create `C:/Users/86135/Desktop/GD32-UI/simulator/ui_platform_sim.c`
- Modify `C:/Users/86135/Desktop/GD32-UI/custom/gui_app.c`
- Modify `C:/Users/86135/Desktop/GD32-UI/simulator/CMakeLists.txt`
- Test `C:/Users/86135/Desktop/GD32-UI/simulator/build/simulator.exe`

**Interfaces:**
- Consumes: 现有 `ui_init()`、`gui_app.c` 页面对象访问器、LVGL 8.3.11 API。
- Produces:
  - `void ui_platform_init(void)`
  - `uint32_t ui_platform_millis(void)`
  - `bool ui_platform_time_get(ui_platform_time_t *time)`
  - `bool ui_platform_key_pressed(ui_key_id_t key)`
  - `ui_key_id_t`：`UI_KEY_FOCUS_NEXT`、`UI_KEY_ACTIVATE`、`UI_KEY_BACK`
  - `ui_platform_time_t`：`uint16_t year`、`uint8_t month/day/hour/minute/second`

- [x] **Step 1: 创建共享平台头文件**

Create `custom/ui_platform.h`，使用 include guard 和 `extern "C"`，只定义上述两个类型、三个枚举值和四个函数声明。文件中不得包含 `<windows.h>`、`gd32w51x_f5hc.h` 或 FreeRTOS 头文件。

- [x] **Step 2: 创建模拟器平台实现**

Create `simulator/ui_platform_sim.c`：

- `ui_platform_init()` 空实现；
- `ui_platform_millis()` 返回 `GetTickCount()`；
- `ui_platform_time_get()` 使用 `GetLocalTime()` 转换为 `ui_platform_time_t` 并返回 `true`；
- `ui_platform_key_pressed()` 第一阶段始终返回 `false`，保留现有 LVGL 键盘输入组，避免 Win32 键盘和平台轮询重复导航；
- 无效 `ui_key_id_t` 返回 `false`。

- [x] **Step 3: 重构 `gui_app.c` 平台依赖**

Modify `custom/gui_app.c`：

- 删除 `GUI_APP_SIMULATOR`、`<time.h>`/`gd32f5hcr_eval.h` 条件分支；
- include `ui_platform.h`；
- 删除 `rtc_clock_init()` 及其调用，改为 `ui_platform_init()`；
- 将 `key_typedef_enum`、`KEYn`、`gd_eval_key_state_get()` 替换为 `ui_key_id_t` 和 `ui_platform_key_pressed()`；
- `key_pressed()` 使用 `ui_platform_millis()`；
- `custom_init()` 无条件创建按键导航 timer；
- `update_header_time()` 优先调用 `ui_platform_time_get()`；返回 `false` 时使用编译日期时间解析出的兜底时间；
- 保留页面焦点、菜单、仪表盘、功率图和 OTA 演示逻辑。

- [x] **Step 4: 接入 simulator 构建**

Modify `simulator/CMakeLists.txt`，把 `ui_platform_sim.c` 加入 `simulator` 可执行目标的源列表；不得修改 `UI_SOURCES` 的 generated/custom 收集规则。

- [x] **Step 5: 构建模拟器**

Run:

```powershell
cmake --build C:\Users\86135\Desktop\GD32-UI\simulator\build --target simulator --parallel 2
```

Expected: `simulator.exe` 生成成功，无缺失 `ui_platform_*` 符号。

- [x] **Step 6: 运行八个页面截图回归**

Run：

```powershell
$exe = "C:\Users\86135\Desktop\GD32-UI\simulator\build\simulator.exe"
$out = "C:\Users\86135\AppData\Local\Temp\gd32-ui-screens"
New-Item -ItemType Directory -Force -Path $out | Out-Null
$screens = @("screen", "screen_1", "screen_2", "screen_3", "screen_4", "screen_5", "screen_6", "screen_7")
foreach ($screen in $screens) {
  & $exe --screenshot (Join-Path $out "$screen.ppm") --screen $screen --ticks 60 --tick-ms 33
  if ($LASTEXITCODE -ne 0) { throw "screenshot failed: $screen" }
}
```

Expected: 每个 PPM 文件生成成功且非空；中文字体、图片、GIF 页面无崩溃。

---

### Task 2: MCU 平台实现与固件接入

**Files:**
- Create `C:/Users/86135/Desktop/GD32F5HCR-EVAL/src/ui_platform_mcu.c`
- Modify `C:/Users/86135/Desktop/GD32F5HCR-EVAL/CMakeLists.txt`
- Modify `C:/Users/86135/Desktop/GD32F5HCR-EVAL/src/main.c`
- Modify `C:/Users/86135/Desktop/GD32F5HCR-EVAL/lvgl/lv_conf.h`
- Test `C:/Users/86135/Desktop/GD32F5HCR-EVAL/build/Debug/gd32f5hcr_eval_lvgl.elf`

**Interfaces:**
- Consumes: Task 1 的 `ui_platform.h`；现有 `gd32w51x_f5hc_*` 外设 API；现有 `lv_port_disp_init()`；`ui_init()`。
- Produces: MCU 实现的四个 `ui_platform_*` 函数，以及可运行 `ui_init()` 的固件目标。

- [x] **Step 1: 创建 MCU 平台实现**

Create `src/ui_platform_mcu.c`，include `ui_platform.h`、`gd32w51x_f5hc.h`、`FreeRTOS.h`、`task.h`。

按键映射固定为：

| 逻辑键 | 板级键 | GPIO | 有效电平 |
|---|---|---|---|
| `UI_KEY_FOCUS_NEXT` | USER_1 | PD2 | low |
| `UI_KEY_ACTIVATE` | USER_2 | PA3 | low |
| `UI_KEY_BACK` | TAMPER_WAKEUP | PA2 | low |

`ui_platform_init()` 打开 GPIOA/GPIOD 时钟，三个引脚配置为无上下拉输入，并初始化 RTC。RTC 使用官方 GD32F5HCR_EVAL Calendar demo 参数：`RCU_RTCSRC_IRC32K`、`factor_asyn=0x63`、`factor_syn=0x13F`、备份寄存器 magic `0x32F0`。首次配置时用编译日期时间写入 BCD 字段。

`ui_platform_key_pressed()` 对合法键读取 GPIO 输入并反转为逻辑 pressed；非法键返回 `false`。  
`ui_platform_millis()` 返回 `xTaskGetTickCount()`（当前 `configTICK_RATE_HZ=1000`）。  
`ui_platform_time_get()` RTC 有效时调用 `rtc_current_time_get()` 并将 BCD 转为十进制；RTC 无效时返回 `false`。

- [x] **Step 2: 添加 GD32-UI CMake 桥接**

Modify `CMakeLists.txt` 用户自定义区：

- 定义 `GD32_UI_DIR`，默认 `${CMAKE_SOURCE_DIR}/../GD32-UI`，类型 `PATH`；
- 检查 `${GD32_UI_DIR}/generated`、`${GD32_UI_DIR}/custom/gui_app.c`，缺失时 `message(FATAL_ERROR ...)`；
- 收集 `${GD32_UI_DIR}/generated/*.c` 与 `${GD32_UI_DIR}/custom/gui_app.c`；
- 追加 include 路径 `${GD32_UI_DIR}`、`${GD32_UI_DIR}/generated`、`.../screens`、`.../lang`、`.../theme`、`.../config`、`${GD32_UI_DIR}/custom`；
- 明确排除 `${GD32_UI_DIR}/simulator`；
- 不修改 `[mcudeva:auto]` 区域。

- [x] **Step 3: 配置 LVGL 资源**

Modify `lvgl/lv_conf.h`：

- `LV_MEM_SIZE` 改为 `128U * 1024U`；
- `LV_USE_GIF` 改为 `1`；
- `LV_FONT_CUSTOM_DECLARE` 声明 `font_notosanscjksc_regular_14_b49ae336_fb3f`；
- `LV_FONT_DEFAULT` 指向该字体；
- 不启用发布画面的性能/内存监控覆盖层。

- [x] **Step 4: 替换 MCU 入口测试 UI**

Modify `src/main.c`：

- include `ui_init.h`；
- 删除 `test_bar`、`refresh_label`、`lv_test_ui_init()`；
- `main()` 保留 `lv_init()`、`systick_config()`、`lv_port_disp_init()`，随后调用 `ui_init()`；
- `vApplicationTask()` 只循环 `lv_timer_handler()` 和 `vTaskDelay(pdMS_TO_TICKS(5))`；
- 将 LVGL task stack depth 调整为 `2048` words，保持优先级 1。

- [x] **Step 5: 配置并构建 MCU**

Run:

```powershell
cmake --preset Debug
cmake --build build\Debug --parallel 2
```

Expected: CMake 配置成功；`gd32f5hcr_eval_lvgl.elf` 和 `.hex` 生成成功。

- [x] **Step 6: 检查资源和共享符号**

Run `arm-none-eabi-size`、`arm-none-eabi-nm`，确认：

- `ui_init`、`custom_init`、`ui_platform_init`、`gif_wave` 存在；
- Flash 使用量低于 2 MB；
- `.bss`、FreeRTOS 20 KB heap、LVGL 128 KB 池、双 display buffer 和任务栈总量不超过 320 KB SRAM；
- 构建不编入 `simulator/main.c` 或 Win32 符号。

- [x] **Step 7: Commit**

```powershell
git add CMakeLists.txt src/main.c src/ui_platform_mcu.c lvgl/lv_conf.h
git commit -m "feat: integrate shared GD32 UI into MCU firmware"
```

---

### Task 3: 集成回归、内存调整与实机验收

**Files:**
- Modify only if regression requires: `lvgl/lv_conf.h`、`src/ui_platform_mcu.c`、`src/main.c`
- Test: `simulator.exe` 八页截图、`gd32f5hcr_eval_lvgl.elf`、GD32F5HCR-EVAL 实机

**Interfaces:**
- Consumes: Task 1 的模拟器和 Task 2 的固件。
- Produces: 通过模拟器和实机验收的集成版本。

- [x] **Step 1: 重新运行模拟器八页截图**

Run Task 1 Step 6，确认共享 UI 重构后八个页面均能渲染。

- [x] **Step 2: 重新构建并读取内存报告**

Run Task 2 Step 5、Step 6。若 LVGL 日志或链接报告显示内存不足，只允许在 `96 KB` 至 `160 KB` 范围内调整 `LV_MEM_SIZE`，并重新检查 FreeRTOS 栈堆余量；不得直接采用模拟器的 256 KB。

- [ ] **Step 3: 实机烧写与显示验收**

烧写 `.hex`，确认 LCD 显示 GD32-UI 页面；依次检查八个页面、仪表盘动画、功率图、OTA 动画、GIF 页面和顶部时间。

- [ ] **Step 4: 实机按键验收**

逐个按键确认：

- PD2/USER_1：焦点移动到下一个控件；
- PA3/USER_2：激活当前控件或进入菜单页面；
- PA2/TAMPER_WAKEUP：取消下拉框或返回页面。

连续按压和快速切换页面时不应出现重复导航、卡死或 HardFault。

- [ ] **Step 5: 实机 RTC 与故障路径验收**

观察顶部时间持续更新；重新上电后时间继续运行。若 RTC 未初始化，顶部时间应显示编译时间兜底而不是非法 BCD 值。

- [x] **Step 6: Commit**

如 Task 3 产生了代码调整：

```powershell
git add lvgl/lv_conf.h src/ui_platform_mcu.c src/main.c
git commit -m "test: tune shared UI integration for simulator and MCU"
```

若无需代码调整，保留 Task 2 的提交作为最终集成提交。
