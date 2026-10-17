# GD32-UI 与 GD32F5HCR-EVAL 集成设计

日期：2026-10-05  
状态：规格已编写，待用户评审

## 1. 目标

将 `C:/Users/86135/Desktop/GD32-UI` 作为 UI 唯一源码来源，使其中的页面、字体、图片、主题和交互逻辑同时运行在：

- GD32-UI Windows 模拟器；
- GD32F5HCR-EVAL MCU 固件。

本轮完成 UI 移植、构建对接、显示适配、三个实体按键适配和 RTC 时间显示。仪表盘、功率图和 OTA 页面继续使用 `gui_app.c` 中的演示数据。

## 2. 非目标

- 不建立温度、功率、开关等真实业务数据模型。
- 不实现真实 OTA 下载、校验和写入。
- 不把 MCU 与 PC 模拟器做成运行时通信的两个进程。
- 不迁移或重构 GD32EmbeddedUI 的设计器工程文件。
- 不升级 LVGL 大版本。

## 3. 当前状态

- MCU 工程：`C:/Users/86135/Desktop/GD32F5HCR-EVAL`
- UI 工程：`C:/Users/86135/Desktop/GD32-UI`
- 两个工程为相邻目录。
- 两边均使用 LVGL 8.3.11、320x240、RGB565。
- MCU 显示链路已经可用：`lv_port_disp.c` -> SPI1 + DMA -> LCD。
- GD32-UI 包含八个页面、NotoSans CJK 字体、图片和 GIF 资源。
- `gui_app.c` 目前通过 `_WIN32` 区分模拟器和 MCU，并直接引用 `gd32f5hcr_eval.h`。
- MCU 工程当前没有 `gd32f5hcr_eval.h`，也没有 `gd_eval_key_*` 板级接口，因此 UI 的 MCU 路径不能直接编译。
- 模拟器配置使用 256 KB LVGL 内存池；MCU 总 SRAM 为 320 KB，且已有 FreeRTOS 堆、显示缓冲和任务栈需求。

## 4. 架构

```text
GD32-UI/generated + GD32-UI/custom/gui_app.c
                    |
                    v
             custom/ui_platform.h
             /                    \
simulator/ui_platform_sim.c   MCU/ui_platform_mcu.c
Win32 时间/输入映射             RTC/GPIO/毫秒时间
             \                    /
              LVGL 8.3.11 + ui_init()
                    |
          simulator.exe      GD32F5HCR-EVAL ELF
          Win32 FrameBuffer   SPI LCD DMA
```

`GD32-UI/generated/` 与 `GD32-UI/custom/` 是共享层。平台差异只能出现在 `ui_platform.h` 的两个实现中，以及模拟器、MCU 各自的 LVGL 显示/输入设备代码中。

## 5. 组件职责

### 5.1 GD32-UI generated

保存设计器生成的页面、事件注册、字体、图片、语言和主题代码。实施时只做编译适配，不手工调整布局代码，以便后续设计器重新生成。

### 5.2 GD32-UI custom/gui_app.c

保存页面导航、仪表盘模拟值、功率曲线、OTA 演示和时间刷新逻辑。删除 `_WIN32` 和 `gd32f5hcr_eval.h` 依赖，改用统一平台接口。

### 5.3 custom/ui_platform.h

定义 UI 所需的最小平台接口：

```c
typedef enum {
    UI_KEY_FOCUS_NEXT = 0,
    UI_KEY_ACTIVATE,
    UI_KEY_BACK
} ui_key_id_t;

typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} ui_platform_time_t;

void ui_platform_init(void);
uint32_t ui_platform_millis(void);
bool ui_platform_time_get(ui_platform_time_t *time);
bool ui_platform_key_pressed(ui_key_id_t key);
```

`ui_platform_key_pressed()` 返回逻辑按键是否按下；硬件有效电平只在平台实现中处理。按键去抖继续由 `gui_app.c` 的统一逻辑完成。

### 5.4 simulator/ui_platform_sim.c

使用 Windows 系统时间提供时间接口，使用 `GetTickCount()` 提供毫秒时间。模拟器保留现有鼠标、键盘、LVGL 输入组和截图能力；方向键/Enter/Esc 可以映射到三个逻辑按键。

### 5.5 MCU/ui_platform_mcu.c

负责：

- 初始化 RTC，并从 RTC 读取日历时间；
- 初始化三个 EVAL 板按键 GPIO，映射为 `UI_KEY_FOCUS_NEXT`、`UI_KEY_ACTIVATE`、`UI_KEY_BACK`；
- 使用 FreeRTOS tick 提供毫秒时间；
- RTC 初始化失败时返回固件编译时间作为兜底。

GPIO 引脚和有效电平只在此文件中的板级常量定义，不进入共享 UI 层。

### 5.6 MCU 显示与调度

保留当前 `lv_port_disp.c` 的 SPI DMA 双缓冲实现。`main()` 在 LVGL、系统节拍和显示驱动初始化后调用 `ui_init()`，FreeRTOS UI 任务继续周期调用 `lv_timer_handler()`。

## 6. 构建集成

