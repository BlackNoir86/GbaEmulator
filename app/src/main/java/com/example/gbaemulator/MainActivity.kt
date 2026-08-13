package com.example.gbaemulator

import android.os.Bundle
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val tv = TextView(this)
        tv.text = "GBA Emulator - Avvio Riuscito!"
        tv.textSize = 24f
        tv.setPadding(32, 64, 32, 32)
        setContentView(tv)
    }
}
