#!/bin/bash

# /CRFzit/scripts/remote_build.sh

# --- 配置 ---
set -e # 任何命令失败则立即退出
REMOTE_USER="zeaolv"
REMOTE_HOST="192.168.31.17"
REMOTE_PROJECT_ROOT="/home/${REMOTE_USER}/CRFzit_build"
# 本地项目根目录（脚本的父目录）
LOCAL_PROJECT_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )/.." && pwd )"
# 最终产物要拷贝到的本地位置
LOCAL_ARTIFACT_DEST="${LOCAL_PROJECT_ROOT}/app/src/main/resources/lib/arm64-v8a/"

# --- SSH 连接信息 ---
SSH_CMD="ssh ${REMOTE_USER}@${REMOTE_HOST}"

echo ">>> [STEP 1] Syncing project source to remote builder: ${REMOTE_HOST}"
# 使用 rsync 同步源代码，排除构建产物和IDE文件
rsync -avz --delete \
    --exclude='.git' --exclude='.idea' --exclude='app/build' --exclude='app/release' \
    --exclude='daemon/build' --exclude='daemon/rust/target' \
    "${LOCAL_PROJECT_ROOT}/" "${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_PROJECT_ROOT}"

echo ">>> [STEP 2] Executing remote build script..."
# 通过SSH在远程服务器上执行编译命令
${SSH_CMD} << 'ENDSSH'
set -e # 确保远程脚本也设置了失败即退出
# 加载环境变量
source "$HOME/.cargo/env"
source "$HOME/.bashrc"

# 进入项目目录
cd /home/your_ubuntu_user/CRFzit_build

echo "--> [Remote] Building Rust core logic (staticlib)..."
cd daemon/rust
cargo build --target aarch64-linux-android --release
cd ../..

echo "--> [Remote] Configuring and building C++ daemon with CMake..."
# 创建构建目录
mkdir -p daemon/build && cd daemon/build

# 运行CMake配置，指定Android NDK工具链文件
cmake -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a \
      -DANDROID_PLATFORM=android-35 \
      -DCMAKE_BUILD_TYPE=Release \
      -G "Ninja" \
      ../cpp

echo "--> [Remote] Compiling with Ninja..."
ninja

echo "--> [Remote] Build finished successfully."
ENDSSH

echo ">>> [STEP 3] Fetching compiled artifact (crfzitd)..."
# 创建本地存放产物的目录
mkdir -p "${LOCAL_ARTIFACT_DEST}"
# 从远程服务器下载编译好的可执行文件
rsync -avz "${REMOTE_USER}@${REMOTE_HOST}:${REMOTE_PROJECT_ROOT}/daemon/build/crfzitd" "${LOCAL_ARTIFACT_DEST}/"

echo ">>> [SUCCESS] Build process complete. 'crfzitd' is now at ${LOCAL_ARTIFACT_DEST}"

echo ">>> [STEP 4] Building Android App (APK)..."
# 在Windows上，Gradle Wrapper通常是 gradlew.bat
# 注意：你需要根据你的环境调整路径
cd "${LOCAL_PROJECT_ROOT}/app"
./gradlew.bat assembleRelease

echo ">>> [SUCCESS] CRFzit-Console.apk built successfully."