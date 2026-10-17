# =============================================================================
# CMakeLists.txt — 由 mcudeva 自动生成
#
#   [mcudeva:auto] 区：自动生成，重新生成 / 同步 .project.json 时会被整体覆盖，
#                      请勿直接修改（改动会丢失）。
#   [mcudeva:user] 区：用户自定义区，重新生成时原样保留，
#                      请把所有手写的 CMake 语句写在这些区域内。
#
# =============================================================================

# ==== [mcudeva:auto] BEGIN project (hash:55f0f194) ====
cmake_minimum_required(VERSION 3.20)

project(gd32f5hcr_eval_lvgl C ASM)

set(LINKER_SCRIPT gd32f5hc_flash.ld)
# ==== [mcudeva:auto] END project ====

# ---- [mcudeva:user] BEGIN prologue — 用户自定义区，重新生成时保留，可自由修改 ----
# 可在此设置前置变量 / option()，作用于后续所有自动区
set(GD32_UI_DIR "${CMAKE_SOURCE_DIR}/../GD32-UI" CACHE PATH "GD32-UI project root")
if(NOT EXISTS "${GD32_UI_DIR}/generated" OR NOT EXISTS "${GD32_UI_DIR}/custom/gui_app.c")
    message(FATAL_ERROR "GD32-UI sources not found: ${GD32_UI_DIR}")
endif()
# ---- [mcudeva:user] END prologue ----

# ==== [mcudeva:auto] BEGIN flags (hash:b36185d7) ====
# 编译参数
set(MCU_FLAGS "-mcpu=cortex-m33 -mthumb")

set(CMAKE_C_FLAGS
    "${MCU_FLAGS} -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -std=gnu11"
    CACHE STRING "" FORCE)

set(CMAKE_ASM_FLAGS "${MCU_FLAGS} -x assembler-with-cpp" CACHE STRING "" FORCE)

# 各配置编译标志（按 CMAKE_BUILD_TYPE 自动叠加）
set(CMAKE_C_FLAGS_DEBUG "-O0 -g3" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "-O0 -g" CACHE STRING "" FORCE)

