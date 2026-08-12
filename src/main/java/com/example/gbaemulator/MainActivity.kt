package com.example.gbaemulator

import android.net.Uri
import android.os.Bundle
import android.widget.Button
import android.widget.FrameLayout
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    private external fun loadRomNative(romData: ByteArray): Boolean

    private val selectRomLauncher = registerForActivityResult(
        ActivityResultContracts.GetContent()
    ) { uri: Uri? ->
        uri?.let { loadRomFromUri(it) }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        val layout = FrameLayout(this)

        val button = Button(this).apply {
            text = "Carica ROM GBA"
            setOnClickListener {
                selectRomLauncher.launch("*/*")
            }
        }

        layout.addView(button)
        setContentView(layout)
    }

    private fun loadRomFromUri(uri: Uri) {
        contentResolver.openInputStream(uri)?.use { inputStream ->
            val bytes = inputStream.readBytes()
            loadRomNative(bytes)
        }
    }

    companion object {
        init {
            System.loadLibrary("gba_core")
        }
    }
}
