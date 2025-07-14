#ifndef CRFZITD_UDS_SERVER_H
#define CRFZITD_UDS_SERVER_H

#include <string>
#include <thread>
#include <vector>
#include <atomic>

class UdsServer {
public:
    // 构造函数，指定UDS路径
    UdsServer(const std::string& socket_path);
    // 析构函数
    ~UdsServer();

    // 启动服务器监听
    void run();
    // 停止服务器
    void stop();

private:
    // 处理单个客户端连接的函数
    void handle_client(int client_fd);

    std::string socket_path_;
    int server_fd_;
    std::atomic<bool> is_running_;
    std::vector<std::thread> client_threads_;
};

#endif //CRFZITD_UDS_SERVER_H