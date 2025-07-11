# --- arm64-toolchain.cmake ---

# 设定目标系统为Android
# Set the target system to Android
set(CMAKE_SYSTEM_NAME Android)

# 设定最低API级别，对应minSDK 31
# Set the minimum API level, corresponding to minSDK 31
set(CMAKE_SYSTEM_VERSION 31)

# 设定目标CPU架构为arm64-v8a
# Set the target CPU architecture to arm64-v8a
set(CMAKE_ANDROID_ARCH_ABI arm64-v8a)

# 指定NDK的根目录，这里我们使用您设定的环境变量
# Specify the NDK root directory, using the environment variable you set
set(CMAKE_ANDROID_NDK ${ENV{ANDROID_NDK_HOME}})

# 设定C++标准库为c++_shared，这对于gRPC等动态库是必需的
# Set the C++ standard library to c++_shared, which is necessary for dynamic libraries like gRPC
set(CMAKE_ANDROID_STL c++_shared)