package com.esptimer.api

import okhttp3.OkHttpClient
import okhttp3.Request
import okhttp3.WebSocket
import okhttp3.WebSocketListener

class WebSocketClient(url: String, private val listener: WebSocketEventListener) {
    private val client = OkHttpClient()
    private var webSocket: WebSocket? = null
    private val request = Request.Builder().url(url).build()

    fun connect() {
        webSocket = client.newWebSocket(request, object : WebSocketListener() {
            override fun onOpen(webSocket: WebSocket, response: okhttp3.Response) {
                listener.onOpen()
            }

            override fun onMessage(webSocket: WebSocket, text: String) {
                listener.onMessage(text)
            }

            override fun onClosing(webSocket: WebSocket, code: Int, reason: String) {
                listener.onClosing(code, reason)
            }

            override fun onFailure(webSocket: WebSocket, t: Throwable, response: okhttp3.Response?) {
                listener.onFailure(t)
            }
        })
    }

    fun send(text: String) {
        webSocket?.send(text)
    }

    fun close() {
        webSocket?.close(NORMAL_CLOSURE_STATUS, "Client closed connection")
    }

    companion object {
        private const val NORMAL_CLOSURE_STATUS = 1000
    }

    interface WebSocketEventListener {
        fun onOpen()
        fun onMessage(message: String)
        fun onClosing(code: Int, reason: String)
        fun onFailure(t: Throwable)
    }
}
