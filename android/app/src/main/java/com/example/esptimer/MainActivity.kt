package com.example.esptimer

import android.Manifest
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.pm.PackageManager
import android.net.wifi.WifiConfiguration
import android.net.wifi.WifiManager
import android.net.wifi.WifiNetworkSuggestion
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.EditText
import android.widget.Spinner
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import android.widget.Toast
import androidx.drawerlayout.widget.DrawerLayout
import com.google.android.material.navigation.NavigationView

class MainActivity : AppCompatActivity() {

    private lateinit var wifiManager: WifiManager
    private lateinit var ssidSpinner: Spinner
    private lateinit var passwordEditText: EditText
    private val LOCATION_PERMISSION_REQUEST = 1

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        ssidSpinner = findViewById(R.id.spinnerSSID)
        passwordEditText = findViewById(R.id.editTextPassword)
        val connectButton: Button = findViewById(R.id.buttonConnect)

        wifiManager = applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager

        if (ActivityCompat.checkSelfPermission(
                this, Manifest.permission.ACCESS_FINE_LOCATION
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            ActivityCompat.requestPermissions(
                this, arrayOf(Manifest.permission.ACCESS_FINE_LOCATION), LOCATION_PERMISSION_REQUEST
            )
        } else {
            registerReceiver(
                wifiScanReceiver, IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION)
            )
            wifiManager.startScan() // Explicit call to startScan
        }

        connectButton.setOnClickListener {
            val ssid = ssidSpinner.selectedItem.toString()
            val password = passwordEditText.text.toString()
            connectToWifi(ssid, password)
            Toast.makeText(this@MainActivity, "Attempting to connect to $ssid", Toast.LENGTH_SHORT).show()
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

    private val wifiScanReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context, intent: Intent) {
            val success = intent.getBooleanExtra(WifiManager.EXTRA_RESULTS_UPDATED, false)
            Log.d("MainActivity", "Scan success: $success")
            if (success) {
                loadWifiNetworks()
            } else {
                Log.e("MainActivity", "WiFi scan failed")
            }
        }
    }

    override fun onRequestPermissionsResult(
        requestCode: Int, permissions: Array<out String>, grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == LOCATION_PERMISSION_REQUEST && grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            registerReceiver(
                wifiScanReceiver, IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION)
            )
            wifiManager.startScan() // Again, start scanning here after permission is granted
            Toast.makeText(this, "Location permission granted", Toast.LENGTH_SHORT).show()
        } else {
            Toast.makeText(this, "Location permission not granted", Toast.LENGTH_SHORT).show()
            Log.e("MainActivity", "Location permission not granted")
        }
    }

    private fun loadWifiNetworks() {
        if (ActivityCompat.checkSelfPermission(this, Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            Toast.makeText(this, "Location permission not granted, cannot load networks", Toast.LENGTH_SHORT).show()
            Log.e("MainActivity", "Location permission not granted, cannot load networks")
            return
        }
        if (!wifiManager.isWifiEnabled) {
            Toast.makeText(this, "WiFi is not enabled, cannot load networks", Toast.LENGTH_SHORT).show()
            Log.e("MainActivity", "WiFi is not enabled, cannot load networks")
            return
        }

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            val status = wifiManager.startScan()
            if (!status) {
                Log.e("MainActivity", "WiFi scan failed")
            }
        }

        val wifiList = wifiManager.scanResults
        val ssidList = wifiList.mapNotNull { it.SSID }.filterNot { it.isBlank() }.distinct()
        Log.d("MainActivity", "Found ${ssidList.size} networks: $ssidList")
        runOnUiThread {
            val adapter = ArrayAdapter(this, android.R.layout.simple_spinner_item, ssidList).apply {
                setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item)
            }
            ssidSpinner.adapter = adapter
        }
    }

    private fun connectToWifi(networkSSID: String, networkPassword: String) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            val suggestion = WifiNetworkSuggestion.Builder().setSsid(networkSSID)
                .setWpa2Passphrase(networkPassword).build()

            val suggestionsList = listOf(suggestion)

            val status = wifiManager.addNetworkSuggestions(suggestionsList)
            if (status != WifiManager.STATUS_NETWORK_SUGGESTIONS_SUCCESS) {
                // Handle error.
                Toast.makeText(this, "Adding network suggestion failed with status: $status", Toast.LENGTH_SHORT).show()
                Log.e("WiFiConnection", "Adding network suggestion failed with status: $status")
            }
        } else {
            val config = WifiConfiguration()
            config.SSID = "\"" + networkSSID + "\""
            config.preSharedKey = "\"" + networkPassword + "\""

            val netId = wifiManager.addNetwork(config)
            wifiManager.disconnect()
            wifiManager.enableNetwork(netId, true)
            wifiManager.reconnect()
        }
    }

    override fun onDestroy() {
        super.onDestroy()
        unregisterReceiver(wifiScanReceiver)
    }
}
