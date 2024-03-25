package com.esptimer.api

import android.annotation.SuppressLint
import android.content.Context
import com.esptimer.config.PreferencesHelper
import retrofit2.Retrofit
import retrofit2.converter.scalars.ScalarsConverterFactory

@SuppressLint("StaticFieldLeak")
object RetrofitInstance {
    private lateinit var context: Context

    var BASE_URL = "http://esptimer.local"

    private val preferencesHelper by lazy { PreferencesHelper(context) }

    private val retrofit by lazy {
        Retrofit.Builder()
            .baseUrl(BASE_URL)
            .addConverterFactory(ScalarsConverterFactory.create())
            .build()
    }

    val api: TimerApiService by lazy {
        retrofit.create(TimerApiService::class.java)
    }
}

