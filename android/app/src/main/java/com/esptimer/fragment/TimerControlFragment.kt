package com.esptimer.fragment

import android.Manifest
import android.annotation.SuppressLint
import android.app.NotificationChannel
import android.app.NotificationManager
import android.app.PendingIntent
import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.content.IntentFilter
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.appcompat.widget.PopupMenu
import androidx.core.app.ActivityCompat
import androidx.core.app.NotificationCompat
import androidx.core.app.NotificationManagerCompat
import androidx.fragment.app.Fragment
import androidx.lifecycle.lifecycleScope
import androidx.localbroadcastmanager.content.LocalBroadcastManager
import com.esptimer.R
import com.esptimer.api.RetrofitInstance
import com.esptimer.api.WebSocketClient
import com.esptimer.databinding.FragmentTimerControlBinding
import com.esptimer.receiver.TimerStopReceiver
import com.google.android.material.snackbar.Snackbar
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import okhttp3.internal.notify
import org.json.JSONException
import org.json.JSONObject

class TimerControlFragment : Fragment() {
    private var _binding: FragmentTimerControlBinding? = null
    private val binding get() = _binding!!

    private val NOTIFICATION_ID = 1
    private val CHANNEL_ID = "timer_channel"

    private var timerRunning = false
    private var mode = "countdown"
    private var selectedMinutes = 0
    private var selectedSeconds = 0
    private var currentMinutes = 0
    private var currentSeconds = 0

    private val MAX_RETRY_ATTEMPTS = 3
    private val RETRY_DELAY_MS = 5000L // 5 seconds
    private var retryCount = 0

    private enum class TimerState {
        STOPPED, RUNNING, PAUSED
    }

    private var currentState = TimerState.STOPPED

    private var webSocket: WebSocketClient? = null

    private val timerStopReceiver = object : BroadcastReceiver() {
        override fun onReceive(context: Context?, intent: Intent?) {
            when (intent?.action) {
                TimerStopReceiver.ACTION_STOP_TIMER -> {
                    stopTimer()

                }
                TimerStopReceiver.ACTION_TIMER_PAUSE -> {
                    pauseTimer()
                }
                TimerStopReceiver.ACTION_TIMER_RESUME -> {
                    resumeTimer()
                }
            }
        }

    }

    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View {
        _binding = FragmentTimerControlBinding.inflate(inflater, container, false)
        return binding.root
    }

    override fun onViewCreated(view: View, savedInstanceState: Bundle?) {
        super.onViewCreated(view, savedInstanceState)
        createNotificationChannel()
        setupUI()
        setupWebSocket()
        registerStopTimerReceiver()
    }

    override fun onStart() {
        super.onStart()
        context?.let {
            LocalBroadcastManager.getInstance(it).registerReceiver(
                timerStopReceiver,
                IntentFilter().apply {
                    addAction(TimerStopReceiver.ACTION_STOP_TIMER)
                    addAction(TimerStopReceiver.ACTION_TIMER_PAUSE)
                    addAction(TimerStopReceiver.ACTION_TIMER_RESUME)
                }
            )
        }
    }

    override fun onStop() {
        super.onStop()
        context?.let { LocalBroadcastManager.getInstance(it).unregisterReceiver(timerStopReceiver) }
    }


    private fun setupUI() {
        with(binding) {
            modeSwitch.setOnCheckedChangeListener { _, isChecked ->
                mode = if (isChecked) "stopwatch" else "countdown"
                modeSwitch.text =
                    getString(if (isChecked) R.string.stopwatch else R.string.countdown)
            }

            buttonPreset1.setOnClickListener { setTimerPreset(1) }
            buttonPreset5.setOnClickListener { setTimerPreset(5) }
            buttonPreset10.setOnClickListener { setTimerPreset(10) }

            binding.fabAction.setOnClickListener {
                when (currentState) {
                    TimerState.STOPPED -> startTimer()
                    TimerState.RUNNING -> pauseTimer()
                    TimerState.PAUSED -> resumeTimer()
                }
                updateUIForCurrentState()
            }

            binding.fabAction.setOnLongClickListener { view ->
                showPopupMenu(view)
                true
            }

            setupTimePicker()
        }
    }
    private fun showPopupMenu(view: View) {
        val popup = PopupMenu(requireContext(), view)
        popup.inflate(R.menu.timer_options_menu)
        popup.setOnMenuItemClickListener { menuItem ->
            when (menuItem.itemId) {
                R.id.action_stop -> {
                    // Example: Stop the timer
                    stopTimer()
                    currentState = TimerState.STOPPED
                    updateActionButton()
                    true
                } else -> false
            }
        }
        popup.show()
    }


