package com.esptimer.fragment

import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.net.Uri
import android.net.wifi.WifiManager
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import androidx.recyclerview.widget.LinearLayoutManager
import com.esptimer.adapter.SettingItem
import com.esptimer.adapter.SettingsAdapter
import com.esptimer.api.RetrofitInstance
import com.esptimer.databinding.FragmentSettingsBinding
import com.google.android.material.snackbar.Snackbar
import kotlinx.coroutines.launch
import org.json.JSONObject

class SettingsFragment : Fragment() {
    private var _binding: FragmentSettingsBinding? = null
    private val binding get() = _binding!!
    private val LOCATION_PERMISSION_REQUEST = 1
    private var SSID = "SSID"


    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View {
        _binding = FragmentSettingsBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        checkWifiPermissionAndFetchSSID()
        fetchDeviceInfo()
    }

    private fun fetchDeviceInfo() {
        lifecycleScope.launch {
            try {
                val response = RetrofitInstance.api.getDeviceInfo()
                if (response.isSuccessful && response.body() != null) {
                    val info = JSONObject(response.body()!!)
                    setupRecyclerView(info)
                } else {
                    showSnackbar("Failed to fetch device info")
                }
            } catch (e: Exception) {
                e.printStackTrace()
                showSnackbar("Error fetching device info: ${e.message}")
            }
        }
    }

    private fun checkWifiPermissionAndFetchSSID() {
        if (ContextCompat.checkSelfPermission(requireContext(), android.Manifest.permission.ACCESS_FINE_LOCATION) != PackageManager.PERMISSION_GRANTED) {
            requestPermissions(arrayOf(android.Manifest.permission.ACCESS_FINE_LOCATION), LOCATION_PERMISSION_REQUEST)
        } else {
            fetchWifiSSID()
        }
    }

    @Deprecated("Deprecated in Java")
    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == LOCATION_PERMISSION_REQUEST && grantResults.isNotEmpty() && grantResults[0] == PackageManager.PERMISSION_GRANTED) {

        } else {
            Snackbar.make(requireView(), "Location permission not granted, cannot load networks", Snackbar.LENGTH_SHORT).show()
        }
    }

    private fun fetchWifiSSID() {
        val wifiManager = context?.applicationContext?.getSystemService(Context.WIFI_SERVICE) as WifiManager?
        val ssid = wifiManager?.connectionInfo?.ssid?.replace("\"", "") ?: "Not connected"
        SSID = ssid
    }

    private fun setupRecyclerView(info: JSONObject) {
        val settingsList = mutableListOf<SettingItem>().apply {
            add(SettingItem("Current WiFi SSID", SSID ?: "Not connected"))
            add(SettingItem("IP Address", info.getString("IP Address")))
            add(SettingItem("WiFi Status", info.getString("WiFi Status")))
            add(SettingItem("Mode", info.getString("Mode")))
            add(SettingItem("SSID", info.getString("SSID")))
            add(SettingItem("MAC Address", info.getString("MAC Address")))
            add(SettingItem("Signal Strength", info.getString("Signal Strength")))
            add(SettingItem("Gateway", info.getString("Gateway")))
            add(SettingItem("Subnet Mask", info.getString("Subnet Mask")))
            add(SettingItem("DNS Server", info.getString("DNS Server")))
            add(SettingItem("Hostname", info.getString("Hostname")))
            add(SettingItem("Serial/Code Version", info.getString("Serial/Code Version")))
            add(SettingItem("ESP32 Chip Model", info.getString("ESP32 Chip Model")))
            add(SettingItem("ESP32 Chip Revision", info.getString("ESP32 Chip Revision")))
            add(SettingItem("Flash Chip Size", info.getString("Flash Chip Size")))
            add(SettingItem("Free Heap Space", info.getString("Free Heap Space")))
            add(SettingItem("Github Repository", "https://github.com/antoinebou12/ESP32-7SEG"))
            add(SettingItem("Reset WiFi Credentials", "Reset", action = {
                lifecycleScope.launch {
                    try {
                        val response = RetrofitInstance.api.resetWifiCredentials()
                        if (response.isSuccessful && response.body() != null) {
                            showSnackbar("WiFi credentials reset successfully")
                        } else {
                            showSnackbar("Failed to reset WiFi credentials")
                        }
                    } catch (e: Exception) {
                        e.printStackTrace()
                        showSnackbar("Error resetting WiFi credentials: ${e.message}")
                    }
                }
            }))
            add(SettingItem("Scan for Networks", "Scan", action = {
                lifecycleScope.launch {
                    try {
                        val response = RetrofitInstance.api.scanForNetworks()
                        if (response.isSuccessful && response.body() != null) {
                            showSnackbar("Scanning for networks")
                        } else {
                            showSnackbar("Failed to scan for networks")
                        }
                    } catch (e: Exception) {
                        e.printStackTrace()
                        showSnackbar("Error scanning for networks: ${e.message}")
                    }
                }
            }))
            add(SettingItem("API Documentation", "View", action = {
                val intent = Intent(Intent.ACTION_VIEW, Uri.parse("http://${info.getString("IP Address")}/docs"))
                startActivity(intent)
            }))

        }

        binding.settingsRecyclerView.layoutManager = LinearLayoutManager(context)
        binding.settingsRecyclerView.adapter = SettingsAdapter(settingsList)
    }

    private fun showSnackbar(message: String) {
        Snackbar.make(binding.root, message, Snackbar.LENGTH_LONG).show()
    }

    override fun onDestroyView() {
        super.onDestroyView()
        _binding = null
    }
}
