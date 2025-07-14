#include "uds_server.h"
#include "db_manager.h"
#include "state_manager.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>
#include <filesystem> // 需要 C++17

#include <android/log.h>
#define LOG_TAG "crfzitd_main"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

using json = nlohmann::json;

const std::string SOCKET_PATH = "crfzit_socket";
// 数据目录，与 Magisk service.sh 中创建的目录一致
const std::string DATA_DIR = "/data/adb/crfzit";
const std::string DB_PATH = DATA_DIR + "/crfzit.db";

// 全局指针，用于信号处理和线程间通信
std::unique_ptr<UdsServer> g_server = nullptr;
std::shared_ptr<StateManager> g_state_manager = nullptr;
std::atomic<bool> g_is_running = true;

// 数据广播线程，现在使用 StateManager 的数据
void data_broadcaster() {
    while (g_is_running) {
        if (g_server && g_state_manager) {
            json payload = g_state_manager->get_dashboard_update_payload();
            
            json update_message;
            update_message["type"] = "stream";
            update_message["command"] = "dashboard_update";
            update_message["payload"] = payload;

            g_server->broadcast_message(update_message.dump());
        }
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    LOGI("Data broadcaster thread finished.");
}

void signal_handler(int signum) {
    LOGI("Caught signal %d, shutting down...", signum);
    g_is_running = false;
    if (g_server) {
        g_server->stop();
    }
}

int main() {
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    LOGI("CRFzit Daemon v1.2 (Core Logic) starting...");

    // 确保数据目录存在
    try {
        if (!std::filesystem::exists(DATA_DIR)) {
            std::filesystem::create_directories(DATA_DIR);
            LOGI("Created data directory: %s", DATA_DIR.c_str());
        }
    } catch(const std::filesystem::filesystem_error& e) {
        LOGE("Failed to create data directory: %s", e.what());
        return 1;
    }

    // --- 初始化核心组件 ---
    auto db_manager = std::make_shared<DbManager>(DB_PATH);
    g_state_manager = std::make_shared<StateManager>(db_manager);
    g_server = std::make_unique<UdsServer>(SOCKET_PATH);
    
    // --- 启动后台线程 ---
    std::thread broadcaster_thread(data_broadcaster);

    // --- 主线程运行UDS服务器 ---
    g_server->run();
    
    // --- 清理 ---
    g_is_running = false;
    if (broadcaster_thread.joinable()) {
        broadcaster_thread.join();
    }
    
    LOGI("CRFzit Daemon has shut down.");
    return 0;
}