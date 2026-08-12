package com.example.gbaemulator

import android.app.Activity
import android.os.Bundle
import android.widget.Button
import android.widget.FrameLayout

class MainActivity : Activity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val layout = FrameLayout(this)
        val button = Button(this).apply {
            text = "Carica ROM GBA"
        }
        layout.addView(button)
        setContentView(layout)
    }
}
