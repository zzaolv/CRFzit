package com.crfzit.crfzit

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import com.crfzit.crfzit.ui.dashboard.DashboardScreen
import com.crfzit.crfzit.ui.theme.CRFzitTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            CRFzitTheme {
                // 直接显示我们的仪表盘界面
                DashboardScreen()
            }
        }
    }
}