plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.android)
    // <<< 已移除: alias(libs.plugins.google.protobuf) >>>
    id("org.jetbrains.kotlin.plugin.compose")
}

android {
    namespace = "com.crfzit.crfzit"
    compileSdk = 35

    defaultConfig {
        applicationId = "com.crfzit.crfzit"
        minSdk = 30 // 保持不变
        targetSdk = 35
        versionCode = 1
        versionName = "1.0"
        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
        vectorDrawables {
            useSupportLibrary = true
        }
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
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }

    buildFeatures {
        compose = true
    }

    composeOptions {
        kotlinCompilerExtensionVersion = libs.versions.composeCompiler.get()
    }

    kotlinOptions {
        jvmTarget = "1.8"
    }

    packaging {
        resources {
            excludes += "/META-INF/{AL2.0,LGPL2.1}"
        }
    }

    // <<< 已移除: Protobuf sourceSets 配置 >>>
    // sourceSets { ... }
}

dependencies {
    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.lifecycle.runtime.ktx)
    implementation(libs.androidx.lifecycle.viewmodel.ktx)
    
    implementation(libs.androidx.activity.compose)
    implementation(platform(libs.androidx.compose.bom))
    implementation(libs.androidx.ui)
    implementation(libs.androidx.ui.graphics)
    implementation(libs.androidx.ui.tooling.preview)
    implementation(libs.androidx.material3)

    implementation(libs.androidx.lifecycle.viewmodel.compose)
    implementation(libs.androidx.annotation)

    // <<< 新增: Gson 依赖 >>>
    implementation(libs.gson)

    testImplementation(libs.junit)
    androidTestImplementation(platform(libs.androidx.compose.bom))
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
    androidTestImplementation(libs.androidx.ui.test.junit4)
    debugImplementation(libs.androidx.ui.tooling)
    debugImplementation(libs.androidx.ui.test.manifest)

    // <<< 已移除: 所有 gRPC 和 Protobuf 相关依赖 >>>
}

// <<< 已移除: 整个 Protobuf 配置块 >>>
// configure<ProtobufExtension> { ... }