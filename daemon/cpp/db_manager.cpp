#include "db_manager.h"
#include <android/log.h>

#define LOG_TAG "crfzitd_db"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

DbManager::DbManager(const std::string& db_path)
    : db_(db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE) {
    LOGI("Database opened at %s", db_path.c_str());
    initialize_database();
}

void DbManager::initialize_database() {
    try {
        if (!db_.tableExists("app_configs")) {
            LOGI("Table 'app_configs' does not exist. Creating it.");
            db_.exec(R"(
                CREATE TABLE app_configs (
                    package_name TEXT PRIMARY KEY,
                    is_whitelisted INTEGER NOT NULL DEFAULT 0,
                    freeze_mode INTEGER NOT NULL DEFAULT 0
                )
            )");
        }
    } catch (const std::exception& e) {
        LOGE("Database initialization failed: %s", e.what());
    }
}

std::optional<AppConfig> DbManager::get_app_config(const std::string& package_name) {
    try {
        SQLite::Statement query(db_, "SELECT is_whitelisted, freeze_mode FROM app_configs WHERE package_name = ?");
        query.bind(1, package_name);

        if (query.executeStep()) {
            AppConfig config;
            config.package_name = package_name;
            config.is_whitelisted = query.getColumn(0).getInt();
            config.freeze_mode = query.getColumn(1).getInt();
            return config;
        }
    } catch (const std::exception& e) {
        LOGE("Failed to get app config for %s: %s", package_name.c_str(), e.what());
    }
    return std::nullopt;
}

bool DbManager::update_app_config(const AppConfig& config) {
    try {
        SQLite::Statement query(db_, R"(
            INSERT INTO app_configs (package_name, is_whitelisted, freeze_mode)
            VALUES (?, ?, ?)
            ON CONFLICT(package_name) DO UPDATE SET
            is_whitelisted = excluded.is_whitelisted,
            freeze_mode = excluded.freeze_mode
        )");
        query.bind(1, config.package_name);
        query.bind(2, static_cast<int>(config.is_whitelisted));
        query.bind(3, config.freeze_mode);
        query.exec();
        return true;
    } catch (const std::exception& e) {
        LOGE("Failed to update app config for %s: %s", config.package_name.c_str(), e.what());
        return false;
    }
}

std::vector<AppConfig> DbManager::get_all_app_configs() {
    std::vector<AppConfig> configs;
    try {
        SQLite::Statement query(db_, "SELECT package_name, is_whitelisted, freeze_mode FROM app_configs");
        while (query.executeStep()) {
            AppConfig config;
            config.package_name = query.getColumn(0).getString();
            config.is_whitelisted = query.getColumn(1).getInt();
            config.freeze_mode = query.getColumn(2).getInt();
            configs.push_back(config);
        }
    } catch (const std::exception& e) {
        LOGE("Failed to get all app configs: %s", e.what());
    }
    return configs;
}