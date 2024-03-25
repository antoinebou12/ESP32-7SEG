package com.esptimer.api

import retrofit2.Response
import retrofit2.http.*

interface TimerApiService {

    @FormUrlEncoded
    @POST("/timer-action")
    suspend fun performTimerAction(
        @Field("action") action: String,
        @Field("mode") mode: String,
        @Field("minutes") minutes: Int,
        @Field("seconds") seconds: Int
    ): Response<String>

    @POST("/reset-wifi")
    suspend fun resetWifiCredentials(): Response<String>

    @FormUrlEncoded
    @POST("/connect")
    suspend fun connectToWiFi(
        @Field("ssid") ssid: String,
        @Field("password") password: String
    ): Response<String>

    @GET("/scan")
    suspend fun scanForNetworks(): Response<String>

    @FormUrlEncoded
    @POST("/update-display")
    suspend fun updateDisplay(
        @Field("minutes") minutes: Int,
        @Field("seconds") seconds: Int
    ): Response<String>

    @GET("/device-info")
    suspend fun getDeviceInfo(): Response<String>

    @GET("/docs")
    suspend fun getDocs(): Response<String>

}
