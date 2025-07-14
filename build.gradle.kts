// Top-level build file where you can add configuration options common to all sub-projects/modules.
plugins {
    // 声明 Android 应用插件
    alias(libs.plugins.android.application) apply false

    // 声明 Kotlin for Android 插件
    alias(libs.plugins.kotlin.android) apply false

    // <<< 新增：在这里只声明，不应用 >>>
    id("org.jetbrains.kotlin.plugin.compose") version "2.0.0" apply false

}