MCU `CMakeLists.txt` 只在 `[mcudeva:user]` 区域增加集成逻辑：

1. 定义 `GD32_UI_DIR`，默认值为 `${CMAKE_SOURCE_DIR}/../GD32-UI`，允许通过 CMake cache 覆盖。
2. 检查 `GD32_UI_DIR/generated` 和 `GD32_UI_DIR/custom` 是否存在；缺失时立即报错。
3. 收集 `generated/**/*.c` 与 `custom/gui_app.c`。
4. 增加 `generated`、各生成子目录、`custom` 的 include 路径。
5. 只编译 `src/ui_platform_mcu.c` 等 MCU 平台代码，不编译 `simulator/`。
6. 保持用户自定义 CMake 区域可被 mcudeva 重新生成而不丢失。

模拟器 CMake 继续只编译 GD32-UI 的 `generated/`、`custom/` 和 `simulator/`。

## 7. 数据流

### 7.1 MCU 启动

```text
main()
  -> lv_init()
  -> systick_config()
  -> lv_port_disp_init()
  -> ui_init()
       -> screen_create() x8
       -> custom_init()
            -> ui_platform_init()
            -> 创建仪表盘、功率图、OTA、时间、按键定时器
  -> xTaskCreate(vApplicationTask)
  -> vTaskStartScheduler()
  -> lv_timer_handler()
  -> disp_flush()
  -> SPI DMA
```

### 7.2 页面更新

仪表盘、功率曲线和 OTA 进度由 `gui_app.c` 的 LVGL timer 更新。时间刷新 timer 每秒调用 `ui_platform_time_get()`。按键 timer 轮询 `ui_platform_key_pressed()` 并复用当前页面导航逻辑。

### 7.3 模拟器

模拟器先注册 Win32 显示和输入设备，再调用同一个 `ui_init()`。业务数据仍是共享 `gui_app.c` 的演示数据，与 MCU 一致。

## 8. LVGL 配置与内存

MCU `lv_conf.h` 需要与 UI 资源匹配：

- 启用 `LV_USE_GIF`；
- 注册 `font_notosanscjksc_regular_14_b49ae336_fb3f`；
- 默认字体使用该 UI 字体；
- `LV_MEM_SIZE` 首轮设置为 128 KB；
- 性能和内存监视器仅在调试配置中启用，不作为发布画面的一部分。

128 KB 是初始预算，不照搬模拟器的 256 KB。构建报告必须记录 Flash、SRAM、FreeRTOS 堆、LVGL 池和显示缓冲占用。如果运行时出现 LVGL 分配失败，优先减少 GIF/图片的 RAM 解码缓存或调整页面生命周期，不直接扩大到挤占 FreeRTOS 栈堆。

## 9. 错误处理

- `GD32_UI_DIR` 不合法：CMake 配置失败并打印实际路径。
- RTC 无效或初始化失败：`ui_platform_time_get()` 返回 false，UI 使用编译时间兜底。
- 无效按键编号：`ui_platform_key_pressed()` 返回 false。
- LVGL 对象或 GIF 解码失败：保留 LVGL 日志和断言，不使用空重试隐藏故障。
- Flash 超限：链接器直接失败，不生成缺少 UI 资源的固件。
- DMA 显示继续使用 `lv_disp_flush_ready()` 通知 LVGL；平台层不绕过现有刷新握手。

## 10. 测试与验收

### 10.1 模拟器

- 使用现有 GD32EmbeddedUI MinGW/CMake 工具链构建 `simulator.exe`。
- 对 `screen`、`screen_1` 至 `screen_7` 执行无窗口截图。
- 确认八个页面均能渲染，中文字体、图片、GIF、主题和按钮事件正常。

### 10.2 MCU 构建

- 构建 `gd32f5hcr_eval_lvgl.elf` 和 `.hex`。
- 链接成功，Flash 位于 2 MB 限制内，SRAM 保留 FreeRTOS 栈堆空间。
- ELF 中包含 `ui_init`、`custom_init`、UI 字体和 GIF 资源符号。

### 10.3 MCU 实机

- LCD 显示 GD32-UI 页面，而不是旧测试画面。
- 三个按键可完成上一页/下一页/返回或控件导航。
- 顶部时间持续刷新。
- 八个页面切换、仪表盘动画、GIF 播放过程中无 DMA 阻塞、LVGL 内存不足或 HardFault。

### 10.4 回归

- 现有 SPI LCD DMA 刷新继续工作。
- mcudeva 重新生成 CMake 自动区后，用户区集成逻辑保留。
- GD32-UI 模拟器仍可独立构建和运行。

最终 LCD 画面、按键手感和 RTC 时间由硬件测试人员确认。

## 11. 完成标准

满足以下条件即完成本轮集成：

1. GD32-UI 是 UI 唯一源码来源，MCU 不维护 UI 副本。
2. 模拟器和 MCU 均可独立构建。
3. MCU 固件运行 GD32-UI 的八个页面。
4. 显示、三个按键、RTC 时间和 LVGL timer 已接入。
5. 构建和实机验收通过，且不存在未说明的平台头文件依赖。



