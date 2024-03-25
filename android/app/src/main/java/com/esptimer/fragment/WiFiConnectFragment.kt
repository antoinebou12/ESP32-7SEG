package com.esptimer.fragment

import android.Manifest
import android.annotation.SuppressLint
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.pm.PackageManager
import android.location.LocationManager
import android.net.wifi.WifiConfiguration
import android.net.wifi.WifiManager
import android.net.wifi.WifiNetworkSuggestion
import android.os.Build
import android.os.Bundle
import android.provider.Settings
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.EditText
import android.widget.Spinner
import android.widget.TextView
import androidx.annotation.RequiresApi
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import com.esptimer.R
import com.esptimer.api.RetrofitInstance
import com.google.android.material.snackbar.Snackbar
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch

class WiFiConnectFragment : Fragment() {
    private lateinit var wifiManager: WifiManager
    private lateinit var ssidSpinner: Spinner
    private lateinit var passwordEditText: EditText
    private lateinit var textViewTitle: TextView

    private val LOCATION_PERMISSION_REQUEST = 1
    private val LOCATION_SETTINGS_REQUEST = 2

    private val wifiScanReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context?, intent: Intent?) {
            val success = intent?.getBooleanExtra(WifiManager.EXTRA_RESULTS_UPDATED, false) ?: false
            if (success) {
                loadWifiNetworks()
            } else {
                Snackbar.make(requireView(), "WiFi scan failed", Snackbar.LENGTH_SHORT).show()
            }
        }
    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View =
        inflater.inflate(R.layout.fragment_wifi_connect, container, false)

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        ssidSpinner = view.findViewById(R.id.spinnerSSID)
        passwordEditText = view.findViewById(R.id.editTextPassword)
        val connectButton: Button = view.findViewById(R.id.buttonConnect)

        wifiManager =
            requireContext().applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager

        requestLocationPermission()
        connectButton.setOnClickListener {
            connectToSelectedWifi()
        }

        val resetButton: Button = view.findViewById(R.id.buttonResetWifi)
        resetButton.setOnClickListener {
            lifecycleScope.launch {
                resetWifi()
            }
        }

        textViewTitle = view.findViewById<TextView>(R.id.textViewTitle)
    }

    private fun requestLocationPermission() {
        if (ContextCompat.checkSelfPermission(
                requireContext(),
                Manifest.permission.ACCESS_FINE_LOCATION
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            requestPermissions(
                arrayOf(Manifest.permission.ACCESS_FINE_LOCATION),
                LOCATION_PERMISSION_REQUEST
            )
        } else {
            checkLocationServiceAndInitiateWifiScan()
        }
    }

    private fun checkLocationServiceAndInitiateWifiScan() {
        if (!isLocationEnabled()) {
            askToEnableLocation()
        } else {
            initiateWifiScan()
        }
    }

    private fun isLocationEnabled(): Boolean {
        val locationManager =
            requireContext().getSystemService(Context.LOCATION_SERVICE) as LocationManager
        return locationManager.isProviderEnabled(LocationManager.GPS_PROVIDER) || locationManager.isProviderEnabled(
            LocationManager.NETWORK_PROVIDER
        )
    }


    private fun askToEnableLocation() {
        // For below Android Q, navigate user to location settings
        val intent = Intent(Settings.ACTION_LOCATION_SOURCE_SETTINGS)
        startActivity(intent)
    }

    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        if (requestCode == LOCATION_SETTINGS_REQUEST) {
            // After returning from settings, you may want to check if the location is now enabled
            if (isLocationEnabled()) {
                // Location was enabled, proceed with your functionality
                initiateWifiScan()
            } else {
                // Inform the user that location is still disabled
                Snackbar.make(
                    requireView(),
                    "Location is required to scan for WiFi networks.",
                    Snackbar.LENGTH_LONG
                ).show()
            }
        }
    }

    @Deprecated("Deprecated in Java")
    override fun onRequestPermissionsResult(
        requestCode: Int,
        permissions: Array<out String>,
        grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == LOCATION_PERMISSION_REQUEST && grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            checkLocationServiceAndInitiateWifiScan()
        } else {
            Snackbar.make(
                requireView(),
                "Location permission required to scan WiFi networks.",
                Snackbar.LENGTH_LONG
            ).show()
        }
    }

    private fun initiateWifiScan() {
        if (!wifiManager.isWifiEnabled) {
            Snackbar.make(
                requireView(),
                "WiFi is disabled. Please enable WiFi.",
                Snackbar.LENGTH_LONG
            ).show()
            return
        }

        val intentFilter = IntentFilter(WifiManager.SCAN_RESULTS_AVAILABLE_ACTION)
        requireContext().registerReceiver(wifiScanReceiver, intentFilter)
        val success = wifiManager.startScan()
        if (!success) {
            // Scan initiation handling
            Snackbar.make(requireView(), "Scan initiation failed", Snackbar.LENGTH_SHORT).show()
        }
    }

    private fun loadWifiNetworks() {
        if (ActivityCompat.checkSelfPermission(
                requireContext(),
                Manifest.permission.ACCESS_FINE_LOCATION
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            return
        }
        val wifiScanList = wifiManager.scanResults
        val ssidList = wifiScanList.mapNotNull { it.SSID }.filterNot { it.isBlank() }.distinct()

        if (ssidList.isNotEmpty()) {
            val adapter = ArrayAdapter(
                requireContext(),
                android.R.layout.simple_spinner_dropdown_item,
                ssidList
            )
            ssidSpinner.adapter = adapter
        } else {
            Snackbar.make(requireView(), "No WiFi networks found.", Snackbar.LENGTH_SHORT).show()
        }
    }

    @SuppressLint("SetTextI18n")
    private fun connectToSelectedWifi() {
        val ssid = ssidSpinner.selectedItem.toString()
        val password = passwordEditText.text.toString()
        if (ssid.isBlank()) {
            Snackbar.make(
                requireView(),
                "Please select a network to connect.",
                Snackbar.LENGTH_SHORT
            ).show()
            return
        }

        // Inform the user about the connection attempt
        Snackbar.make(requireView(), "Attempting to connect to $ssid", Snackbar.LENGTH_SHORT).show()

        val connectedSSID = wifiManager.connectionInfo.ssid

        Log.d("WiFiConnectFragment", "Connected SSID: $connectedSSID")

        if (connectedSSID == "\"ESPTimer\"") {
            textViewTitle.text = "Select a WiFi network to connect"
            Log.d("WiFiConnectFragment", "Already connected to $ssid")
            lifecycleScope.launch {
                val response = RetrofitInstance.api.connectToWiFi(ssid, password)
                if (response.isSuccessful) {
                    Snackbar.make(
                        requireView(),
                        "Connection to $ssid successful.",
                        Snackbar.LENGTH_SHORT
                    ).show()
                    delay(10000)
                    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                        connectToWifiQAndAbove(ssid, password)
                    } else {
                        connectToWifiBelowQ(ssid, password)
                    }
                } else {
                    Snackbar.make(
                        requireView(),
                        "Failed to connect to $ssid.",
                        Snackbar.LENGTH_LONG
                    ).show()
                }
            }
            return
        }


    }

    @RequiresApi(Build.VERSION_CODES.Q)
    private fun connectToWifiQAndAbove(ssid: String, password: String) {
        val suggestion = WifiNetworkSuggestion.Builder()
            .setSsid(ssid)
            .setWpa2Passphrase(password)
            .build()

        val suggestionsList = listOf(suggestion)
        val status = wifiManager.addNetworkSuggestions(suggestionsList)
        if (status == WifiManager.STATUS_NETWORK_SUGGESTIONS_SUCCESS) {
            Snackbar.make(
                requireView(),
                "Connection to $ssid suggested successfully.",
                Snackbar.LENGTH_SHORT
            ).show()
        } else {
            Snackbar.make(
                requireView(),
                "Failed to suggest connection to $ssid.",
                Snackbar.LENGTH_LONG
            ).show()
        }
    }

    private fun connectToWifiBelowQ(ssid: String, password: String) {
        val wifiConfig = WifiConfiguration().apply {
            SSID = "\"$ssid\""
            preSharedKey = "\"$password\""
        }

        val netId = wifiManager.addNetwork(wifiConfig)
        wifiManager.disconnect()
        wifiManager.enableNetwork(netId, true)
        wifiManager.reconnect()

        Snackbar.make(requireView(), "Connecting to $ssid", Snackbar.LENGTH_SHORT).show()
    }

    private suspend fun resetWifi() {
        try {
            val response = RetrofitInstance.api.resetWifiCredentials()
            if (response.isSuccessful) {
                // Handle successful reset
                Snackbar.make(requireView(), "WiFi reset successful", Snackbar.LENGTH_SHORT).show()
            } else {
                // Handle failure
                Snackbar.make(requireView(), "Failed to reset WiFi", Snackbar.LENGTH_LONG).show()
            }
        } catch (e: Exception) {
            // Handle error, such as network error
            Snackbar.make(requireView(), "Failed to reset WiFi", Snackbar.LENGTH_LONG).show()
            Log.e("WiFiConnectFragment", "Error resetting WiFi", e)
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        try {
            context?.unregisterReceiver(wifiScanReceiver)
        } catch (e: IllegalArgumentException) {
            Log.e("WifiConnectFragment", "Receiver was not registered or already unregistered.", e)
        }
    }
}
