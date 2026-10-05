set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m33)

set(_GD32_TOOLCHAIN_BIN "${CMAKE_CURRENT_LIST_DIR}/tools/toolchain/arm-gnu-toolchain-15.3.rel1-mingw-w64-x86_64-arm-none-eabi/bin")

# 使用项目内交叉编译工具
set(CMAKE_C_COMPILER "${_GD32_TOOLCHAIN_BIN}/arm-none-eabi-gcc.exe")
set(CMAKE_CXX_COMPILER "${_GD32_TOOLCHAIN_BIN}/arm-none-eabi-g++.exe")
set(CMAKE_ASM_COMPILER "${_GD32_TOOLCHAIN_BIN}/arm-none-eabi-gcc.exe")

set(CMAKE_OBJCOPY "${_GD32_TOOLCHAIN_BIN}/arm-none-eabi-objcopy.exe")
set(CMAKE_OBJDUMP "${_GD32_TOOLCHAIN_BIN}/arm-none-eabi-objdump.exe")
set(CMAKE_SIZE "${_GD32_TOOLCHAIN_BIN}/arm-none-eabi-size.exe")

# 交叉编译目标无法在主机上运行
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
