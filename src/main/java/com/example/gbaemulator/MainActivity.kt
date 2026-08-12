package com.example.gbaemulator

import android.net.Uri
import android.os.Bundle
import android.widget.Button
import android.widget.FrameLayout
import android.widget.Toast
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

        try {
            System.loadLibrary("gba_core")
        } catch (e: UnsatisfiedLinkError) {
            e.printStackTrace()
        }

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
        try {
            contentResolver.openInputStream(uri)?.use { inputStream ->
                val bytes = inputStream.readBytes()
                val success = loadRomNative(bytes)
                if (success) {
                    Toast.makeText(this, "ROM Caricata!", Toast.LENGTH_SHORT).show()
                }
            }
        } catch (e: Exception) {
            e.printStackTrace()
        }
    }
}
