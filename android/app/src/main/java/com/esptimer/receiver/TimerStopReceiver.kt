package com.esptimer.receiver

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import androidx.localbroadcastmanager.content.LocalBroadcastManager

class TimerStopReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context?, intent: Intent?) {
        // Broadcast a custom action or use LocalBroadcastManager to notify the app
        val stopIntent = Intent(
            ACTION_STOP_TIMER)
        val timerIntent = Intent(ACTION_TIMER_FINISHED)
        val restartIntent = Intent(ACTION_TIMER_RESTART)
        val pauseIntent = Intent(ACTION_TIMER_PAUSE)
        val resumeIntent = Intent(ACTION_TIMER_RESUME)

        when (intent?.action) {
            ACTION_STOP_TIMER -> LocalBroadcastManager.getInstance(context!!).sendBroadcast(stopIntent)
            ACTION_TIMER_FINISHED -> LocalBroadcastManager.getInstance(context!!).sendBroadcast(timerIntent)
            ACTION_TIMER_RESTART -> LocalBroadcastManager.getInstance(context!!).sendBroadcast(restartIntent)
            ACTION_TIMER_PAUSE -> LocalBroadcastManager.getInstance(context!!).sendBroadcast(pauseIntent)
            ACTION_TIMER_RESUME -> LocalBroadcastManager.getInstance(context!!).sendBroadcast(resumeIntent)
        }
    }

    companion object {
        const val ACTION_STOP_TIMER: String = "com.esptimer.STOP_TIMER"
        const val ACTION_TIMER_FINISHED: String = "com.esptimer.TIMER_FINISHED"
        const val ACTION_TIMER_RESTART: String = "com.esptimer.TIMER_RESTART"
        const val ACTION_TIMER_PAUSE: String = "com.esptimer.TIMER_PAUSE"
        const val ACTION_TIMER_RESUME: String = "com.esptimer.TIMER_RESUME"
    }
}
