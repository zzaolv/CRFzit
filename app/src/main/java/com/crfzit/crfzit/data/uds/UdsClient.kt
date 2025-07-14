package com.crfzit.crfzit.data.uds

import android.net.LocalSocket
import android.net.LocalSocketAddress
import android.util.Log
import kotlinx.coroutines.*
import kotlinx.coroutines.flow.MutableSharedFlow
import kotlinx.coroutines.flow.asSharedFlow
import java.io.IOException
import java.nio.charset.StandardCharsets

class UdsClient(private val scope: CoroutineScope) {

    private var socket: LocalSocket? = null
    private var connectionJob: Job? = null
    private val _incomingMessages = MutableSharedFlow<String>()
    val incomingMessages = _incomingMessages.asSharedFlow()

    companion object {
        private const val TAG = "UdsClient"
        private const val SOCKET_NAME = "crfzit_socket"
        private const val RECONNECT_DELAY_MS = 5000L
    }

    fun start() {
        if (connectionJob?.isActive == true) return
        connectionJob = scope.launch(Dispatchers.IO) {
            while (isActive) {
                try {
                    Log.i(TAG, "Attempting to connect to @$SOCKET_NAME...")
                    socket = LocalSocket().also {
                        it.connect(LocalSocketAddress(SOCKET_NAME, LocalSocketAddress.Namespace.ABSTRACT))
                    }
                    Log.i(TAG, "Successfully connected to daemon.")
                    listenForMessages()
                } catch (e: IOException) {
                    Log.w(TAG, "Connection failed or lost: ${e.message}. Retrying in ${RECONNECT_DELAY_MS}ms...")
                    cleanupSocket()
                }
                if (isActive) {
                    delay(RECONNECT_DELAY_MS)
                }
            }
        }
    }

    private suspend fun listenForMessages() {
        val currentSocket = socket ?: return
        val inputStream = currentSocket.inputStream
        val reader = inputStream.bufferedReader(StandardCharsets.UTF_8)
        
        try {
            reader.forEachLine { line ->
                if (line.isNotBlank()) {
                    scope.launch {
                        _incomingMessages.emit(line)
                    }
                }
            }
        } catch (e: Exception) {
            Log.e(TAG, "Error while reading from socket: ${e.message}")
        } finally {
            Log.i(TAG, "Socket read loop finished.")
            cleanupSocket()
        }
    }
    
    fun stop() {
        connectionJob?.cancel()
        connectionJob = null
        cleanupSocket()
        Log.i(TAG, "UDS client stopped.")
    }

    private fun cleanupSocket() {
        try {
            socket?.close()
        } catch (e: IOException) {
            // Ignore
        }
        socket = null
    }
}