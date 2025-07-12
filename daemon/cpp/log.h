#ifndef CRFZIT_LOG_H
#define CRFZIT_LOG_H

#include <android/log.h>

// C++侧使用的日志宏
#define LOG_TAG "CRFzitd_CPP"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// 为Rust FFI桥提供具体的日志函数实现
namespace crfzit::cpp_log {

// 这些函数由 Rust 调用
void log_info(rust::Str tag, rust::Str message) {
    __android_log_print(ANDROID_LOG_INFO, std::string(tag).c_str(), "%s", std::string(message).c_str());
}

void log_error(rust::Str tag, rust::Str message) {
    __android_log_print(ANDROID_LOG_ERROR, std::string(tag).c_str(), "%s", std::string(message).c_str());
}

} // namespace crfzit::cpp_log

#endif //CRFZIT_LOG_H