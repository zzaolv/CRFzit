#ifndef CRFZITD_UDS_SERVER_H
#define CRFZITD_UDS_SERVER_H

#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>

class UdsServer {
public:
    UdsServer(const std::string& socket_path);
    ~UdsServer();

    void run();
    void stop();

    // 新增: 向所有连接的客户端广播消息
    void broadcast_message(const std::string& message);

private:
    void handle_client(int client_fd);
    void add_client(int client_fd);
    void remove_client(int client_fd);

    std::string socket_path_;
    int server_fd_;
    std::atomic<bool> is_running_;
    
    // 客户端管理
    std::vector<int> client_fds_;
    std::mutex client_mutex_;

    // 移除 client_threads_，因为 handle_client 现在在主接受线程中管理
};

#endif //CRFZITD_UDS_SERVER_H