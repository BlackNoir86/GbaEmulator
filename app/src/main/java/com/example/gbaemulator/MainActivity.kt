package com.example.gbaemulator

import android.net.Uri
import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    private lateinit var tvStatus: TextView

    // Contratto per aprire il file picker nativo
    private val openRomLauncher = registerForActivityResult(ActivityResultContracts.GetContent()) { uri: Uri? ->
        uri?.let {
            val fileName = getFileName(it)
            tvStatus.text = "ROM Caricata:\n$fileName"
            Toast.makeText(this, "File selezionato: $fileName", Toast.LENGTH_SHORT).show()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        tvStatus = findViewById(R.id.screen_placeholder)
        val btnLoadRom: Button = findViewById(R.id.btn_load_rom)

        btnLoadRom.setOnClickListener {
            // Apre il selettore file per qualsiasi tipo di file (o binari/zip)
            openRomLauncher.launch("*/*")
        }
    }

    private fun getFileName(uri: Uri): String {
        var name = uri.lastPathSegment ?: "ROM Descritta"
        if (name.contains("/")) {
            name = name.substringAfterLast("/")
        }
        return name
    }
}
