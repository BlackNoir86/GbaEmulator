package com.example.gbaemulator

import android.annotation.SuppressLint
import android.net.Uri
import android.os.Bundle
import android.view.MotionEvent
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

    // Maschera dei tasti GBA (Active LOW)
    private var keyState = 0x03FF

    companion object {
        init {
            System.loadLibrary("gbaemulator")
        }
    }

    private external fun nativeLoadRom(romBytes: ByteArray)
    private external fun nativeRenderFrame(surface: Any)
    private external fun nativeSetKeyState(keys: Int)

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

        setupControllerButtons()
    }

    @SuppressLint("ClickableViewAccessibility")
    private fun setupControllerButtons() {
        // Mappatura tasti: Bit 0=A, 1=B, 2=Select, 3=Start, 4=Right, 5=Left, 6=Up, 7=Down, 8=R, 9=L
        bindButton(R.id.btn_a, 0)
        bindButton(R.id.btn_b, 1)
        bindButton(R.id.btn_select, 2)
        bindButton(R.id.btn_start, 3)
        bindButton(R.id.btn_right, 4)
        bindButton(R.id.btn_left, 5)
        bindButton(R.id.btn_up, 6)
        bindButton(R.id.btn_down, 7)
        bindButton(R.id.btn_r, 8)
        bindButton(R.id.btn_l, 9)
    }

    @SuppressLint("ClickableViewAccessibility")
    private fun bindButton(buttonId: Int, bitPosition: Int) {
        val btn: Button = findViewById(buttonId) ?: return
        btn.setOnTouchListener { _, event ->
            when (event.action) {
                MotionEvent.ACTION_DOWN -> {
                    keyState = keyState and (1 shl bitPosition).inv() // Tasto premuto (bit 0)
                    nativeSetKeyState(keyState)
                    true
                }
                MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                    keyState = keyState or (1 shl bitPosition) // Tasto rilasciato (bit 1)
                    nativeSetKeyState(keyState)
                    true
                }
                else -> false
            }
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
                Thread.sleep(16) // 60 FPS
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
