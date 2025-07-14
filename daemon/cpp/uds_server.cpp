#include "uds_server.h"
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <vector>

// 引入 nlohmann/json
#include "nlohmann/json.hpp"

// 引入 Android 日志
#include <android/log.h>
#define LOG_TAG "crfzitd"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)


// 缓冲区大小
constexpr int BUFFER_SIZE = 4096;

UdsServer::UdsServer(const std::string& socket_path)
    : socket_path_(socket_path), server_fd_(-1), is_running_(false) {}

UdsServer::~UdsServer() {
    stop();
}

void UdsServer::stop() {
    is_running_ = false;
    if (server_fd_ != -1) {
        close(server_fd_);
        server_fd_ = -1;
        // 注意：我们使用了 abstract namespace，不需要 unlink
    }
    for (auto& t : client_threads_) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void UdsServer::run() {
    // 1. 创建 socket
    server_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd_ == -1) {
        LOGE("Failed to create socket: %s", strerror(errno));
        return;
    }

    // 2. 配置地址 (使用 Abstract Namespace)
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    // 第一个字节必须是 null，表示 abstract namespace
    strncpy(addr.sun_path + 1, socket_path_.c_str(), sizeof(addr.sun_path) - 2);
    
    // Abstract Namespace 的长度计算要包含 null 终止符
    socklen_t addr_len = sizeof(addr.sun_family) + strlen(socket_path_.c_str()) + 1;

    // 3. 绑定 socket
    if (bind(server_fd_, (struct sockaddr*)&addr, addr_len) == -1) {
        LOGE("Failed to bind socket '@%s': %s", socket_path_.c_str(), strerror(errno));
        close(server_fd_);
        return;
    }

    // 4. 开始监听
    if (listen(server_fd_, 5) == -1) {
        LOGE("Failed to listen on socket: %s", strerror(errno));
        close(server_fd_);
        return;
    }

    LOGI("Server is listening on abstract UDS socket: @%s", socket_path_.c_str());
    is_running_ = true;

    // 5. 接受连接循环
    while (is_running_) {
        int client_fd = accept(server_fd_, nullptr, nullptr);
        if (client_fd == -1) {
            if (is_running_) {
                LOGE("Failed to accept connection: %s", strerror(errno));
            }
            continue;
        }
        
        LOGI("Accepted new client connection, fd: %d", client_fd);
        
        // 为每个客户端创建一个新线程来处理
        client_threads_.emplace_back(&UdsServer::handle_client, this, client_fd);
    }
}

void UdsServer::handle_client(int client_fd) {
    char buffer[BUFFER_SIZE];
    std::string incomplete_line;

    while (is_running_) {
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);

        if (bytes_read <= 0) {
            if (bytes_read < 0) {
                LOGE("Read error from client fd %d: %s", client_fd, strerror(errno));
            } else {
                LOGI("Client fd %d disconnected.", client_fd);
            }
            break; // 连接断开或出错
        }

        buffer[bytes_read] = '\0';
        incomplete_line += buffer;

        size_t pos;
        // 处理所有完整的行 (以 '\n' 分隔)
        while ((pos = incomplete_line.find('\n')) != std::string::npos) {
            std::string json_line = incomplete_line.substr(0, pos);
            incomplete_line.erase(0, pos + 1);

            if (json_line.empty()) continue;

            LOGI("Received JSON line from client: %s", json_line.c_str());
            
            try {
                auto json = nlohmann::json::parse(json_line);
                LOGI("Successfully parsed JSON. Command: %s", json.value("command", "N/A").c_str());

                // 在第一阶段，我们不发送响应，只打印日志
            } catch (const nlohmann::json::parse_error& e) {
                LOGE("JSON parse error: %s", e.what());
            }
        }
    }

    close(client_fd);
    LOGI("Closed client connection fd: %d", client_fd);
}