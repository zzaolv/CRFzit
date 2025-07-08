// 文件路径: D:/CRFzit/build.gradle.kts (项目根目录)
//
// Top-level build file where you can add configuration options common to all sub-projects/modules.
plugins {
    // 声明 Android 应用插件
    alias(libs.plugins.android.application) apply false

    // 声明 Kotlin for Android 插件
    alias(libs.plugins.kotlin.android) apply false

    // <<< 新增：声明 Compose Compiler 插件 >>>
    alias(libs.plugins.kotlin.compose.compiler) apply false

    // <<< 新增：声明 Protobuf 插件 >>>
    alias(libs.plugins.google.protobuf) apply false
}