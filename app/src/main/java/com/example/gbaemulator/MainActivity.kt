package com.example.gbaemulator

import android.net.Uri
import android.os.Bundle
import android.view.SurfaceView
import android.view.View
import android.widget.Button
import android.widget.LinearLayout
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    private lateinit var surfaceView: SurfaceView
    private lateinit var loadRomLayout: LinearLayout
    private lateinit var tvStatus: TextView

    private val openRomLauncher = registerForActivityResult(ActivityResultContracts.GetContent()) { uri: Uri? ->
        uri?.let { loadRom(it) }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        surfaceView = findViewById(R.id.emulator_surface)
        loadRomLayout = findViewById(R.id.load_rom_layout)
        tvStatus = findViewById(R.id.screen_placeholder)

        val btnLoadRom: Button = findViewById(R.id.btn_load_rom)
        btnLoadRom.setOnClickListener {
            openRomLauncher.launch("*/*")
        }
    }

    private fun loadRom(uri: Uri) {
        try {
            contentResolver.openInputStream(uri)?.use { inputStream ->
                val romBytes = inputStream.readBytes()
                val fileName = getFileName(uri)
                val romSizeMb = String.format("%.2f", romBytes.size / (1024.0 * 1024.0))

                Toast.makeText(this, "Caricata: $fileName ($romSizeMb MB)", Toast.LENGTH_LONG).show()

                // Nasconde il menù di caricamento e mostra la schermata di gioco attiva
                loadRomLayout.visibility = View.GONE
                surfaceView.visibility = View.VISIBLE
            }
        } catch (e: Exception) {
            Toast.makeText(this, "Errore nel caricamento del file: ${e.message}", Toast.LENGTH_LONG).show()
        }
    }

    private fun getFileName(uri: Uri): String {
        var name = uri.lastPathSegment ?: "ROM GBA"
        if (name.contains("/")) {
            name = name.substringAfterLast("/")
        }
        return name
    }
}
