# --- arm64-toolchain.cmake (Corrected Version) ---

# 设定目标系统为Android
set(CMAKE_SYSTEM_NAME Android)

# 设定最低API级别，对应minSDK 31
set(CMAKE_SYSTEM_VERSION 31)

# 设定目标CPU架构为arm64-v8a
set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)

# ---------------------------------------------------------
# [FIXED] 修正了这里的语法
# [FIXED] Corrected the syntax here
# ---------------------------------------------------------
# 指定NDK的根目录，使用正确的 $ENV{} 语法
# Specify the NDK root directory, using the correct $ENV{} syntax
set(CMAKE_ANDROID_NDK $ENV{ANDROID_NDK_HOME})

# 设定C++标准库为c++_shared，这对于gRPC等动态库是必需的
set(CMAKE_ANDROID_STL c++_shared)