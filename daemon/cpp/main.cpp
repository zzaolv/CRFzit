#include "uds_server.h"
#include <iostream>
#include <csignal>

#include <android/log.h>
#define LOG_TAG "crfzitd"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// 定义Socket路径，与文档一致
const std::string SOCKET_PATH = "crfzit_socket";

// 全局服务器实例指针，用于信号处理
UdsServer* g_server = nullptr;

void signal_handler(int signum) {
    LOGI("Caught signal %d, shutting down...", signum);
    if (g_server) {
        g_server->stop();
    }
    exit(signum);
}


int main() {
    // 设置信号处理器，以优雅地关闭服务器
    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    LOGI("CRFzit Daemon v1.0 starting...");
    
    UdsServer server(SOCKET_PATH);
    g_server = &server; // 将服务器实例地址赋给全局指针
    
    server.run(); // 阻塞运行
    
    LOGI("CRFzit Daemon has shut down.");
    return 0;
}