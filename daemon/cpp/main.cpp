#include "uds_server.h"
#include "nlohmann/json.hpp"
#include <iostream>
#include <csignal>
#include <thread>
#include <chrono>
#include <random>

#include <android/log.h>
#define LOG_TAG "crfzitd"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

using json = nlohmann::json;

const std::string SOCKET_PATH = "crfzit_socket";
UdsServer* g_server = nullptr;
std::atomic<bool> g_is_running = true;

// 模拟数据生成函数
json generate_mock_dashboard_update() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> cpu_dist(10.0, 60.0);
    std::uniform_int_distribution<> mem_dist(4 * 1024 * 1024, 5 * 1024 * 1024);
    std::uniform_int_distribution<> net_dist(0, 2 * 1024 * 1024 * 8);

    json payload;
    payload["global_stats"] = {
        {"total_cpu_usage_percent", cpu_dist(gen)},
        {"total_mem_kb", 8 * 1024 * 1024},
        {"avail_mem_kb", mem_dist(gen)},
        {"net_down_speed_bps", net_dist(gen)},
        {"net_up_speed_bps", net_dist(gen) / 4},
        {"active_profile_name", "常规模式"}
    };

    payload["apps_runtime_state"] = json::array({
        {
            {"package_name", "com.tencent.tmgp.sgame"},
            {"app_name", "王者荣耀"},
            {"display_status", "FOREGROUND_GAME"},
            {"active_freeze_mode", nullptr},
            {"mem_usage_kb", 1258291},
            {"cpu_usage_percent", 45.1},
            {"is_whitelisted", false},
            {"is_foreground", true}
        },
        {
            {"package_name", "com.taobao.taobao"},
            {"app_name", "淘宝"},
            {"display_status", "FROZEN"},
            {"active_freeze_mode", "CGROUP"},
            {"mem_usage_kb", 184320},
            {"cpu_usage_percent", 0.0},
            {"is_whitelisted", false},
            {"is_foreground", false}
        },
        {
            {"package_name", "com.zhihu.android"},
            {"app_name", "知乎"},
            {"display_status", "FROZEN"},
            {"active_freeze_mode", "SIGSTOP"},
            {"mem_usage_kb", 153600},
            {"cpu_usage_percent", 0.0},
            {"is_whitelisted", false},
            {"is_foreground", false}
        }
    });

    json update_message;
    update_message["type"] = "stream";
    update_message["command"] = "dashboard_update";
    update_message["payload"] = payload;

    return update_message;
}


// 数据广播线程
void data_broadcaster() {
    while (g_is_running) {
        if (g_server) {
            json msg = generate_mock_dashboard_update();
            g_server->broadcast_message(msg.dump());
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

    LOGI("CRFzit Daemon v1.1 (JSON Broadcaster) starting...");
    
    UdsServer server(SOCKET_PATH);
    g_server = &server;
    
    // 启动数据广播线程
    std::thread broadcaster_thread(data_broadcaster);

    // 主线程运行UDS服务器
    server.run();
    
    g_is_running = false; // 确保广播线程退出
    if (broadcaster_thread.joinable()) {
        broadcaster_thread.join();
    }
    
    LOGI("CRFzit Daemon has shut down.");
    return 0;
}