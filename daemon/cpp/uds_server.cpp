#include "uds_server.h"
#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <vector>
#include <algorithm>
#include "nlohmann/json.hpp"
#include <android/log.h>

#define LOG_TAG "crfzitd"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

UdsServer::UdsServer(const std::string& socket_path)
    : socket_path_(socket_path), server_fd_(-1), is_running_(false) {}

UdsServer::~UdsServer() {
    stop();
}

void UdsServer::add_client(int client_fd) {
    std::lock_guard<std::mutex> lock(client_mutex_);
    client_fds_.push_back(client_fd);
    LOGI("Client fd %d added. Total clients: %zu", client_fd, client_fds_.size());
}

void UdsServer::remove_client(int client_fd) {
    std::lock_guard<std::mutex> lock(client_mutex_);
    auto it = std::remove(client_fds_.begin(), client_fds_.end(), client_fd);
    if (it != client_fds_.end()) {
        client_fds_.erase(it, client_fds_.end());
        LOGI("Client fd %d removed. Total clients: %zu", client_fd, client_fds_.size());
    }
    close(client_fd);
}

void UdsServer::broadcast_message(const std::string& message) {
    std::lock_guard<std::mutex> lock(client_mutex_);
    if (client_fds_.empty()) return;

    // 确保以换行符结尾，符合JSON Lines
    std::string line = message + "\n";
    std::vector<int> disconnected_clients;

    for (int fd : client_fds_) {
        ssize_t bytes_sent = send(fd, line.c_str(), line.length(), MSG_NOSIGNAL);
        if (bytes_sent < 0) {
            // EPIPE 表示客户端已关闭连接
            if (errno == EPIPE || errno == ECONNRESET) {
                LOGW("Client fd %d disconnected (EPIPE/ECONNRESET on write).", fd);
                disconnected_clients.push_back(fd);
            } else {
                LOGE("Failed to send to client fd %d: %s", fd, strerror(errno));
            }
        }
    }

    // 从主列表移除已断开的客户端
    for (int fd : disconnected_clients) {
        auto it = std::remove(client_fds_.begin(), client_fds_.end(), fd);
        client_fds_.erase(it, client_fds_.end());
        close(fd);
    }
}


void UdsServer::stop() {
    is_running_ = false;
    if (server_fd_ != -1) {
        shutdown(server_fd_, SHUT_RDWR);
        close(server_fd_);
        server_fd_ = -1;
    }
    // 关闭所有客户端连接
    std::lock_guard<std::mutex> lock(client_mutex_);
    for(int fd : client_fds_) {
        close(fd);
    }
    client_fds_.clear();
}

void UdsServer::run() {
    server_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd_ == -1) {
        LOGE("Failed to create socket: %s", strerror(errno));
        return;
    }

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path + 1, socket_path_.c_str(), sizeof(addr.sun_path) - 2);
    socklen_t addr_len = sizeof(addr.sun_family) + strlen(socket_path_.c_str()) + 1;

    if (bind(server_fd_, (struct sockaddr*)&addr, addr_len) == -1) {
        LOGE("Failed to bind socket '@%s': %s", socket_path_.c_str(), strerror(errno));
        close(server_fd_);
        return;
    }

    if (listen(server_fd_, 5) == -1) {
        LOGE("Failed to listen on socket: %s", strerror(errno));
        close(server_fd_);
        return;
    }

    LOGI("Server is listening on abstract UDS socket: @%s", socket_path_.c_str());
    is_running_ = true;

    while (is_running_) {
        int client_fd = accept(server_fd_, nullptr, nullptr);
        if (client_fd == -1) {
            if (is_running_) {
                LOGE("Failed to accept connection: %s", strerror(errno));
            }
            continue;
        }
        
        LOGI("Accepted new client connection, fd: %d", client_fd);
        // 不再为每个客户端创建线程，而是将其fd加入管理列表
        add_client(client_fd);
        
        // 此处可以启动一个线程来处理该客户端的 *读* 操作，但为了简化，
        // 在此阶段我们只关心 daemon -> client 的广播，忽略 client -> daemon 的消息
    }
}