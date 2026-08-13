package com.example.gbaemulator

import android.net.Uri
import android.os.Bundle
import android.view.SurfaceHolder
import android.view.SurfaceView
import android.widget.Button
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import java.io.File
import java.io.FileOutputStream

class MainActivity : AppCompatActivity(), SurfaceHolder.Callback {

    private lateinit var surfaceView: SurfaceView
    private var isSurfaceReady = false
    private var selectedRomPath: String? = null

    private external fun nativeLoadRomPath(path: String): Boolean
    private external fun nativeSetKeyState(keys: Int)
    private external fun nativeRenderFrame(surface: Any)

    companion object {
        init {
            System.loadLibrary("gbaemulator")
        }
    }

    private val openDocumentLauncher = registerForActivityResult(ActivityResultContracts.OpenDocument()) { uri: Uri? ->
        uri?.let {
            val localRomFile = File(filesDir, "current_game.gba")
            contentResolver.openInputStream(it)?.use { input ->
                FileOutputStream(localRomFile).use { output ->
                    input.copyTo(output)
                }
            }
            selectedRomPath = localRomFile.absolutePath
            if (isSurfaceReady) {
                nativeLoadRomPath(localRomFile.absolutePath)
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Recupera i riferimenti espliciti al layout XML tramite R.id
        surfaceView = findViewById(R.id.surfaceView)
        surfaceView.holder.addCallback(this)

        val btnSelectRom: Button = findViewById(R.id.btnSelectRom)
        btnSelectRom.setOnClickListener {
            openDocumentLauncher.launch(arrayOf("*/*"))
        }

        startRenderLoop()
    }

    private fun startRenderLoop() {
        Thread {
            while (!isFinishing) {
                if (isSurfaceReady && selectedRomPath != null) {
                    nativeRenderFrame(surfaceView.holder.surface)
                }
                try {
                    Thread.sleep(16) // ~60 FPS
                } catch (e: InterruptedException) {
                    e.printStackTrace()
                }
            }
        }.start()
    }

    override fun surfaceCreated(holder: SurfaceHolder) {
        isSurfaceReady = true
        selectedRomPath?.let { nativeLoadRomPath(it) }
    }

    override fun surfaceChanged(holder: SurfaceHolder, format: Int, width: Int, height: Int) {}

    override fun surfaceDestroyed(holder: SurfaceHolder) {
        isSurfaceReady = false
    }
}
