#ifndef CRFZITD_DB_MANAGER_H
#define CRFZITD_DB_MANAGER_H

#include <string>
#include <vector>
#include <optional>
#include "nlohmann/json.hpp"
#include <SQLiteCpp/Database.h>

// 应用配置的数据结构
struct AppConfig {
    std::string package_name;
    bool is_whitelisted = false;
    int freeze_mode = 0; // 0: Cgroup, 1: SIGSTOP, etc.
};

class DbManager {
public:
    // 构造函数，传入数据库文件路径
    explicit DbManager(const std::string& db_path);

    // 获取单个应用的配置
    std::optional<AppConfig> get_app_config(const std::string& package_name);

    // 更新或插入一个应用的配置
    bool update_app_config(const AppConfig& config);

    // 获取所有应用的配置
    std::vector<AppConfig> get_all_app_configs();

private:
    // 初始化数据库，如果表不存在则创建
    void initialize_database();

    SQLite::Database db_;
};

#endif //CRFZITD_DB_MANAGER_H