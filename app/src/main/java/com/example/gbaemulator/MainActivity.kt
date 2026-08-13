package com.example.gbaemulator

import android.net.Uri
import android.os.Bundle
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.view.View
import android.widget.Button
import android.widget.LinearLayout
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity(), SurfaceHolder.Callback {

    private lateinit var surfaceView: SurfaceView
    private lateinit var loadRomLayout: LinearLayout
    private lateinit var tvStatus: TextView
    private var isRunning = false
    private var renderThread: Thread? = null

    companion object {
        init {
            System.loadLibrary("gbaemulator")
        }
    }

    private external fun nativeLoadRom(romBytes: ByteArray)
    private external fun nativeRenderFrame(surface: Any)

    private val openRomLauncher = registerForActivityResult(ActivityResultContracts.GetContent()) { uri: Uri? ->
        uri?.let { loadRom(it) }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        surfaceView = findViewById(R.id.emulator_surface)
        loadRomLayout = findViewById(R.id.load_rom_layout)
        tvStatus = findViewById(R.id.screen_placeholder)

        surfaceView.holder.addCallback(this)

        val btnLoadRom: Button = findViewById(R.id.btn_load_rom)
        btnLoadRom.setOnClickListener {
            openRomLauncher.launch("*/*")
        }
    }

    private fun loadRom(uri: Uri) {
        try {
            contentResolver.openInputStream(uri)?.use { inputStream ->
                val romBytes = inputStream.readBytes()
                nativeLoadRom(romBytes)

                val fileName = getFileName(uri)
                Toast.makeText(this, "Caricata in C++: $fileName", Toast.LENGTH_SHORT).show()

                loadRomLayout.visibility = View.GONE
                surfaceView.visibility = View.VISIBLE
                
                startEmulatorLoop()
            }
        } catch (e: Exception) {
            Toast.makeText(this, "Errore: ${e.message}", Toast.LENGTH_LONG).show()
        }
    }

    private fun startEmulatorLoop() {
        isRunning = true
        renderThread = Thread {
            while (isRunning) {
                if (surfaceView.holder.surface.isValid) {
                    nativeRenderFrame(surfaceView.holder.surface)
                }
                Thread.sleep(16) // ~60 FPS
            }
        }
        renderThread?.start()
    }

    override fun surfaceCreated(holder: SurfaceHolder) {}
    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {}
    override fun surfaceDestroyed(holder: SurfaceHolder) {
        isRunning = false
    }

    private fun getFileName(uri: Uri): String {
        var name = uri.lastPathSegment ?: "ROM GBA"
        if (name.contains("/")) {
            name = name.substringAfterLast("/")
        }
        return name
    }
}
