#ifndef CRFZITD_STATE_MANAGER_H
#define CRFZITD_STATE_MANAGER_H

#include "db_manager.h"
#include "nlohmann/json.hpp"
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <mutex>

// 运行时状态，与AppConfig分离
struct AppRuntimeInfo {
    std::string package_name;
    std::string app_name; // 应用名
    int uid;
    bool is_system_app;
    
    // 配置
    AppConfig config;

    // 动态状态
    bool is_frozen = false;
    bool is_foreground = false;
    // ... 其他运行时状态
};

class StateManager {
public:
    explicit StateManager(std::shared_ptr<DbManager> db_manager);

    // 从系统中加载/刷新所有应用列表
    void refresh_app_list();
    
    // 生成用于UI仪表盘的JSON数据
    nlohmann::json get_dashboard_update_payload();
    
    // 获取所有应用信息（用于响应UI的请求）
    nlohmann::json get_all_apps_info_payload();

private:
    std::shared_ptr<DbManager> db_manager_;
    std::map<std::string, AppRuntimeInfo> managed_apps_;
    std::mutex state_mutex_;
};

#endif //CRFZITD_STATE_MANAGER_H