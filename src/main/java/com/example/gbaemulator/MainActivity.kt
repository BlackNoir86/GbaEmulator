package com.example.gbaemulator

import android.app.Activity
import android.os.Bundle
import android.widget.Button
import android.widget.FrameLayout
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.result.contract.ActivityResultContracts

class MainActivity : ComponentActivity() {

    private external fun loadRomNative(romData: ByteArray): Boolean

    private val selectRomLauncher = registerForActivityResult(
        ActivityResultContracts.GetContent()
    ) { uri ->
        uri?.let {
            try {
                contentResolver.openInputStream(it)?.use { inputStream ->
                    val bytes = inputStream.readBytes()
                    val ok = loadRomNative(bytes)
                    Toast.makeText(this, if (ok) "ROM Caricata!" else "Errore caricamento", Toast.LENGTH_SHORT).show()
                }
            } catch (e: Exception) {
                Toast.makeText(this, "Errore lettore file", Toast.LENGTH_SHORT).show()
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        try {
            System.loadLibrary("gba_core")
        } catch (e: Throwable) {
            e.printStackTrace()
        }

        val layout = FrameLayout(this)
        val button = Button(this).apply {
            text = "Carica ROM GBA"
            setOnClickListener { selectRomLauncher.launch("*/*") }
        }
        layout.addView(button)
        setContentView(layout)
    }
}