set(CMAKE_EXE_LINKER_FLAGS
    "${MCU_FLAGS} \
     -T${LINKER_SCRIPT} \
     -Wl,--print-memory-usage \
     -Xlinker --gc-sections \
     -Wl,-Map=${PROJECT_NAME}.map \
     --specs=nano.specs \
     --specs=nosys.specs \
     -L${CMAKE_SOURCE_DIR}/ldscripts \
     -Wl,--no-warn-rwx-segments"
    CACHE STRING "" FORCE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# ==== [mcudeva:auto] END flags ====

# ---- [mcudeva:user] BEGIN flags — 用户自定义区，重新生成时保留，可自由修改 ----
# 可在此追加编译/链接选项，例如 set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
# ---- [mcudeva:user] END flags ----

# ==== [mcudeva:auto] BEGIN definitions (hash:b4217149) ====
# 预处理器宏定义
add_compile_definitions(
    USE_STDPERIPH_DRIVER
    SYS_NS
    GD32F5HC
    FW_DEBUG_ERR_REPORT
    GD_ECLIPSE_GCC
)
# ==== [mcudeva:auto] END definitions ====

# ---- [mcudeva:user] BEGIN definitions — 用户自定义区，重新生成时保留，可自由修改 ----
# 可在此追加宏定义，例如 add_compile_definitions(MY_FLAG=1)
# ---- [mcudeva:user] END definitions ----

# ==== [mcudeva:auto] BEGIN includes (hash:a071b82a) ====
# 头文件路径
set(INC_DIRS
    ${CMAKE_SOURCE_DIR}/Firmware/CMSIS
    ${CMAKE_SOURCE_DIR}/Firmware/CMSIS/GD/GD32W51x_F5HC/Include
    ${CMAKE_SOURCE_DIR}/Firmware/GD32W51x_F5HC_standard_peripheral/Include
    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/freertos
    ${CMAKE_SOURCE_DIR}/lvgl
    ${CMAKE_SOURCE_DIR}/lvgl/lvgl_port
    ${CMAKE_SOURCE_DIR}/freertos/include
)
# ==== [mcudeva:auto] END includes ====

# ---- [mcudeva:user] BEGIN includes — 用户自定义区，重新生成时保留，可自由修改 ----
# 可在此追加头文件路径，例如 list(APPEND INC_DIRS ${CMAKE_SOURCE_DIR}/my_inc)
list(APPEND INC_DIRS
    "${GD32_UI_DIR}"
    "${GD32_UI_DIR}/generated"
    "${GD32_UI_DIR}/generated/screens"
    "${GD32_UI_DIR}/generated/lang"
    "${GD32_UI_DIR}/generated/theme"
    "${GD32_UI_DIR}/generated/config"
    "${GD32_UI_DIR}/custom"
)
# ---- [mcudeva:user] END includes ----

# ==== [mcudeva:auto] BEGIN sources (hash:73eb9583) ====
# 源文件列表
set(SOURCES)
file(GLOB_RECURSE SOURCE_GLOB_0 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.c"
)
list(REVERSE SOURCE_GLOB_0)
list(APPEND SOURCES ${SOURCE_GLOB_0})

file(GLOB_RECURSE SOURCE_GLOB_1 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/gcc_startup/*.c"
    "${CMAKE_SOURCE_DIR}/gcc_startup/*.s"
    "${CMAKE_SOURCE_DIR}/gcc_startup/*.S"
)
list(REVERSE SOURCE_GLOB_1)
list(APPEND SOURCES ${SOURCE_GLOB_1})

file(GLOB_RECURSE SOURCE_GLOB_2 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/Firmware/GD32W51x_F5HC_standard_peripheral/Source/*.c"
)
list(REVERSE SOURCE_GLOB_2)
list(APPEND SOURCES ${SOURCE_GLOB_2})

file(GLOB_RECURSE SOURCE_GLOB_3 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/Firmware/CMSIS/GD/GD32W51x_F5HC/Source/*.c"
)
list(REVERSE SOURCE_GLOB_3)
list(APPEND SOURCES ${SOURCE_GLOB_3})

file(GLOB_RECURSE SOURCE_GLOB_4 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/freertos/*.c"
)
list(REVERSE SOURCE_GLOB_4)
list(APPEND SOURCES ${SOURCE_GLOB_4})

file(GLOB_RECURSE SOURCE_GLOB_5 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/lvgl/lvgl_port/*.c"
)
list(REVERSE SOURCE_GLOB_5)
list(APPEND SOURCES ${SOURCE_GLOB_5})

file(GLOB_RECURSE SOURCE_GLOB_6 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/lvgl/src/core/*.c"
)
list(REVERSE SOURCE_GLOB_6)
list(APPEND SOURCES ${SOURCE_GLOB_6})

file(GLOB_RECURSE SOURCE_GLOB_7 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/lvgl/src/draw/*.c"
)
list(REVERSE SOURCE_GLOB_7)
list(APPEND SOURCES ${SOURCE_GLOB_7})

file(GLOB_RECURSE SOURCE_GLOB_8 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/lvgl/src/extra/*.c"
)
list(REVERSE SOURCE_GLOB_8)
list(APPEND SOURCES ${SOURCE_GLOB_8})

file(GLOB_RECURSE SOURCE_GLOB_9 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/lvgl/src/font/*.c"
)
list(REVERSE SOURCE_GLOB_9)
list(APPEND SOURCES ${SOURCE_GLOB_9})

file(GLOB_RECURSE SOURCE_GLOB_10 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/lvgl/src/hal/*.c"
)
list(REVERSE SOURCE_GLOB_10)
list(APPEND SOURCES ${SOURCE_GLOB_10})

file(GLOB_RECURSE SOURCE_GLOB_11 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/lvgl/src/misc/*.c"
)
list(REVERSE SOURCE_GLOB_11)
list(APPEND SOURCES ${SOURCE_GLOB_11})

file(GLOB_RECURSE SOURCE_GLOB_12 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/lvgl/src/widgets/*.c"
)
list(REVERSE SOURCE_GLOB_12)
list(APPEND SOURCES ${SOURCE_GLOB_12})

file(GLOB_RECURSE SOURCE_GLOB_13 CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/tests/*.c"
)
list(REVERSE SOURCE_GLOB_13)
list(APPEND SOURCES ${SOURCE_GLOB_13})

# ==== [mcudeva:auto] END sources ====

# ---- [mcudeva:user] BEGIN sources — 用户自定义区，重新生成时保留，可自由修改 ----
# 可在此增删源文件，例如 list(APPEND SOURCES ${CMAKE_SOURCE_DIR}/my/extra.c)
file(GLOB_RECURSE GD32_UI_SOURCES CONFIGURE_DEPENDS
    "${GD32_UI_DIR}/generated/*.c"
)
list(APPEND GD32_UI_SOURCES
    "${GD32_UI_DIR}/custom/gui_app.c"
    "${GD32_UI_DIR}/custom/ui_platform.c"
)
list(FILTER GD32_UI_SOURCES EXCLUDE REGEX "/simulator/")
list(APPEND SOURCES ${GD32_UI_SOURCES})
list(FILTER SOURCES EXCLUDE REGEX "/tools/|/tests/")
# ---- [mcudeva:user] END sources ----

# ==== [mcudeva:auto] BEGIN target (hash:57a9b457) ====
set(TARGET_ELF ${PROJECT_NAME}.elf)

add_executable(${TARGET_ELF}
    ${SOURCES}
)

target_include_directories(${TARGET_ELF} PRIVATE ${INC_DIRS})

# ==== [mcudeva:auto] END target ====

# ---- [mcudeva:user] BEGIN target — 用户自定义区，重新生成时保留，可自由修改 ----
# 可在此写 target_* 命令，例如 target_link_libraries(${TARGET_ELF} PRIVATE m)
# ---- [mcudeva:user] END target ----

# ==== [mcudeva:auto] BEGIN post-build (hash:88de8463) ====
# 构建后处理
set(TARGET_HEX ${PROJECT_NAME}.hex)
set(TARGET_LST ${PROJECT_NAME}.lst)

add_custom_command(TARGET ${TARGET_ELF} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${TARGET_ELF}> ${TARGET_HEX}
    COMMAND ${CMAKE_OBJDUMP} --source --all-headers --demangle --line-numbers --wide
            $<TARGET_FILE:${TARGET_ELF}> > ${TARGET_LST}
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${TARGET_ELF}>
    COMMENT "Generating ${TARGET_HEX}, ${TARGET_LST}"
    BYPRODUCTS ${TARGET_HEX} ${TARGET_LST}
)
# ==== [mcudeva:auto] END post-build ====

# ---- [mcudeva:user] BEGIN epilogue — 用户自定义区，重新生成时保留，可自由修改 ----
# 可在此追加自定义 target / include(user.cmake) 等收尾语句
# ---- [mcudeva:user] END epilogue ----

