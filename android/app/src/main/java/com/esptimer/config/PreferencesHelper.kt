package com.esptimer.config

import android.content.Context

class PreferencesHelper(context: Context) {
    private val sharedPreferences = context.getSharedPreferences("ESP32Preferences", Context.MODE_PRIVATE)

    var espIpAddress: String
        get() = sharedPreferences.getString("espIpAddress", "http://esptimer.local") ?: "http://esptimer.local"
        set(value) = sharedPreferences.edit().putString("espIpAddress", value).apply()
}