    private fun updateUIForCurrentState() {
        updateActionButton()
    }

    private fun resumeTimer() {
        lifecycleScope.launch {
            val response = RetrofitInstance.api.performTimerAction(
                "start",
                mode,
                selectedMinutes,
                selectedSeconds
            )
            if (response.isSuccessful) {
                showSnackbar("Timer resumed successfully!")
                currentState = TimerState.RUNNING
            } else {
                showSnackbar("Failed to resume timer: ${response.errorBody()?.string() ?: "Unknown error"}")
            }
        }
    }

    private fun setupWebSocket() {
        retryCount = 0
        connectWebSocket()
    }

    private fun connectWebSocket() {
        val wsUrl = "ws://${RetrofitInstance.BASE_URL.replace("http://", "")}/ws"
        Log.d("WebSocket", "Connecting to $wsUrl")
        webSocket = WebSocketClient(wsUrl, object : WebSocketClient.WebSocketEventListener {
            override fun onOpen() {
                Log.d("WebSocket", "Connection opened")
                retryCount = 0 // Reset retry count on successful connection
            }

            override fun onMessage(message: String) = handleWebSocketMessage(message)

            override fun onClosing(code: Int, reason: String) {
                Log.d("WebSocket", "Connection closing: $code, $reason")
            }

            override fun onFailure(t: Throwable) {
                Log.e("WebSocket", "Connection failure", t)
                if (retryCount < MAX_RETRY_ATTEMPTS) {
                    retryCount++
                    Log.d("WebSocket", "Attempting to reconnect... (Attempt $retryCount/$MAX_RETRY_ATTEMPTS)")
                    lifecycleScope.launch {
                            delay(RETRY_DELAY_MS)
                        connectWebSocket() // Attempt to reconnect
                    }
                } else {
                    activity?.runOnUiThread {
                        showErrorSnackbar("WebSocket connection failed. Please try again later.")
                    }
                }
            }
        }).also { it.connect() }
    }


    private fun showErrorSnackbar(s: String) {
        activity?.runOnUiThread { Snackbar.make(binding.root, s, Snackbar.LENGTH_SHORT).show() }
    }

    private fun pauseTimer() {
        // Example API call to pause the timer on the server
        lifecycleScope.launch {
            val response = RetrofitInstance.api.performTimerAction(
                "pause",
                mode,
                selectedMinutes,
                selectedSeconds
            )
            if (response.isSuccessful) {
                showSnackbar("Timer paused successfully!")
                currentState = TimerState.PAUSED
                selectedMinutes = currentMinutes
                selectedSeconds = currentSeconds
                updateUIForCurrentState()
                showOrUpdateNotification(currentMinutes, currentSeconds)
            } else {
                showSnackbar("Failed to pause timer: ${response.errorBody()?.string() ?: "Unknown error"}")
            }
        }
    }

    private fun setTimerPreset(minutes: Int) {
        selectedMinutes = minutes
        selectedSeconds = 0
        if (!timerRunning) startTimer()
    }

