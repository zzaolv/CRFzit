#include "state_manager.h"
#include <android/log.h>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <array>
#include <sstream> // <<< 新增这一行

#define LOG_TAG "crfzitd_state"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// 辅助函数：执行shell命令并获取其输出
std::string exec_shell(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}


StateManager::StateManager(std::shared_ptr<DbManager> db_manager)
    : db_manager_(std::move(db_manager)) {
    LOGI("StateManager initialized.");
    refresh_app_list();
}

void StateManager::refresh_app_list() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    LOGI("Refreshing app list...");
    
    managed_apps_.clear();
    std::string package_list_str;
    try {
        // -f: 显示apk路径; -U: 显示UID; -3: 只显示第三方应用
        package_list_str = exec_shell("pm list packages -f -U -3");
    } catch (const std::exception& e) {
        LOGE("Failed to execute 'pm list packages': %s", e.what());
        return;
    }

    std::stringstream ss(package_list_str);
    std::string line;
    while (std::getline(ss, line, '\n')) {
        // line format: package:/data/app/~~.../com.example.app-....==/base.apk=com.example.app uid:10234
        if (line.rfind("package:", 0) != 0) continue;

        size_t eq_pos = line.find('=');
        size_t uid_pos = line.find(" uid:");
        if (eq_pos == std::string::npos || uid_pos == std::string::npos) continue;

        std::string package_name = line.substr(eq_pos + 1, uid_pos - (eq_pos + 1));
        
        AppRuntimeInfo info;
        info.package_name = package_name;
        info.app_name = package_name; // 暂时用包名代替应用名
        info.uid = std::stoi(line.substr(uid_pos + 5));
        info.is_system_app = false;
        
        // 从数据库加载该应用的配置
        auto config_opt = db_manager_->get_app_config(package_name);
        if (config_opt) {
            info.config = *config_opt;
        } else {
            // 如果数据库没有，使用默认配置
            info.config.package_name = package_name;
        }

        managed_apps_[package_name] = info;
    }
    LOGI("Finished refreshing app list. Total non-system apps found: %zu", managed_apps_.size());
}


nlohmann::json StateManager::get_dashboard_update_payload() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    using json = nlohmann::json;

    json payload;
    // 暂时保留模拟的全局状态
    payload["global_stats"] = {
        {"total_cpu_usage_percent", 35.0},
        {"total_mem_kb", 8 * 1024 * 1024},
        {"avail_mem_kb", 4 * 1024 * 1024},
        {"net_down_speed_bps", 0},
        {"net_up_speed_bps", 0},
        {"active_profile_name", "常规模式"}
    };
    
    json apps_state = json::array();
    for (const auto& pair : managed_apps_) {
        const auto& info = pair.second;
        json app_json;
        app_json["package_name"] = info.package_name;
        app_json["app_name"] = info.app_name; // 实际应获取真实应用名
        app_json["display_status"] = info.is_frozen ? "FROZEN" : "BACKGROUND_ACTIVE";
        app_json["active_freeze_mode"] = info.config.is_whitelisted ? nullptr : "CGROUP";
        app_json["mem_usage_kb"] = 0; // 暂不实现
        app_json["cpu_usage_percent"] = 0.0; // 暂不实现
        app_json["is_whitelisted"] = info.config.is_whitelisted;
        app_json["is_foreground"] = info.is_foreground;
        apps_state.push_back(app_json);
    }
    payload["apps_runtime_state"] = apps_state;
    return payload;
}

nlohmann::json StateManager::get_all_apps_info_payload() {
    std::lock_guard<std::mutex> lock(state_mutex_);
    using json = nlohmann::json;

    json apps_info = json::array();
    for (const auto& pair : managed_apps_) {
        const auto& info = pair.second;
        json app_json;
        app_json["package_name"] = info.package_name;
        app_json["app_name"] = info.app_name;
        app_json["is_whitelisted"] = info.config.is_whitelisted;
        app_json["freeze_mode"] = info.config.freeze_mode;
        app_json["is_system_app"] = info.is_system_app;
        apps_info.push_back(app_json);
    }
    return apps_info;
}