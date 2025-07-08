// 文件路径: /CRFzit/app/build.gradle.kts
// 这是完整的、修改后的文件，可以直接替换使用。

plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.android)
    // compose 插件已移除，因为新版 AGP 默认包含在 buildFeatures 中
    // alias(libs.plugins.kotlin.compose)
    alias(libs.plugins.google.protobuf) // <<< 新增: 应用 Protobuf 插件
}

android {
    namespace = "com.crfzit.crfzit" // 使用您原来的 namespace
    compileSdk = 35 // <<< 修改: 遵循规格书的 API 35 (Android 15)

    defaultConfig {
        applicationId = "com.crfzit.crfzit"
        minSdk = 30 // <<< 修改: 遵循规格书的 minSdk
        targetSdk = 35 // <<< 修改: 遵循规格书的 targetSdk
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8 // <<< 修改: gRPC 依赖需要 Java 8
        targetCompatibility = JavaVersion.VERSION_1_8 // <<< 修改: gRPC 依赖需要 Java 8
    }
    kotlinOptions {
        jvmTarget = "1.8" // <<< 修改: gRPC 依赖需要 Java 8
    }
    buildFeatures {
        compose = true
    }
    composeOptions { // <<< 新增: 明确指定 compose 编译器版本
        kotlinCompilerExtensionVersion = "1.5.1" // 请根据你的Kotlin版本选择合适的编译器版本
    }
    packaging { // <<< 新增: 避免 gRPC 依赖中的重复文件问题
        resources {
            excludes += "/META-INF/{AL2.0,LGPL2.1}"
        }
    }
    // <<< 新增: 告诉 Gradle 在打包APK时包含我们的原生库 (crfzitd)
    sourceSets {
        getByName("main") {
            resources.srcDirs("src/main/resources")
        }
    }
}

dependencies {
    // 原始依赖保持不变
    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.lifecycle.runtime.ktx)
    implementation(libs.androidx.activity.compose)
    implementation(platform(libs.androidx.compose.bom))
    implementation(libs.androidx.ui)
    implementation(libs.androidx.ui.graphics)
    implementation(libs.androidx.ui.tooling.preview)
    implementation(libs.androidx.material3)
    testImplementation(libs.junit)
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
    androidTestImplementation(platform(libs.androidx.compose.bom))
    androidTestImplementation(libs.androidx.ui.test.junit4)
    debugImplementation(libs.androidx.ui.tooling)
    debugImplementation(libs.androidx.ui.test.manifest)

    // <<< 新增: gRPC 和 Protobuf 相关依赖
    implementation(libs.grpc.okhttp)
    implementation(libs.grpc.protobuf.lite)
    implementation(libs.grpc.stub)
    implementation(libs.grpc.kotlin.stub)
    implementation(libs.protobuf.kotlin.lite)
    // AndroidX 注解库，某些 gRPC 生成的代码需要它
    implementation("androidx.annotation:annotation:1.7.1")
}

// <<< 新增: Protobuf 插件的详细配置
protobuf {
    protoc {
        // 从版本目录中获取 protoc 编译器
        artifact = libs.protoc.get().toString()
    }
    plugins {
        // 定义 java grpc 代码生成器插件
        id("grpc") {
            artifact = libs.protoc.gen.grpc.java.get().toString()
        }
        // 定义 kotlin grpc 代码生成器插件
        id("grpckt") {
            artifact = libs.protoc.gen.grpc.kotlin.get().toString()
        }
    }
    generateProtoTasks {
        all().forEach { task ->
            // 为所有任务应用插件
            task.plugins {
                id("grpc") { }
                id("grpckt") { }
            }
            // 指定也生成 Kotlin 代码
            task.builtins.create("kotlin") { }
        }
    }
    // 指定 .proto 源文件的位置
    sourceSets {
        main {
            proto {
                srcDir("../ipc/proto")
                include("**/*.proto")
            }
        }
    }
}