    private fun startTimer() {
        Log.d("TimerControlFragment", "startTimer: $selectedMinutes:$selectedSeconds")
        lifecycleScope.launch {
            val response = RetrofitInstance.api.performTimerAction(
                "start",
                mode,
                selectedMinutes,
                selectedSeconds
            )
            Log.d("TimerControlFragment", "startTimer: $response")
            if (response.isSuccessful) {
                showSnackbar("Timer started successfully!")
                showPersistentNotification()
            } else {
                showSnackbar(
                    "Failed to start timer: ${
                        response.errorBody()?.string() ?: "Unknown error"
                    }"
                )
            }
            currentState = TimerState.RUNNING
            updateUIForCurrentState()
        }
    }

    private fun stopTimer() {
        lifecycleScope.launch {
            val response = RetrofitInstance.api.performTimerAction("stop", mode, 0, 0)
            if (response.isSuccessful) {
                showSnackbar("Timer stopped successfully!")
                cancelPersistentNotification()
            } else {
                // Continuing from the stopTimer method
                showSnackbar(
                    "Failed to stop timer: ${
                        response.errorBody()?.string() ?: "Unknown error"
                    }"
                )
            }
            currentState = TimerState.STOPPED
            selectedMinutes = 0
            selectedSeconds = 0
            updateUIForCurrentState()
        }
    }

    private fun cancelPersistentNotification() {
        with(NotificationManagerCompat.from(requireContext())) {
            cancel(NOTIFICATION_ID)
        }
    }

    @SuppressLint("DiscouragedApi")
    private fun setupTimePicker() {
        binding.timePicker.apply {
            setIs24HourView(true)
            setOnTimeChangedListener { _, hour, minute ->
                selectedMinutes = hour
                selectedSeconds = minute
                if (timerRunning) {
                    startTimer()
                }
            }
            currentHour = 0
            currentMinute = 0
            findViewById<View>(resources.getIdentifier("android:id/amPm", null, null))?.visibility =
                View.GONE
        }
    }

    private fun handleWebSocketMessage(message: String) {
        try {
            val jsonObject = JSONObject(message)
            val innerMessage = jsonObject.getString("message")
            val messageObject = JSONObject(innerMessage)
            val messageType = messageObject.getString("type")
            val data = messageObject.getJSONObject("data")
            val minutes = data.getInt("minutes")
            val seconds = data.getInt("seconds")

            activity?.runOnUiThread {
                currentMinutes = minutes
                currentSeconds = seconds
                updateCurrentTimeDisplay(minutes, seconds)
                if (messageType == "timerStop") {
                    timerRunning = false
                    currentState = TimerState.STOPPED
                    showOrUpdateNotification(0, 0)
                }
            }
        } catch (e: JSONException) {
            e.printStackTrace()
        }
    }

    private fun updateCurrentTimeDisplay(minutes: Int, seconds: Int) {
        val formattedTime = String.format("%02d:%02d", minutes, seconds)
        binding.tvCurrentTime.text = formattedTime
        // When time updates, also update the notification
        showOrUpdateNotification(minutes, seconds)
    }

    private fun createOrUpdateNotification(text: String) {
        val notification = NotificationCompat.Builder(requireContext(), CHANNEL_ID)
            .setSmallIcon(R.drawable.baseline_timer_24)
            .setContentTitle("ESP32 Timer")
            .setContentText(text)
            .setPriority(NotificationCompat.PRIORITY_DEFAULT)
            .setContentIntent(PendingIntent.getActivity(requireContext(), 0, Intent(),
                PendingIntent.FLAG_IMMUTABLE))
            .setAutoCancel(true)
            .build()

        with(NotificationManagerCompat.from(requireContext())) {
            if (ActivityCompat.checkSelfPermission(
                    requireContext(),
                    Manifest.permission.POST_NOTIFICATIONS
                ) != PackageManager.PERMISSION_GRANTED
            ) {
                return
            }
            notify(NOTIFICATION_ID, notification)
        }
    }

    private fun updateActionButton() {
        with(binding.fabAction) {
            when (currentState) {
                TimerState.STOPPED -> {
                    text = getString(R.string.start)
                    setIconResource(R.drawable.ic_baseline_play_arrow_24)
                }
                TimerState.RUNNING -> {
                    text = getString(R.string.pause)
                    setIconResource(R.drawable.baseline_motion_photos_pause_24)
                }
                TimerState.PAUSED -> {
                    text = getString(R.string.start)
                    setIconResource(R.drawable.ic_baseline_play_arrow_24)
                }
            }
        }
    }

    private fun showPersistentNotification() {
        showOrUpdateNotification(selectedMinutes, selectedSeconds)
    }

    private fun showOrUpdateNotification(minutes: Int, seconds: Int) {
        val timeText = if (currentState == TimerState.RUNNING || currentState == TimerState.PAUSED) {
            String.format("%02d:%02d left", minutes, seconds)
        } else {
            "Timer stopped."
        }

        val stopIntent = Intent(context, TimerStopReceiver::class.java).apply {
            action = TimerStopReceiver.ACTION_STOP_TIMER
            selectedMinutes = 0
            selectedSeconds = 0
            currentMinutes = 0
            currentSeconds = 0
        }
        val stopPendingIntent = PendingIntent.getBroadcast(context, 0, stopIntent, PendingIntent.FLAG_UPDATE_CURRENT)

        val pauseIntent = Intent(context, TimerStopReceiver::class.java).apply {
            action = TimerStopReceiver.ACTION_TIMER_PAUSE
            selectedMinutes = minutes
            selectedSeconds = seconds
            currentMinutes = minutes
            currentSeconds = seconds

        }
        val pausePendingIntent = PendingIntent.getBroadcast(context, 1, pauseIntent, PendingIntent.FLAG_UPDATE_CURRENT)

        val resumeIntent = Intent(context, TimerStopReceiver::class.java).apply {
            action = TimerStopReceiver.ACTION_TIMER_RESUME
        }
        val resumePendingIntent = PendingIntent.getBroadcast(context, 2, resumeIntent, PendingIntent.FLAG_UPDATE_CURRENT)

        var timerTitle = if (currentState == TimerState.RUNNING) "Timer running" else "Timer paused"
        if (currentState == TimerState.STOPPED) {
            cancelPersistentNotification()
            timerTitle = "Timer stopped"
            return
        }

        // Notification Builder
        val notificationBuilder = NotificationCompat.Builder(requireContext(), CHANNEL_ID)
            .setSmallIcon(R.drawable.baseline_notifications_24)
            .setContentTitle(timerTitle)
            .setContentText(timeText)
            .setPriority(NotificationCompat.PRIORITY_DEFAULT)
            .setOnlyAlertOnce(true)
            .setOngoing(true)

        // Conditionally add pause or resume action based on the current state
        if (currentState == TimerState.RUNNING) {
            notificationBuilder.addAction(R.drawable.baseline_motion_photos_pause_24, "Pause", pausePendingIntent)
        } else if (currentState == TimerState.PAUSED) {
            notificationBuilder.addAction(R.drawable.ic_baseline_play_arrow_24, "Resume", resumePendingIntent)
        }

        // Always add stop action
        notificationBuilder.addAction(R.drawable.ic_baseline_stop_24, "Stop", stopPendingIntent)

        with(NotificationManagerCompat.from(requireContext())) {
            if (ActivityCompat.checkSelfPermission(requireContext(), Manifest.permission.POST_NOTIFICATIONS) == PackageManager.PERMISSION_GRANTED) {
                notify(NOTIFICATION_ID, notificationBuilder.build())
            }
        }
    }


    private fun createNotificationChannel() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            val name = getString(R.string.channel_name)
            val descriptionText = getString(R.string.channel_description)
            val importance = NotificationManager.IMPORTANCE_DEFAULT
            val channel = NotificationChannel(CHANNEL_ID, name, importance).apply {
                description = descriptionText
            }
            val notificationManager: NotificationManager =
                requireActivity().getSystemService(Context.NOTIFICATION_SERVICE) as NotificationManager
            notificationManager.createNotificationChannel(channel)
        }
    }


    private fun registerStopTimerReceiver() {
        LocalBroadcastManager.getInstance(requireContext())
            .registerReceiver(timerStopReceiver, IntentFilter("com.esptimer.STOP_TIMER_ACTION"))
    }

    private fun showSnackbar(s: String) {
        activity?.runOnUiThread { Snackbar.make(binding.root, s, Snackbar.LENGTH_SHORT).show() }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        LocalBroadcastManager.getInstance(requireContext()).unregisterReceiver(timerStopReceiver)
        webSocket?.close()
        _binding = null
    }
}
