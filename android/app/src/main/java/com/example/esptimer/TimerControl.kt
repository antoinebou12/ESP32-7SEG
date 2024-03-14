package com.example.esptimer

import android.annotation.SuppressLint
import android.content.Intent
import android.os.Bundle
import android.widget.Button
import android.widget.EditText
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity
import androidx.drawerlayout.widget.DrawerLayout
import com.google.android.material.navigation.NavigationView
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import okhttp3.OkHttpClient
import okhttp3.Request
import java.io.IOException
import java.util.Locale

class TimerControl : AppCompatActivity() {

    private val client = OkHttpClient()
    private val coroutineScope = CoroutineScope(Dispatchers.Main)

    @SuppressLint("MissingInflatedId")
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.timercontrol)

        val toggleButton: Button = findViewById(R.id.toggleButton)
        val numberEditText: EditText = findViewById(R.id.editTextNumber)
        val startButton: Button = findViewById(R.id.buttonStart)
        val stopButton: Button = findViewById(R.id.buttonStop)
        val presetButtons = listOf(
            findViewById<Button>(R.id.buttonPreset1),
            findViewById<Button>(R.id.buttonPreset5),
            findViewById<Button>(R.id.buttonPreset10)
        )
        val resetWifiButton: Button = findViewById(R.id.buttonResetWifi)

        startButton.setOnClickListener {
            val mode = toggleButton.text.toString().lowercase(Locale.ROOT)
            val number = numberEditText.text.toString()
            callApi("http://esptimer.local/start?mode=$mode&number=$number")
        }

        stopButton.setOnClickListener {
            callApi("http://esptimer.local/stop")
        }

        presetButtons.forEach { button ->
            button.setOnClickListener {
                val minutes = button.text.removeSuffix(" min")
                callApi("http://esptimer.local/start?mode=countdown&number=${minutes}00")
            }
        }

        resetWifiButton.setOnClickListener {
            callApi("http://esptimer.local/resetWifi")
        }

        val drawerLayout = findViewById<DrawerLayout>(R.id.drawer_layout)
        val navigationView = findViewById<NavigationView>(R.id.nav_view)

        navigationView.setNavigationItemSelectedListener { menuItem ->
            when (menuItem.itemId) {
                R.id.nav_main_activity -> {
                    // Current Activity. Close drawer.
                    drawerLayout.closeDrawers()
                    true
                }
                R.id.nav_timer_control -> {
                    // Start TimerControl Activity.
                    val intent = Intent(this, TimerControl::class.java)
                    startActivity(intent)
                    true
                }
                else -> false
            }
        }
    }

    private fun callApi(url: String) {
        coroutineScope.launch {
            try {
                val request = Request.Builder().url(url).build()
                client.newCall(request).execute().use { response ->
                    if (!response.isSuccessful) throw IOException("Unexpected code $response")

                    // Success
                    launch(Dispatchers.Main) {
                        Toast.makeText(this@TimerControl, "Success!", Toast.LENGTH_SHORT).show()
                    }
                }
            } catch (e: Exception) {
                // Error handling
                launch(Dispatchers.Main) {
                    Toast.makeText(this@TimerControl, "Error: ${e.message}", Toast.LENGTH_SHORT).show()
                }
            }
        }
    }
}
