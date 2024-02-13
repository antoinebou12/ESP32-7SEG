#include "web_server.h"
#include "wifi.h"
#include "display.h"

WebServer server(80);

void setupWebServer()
{

  server.on("/", HTTP_GET, []()
            {
  String html = R"(
  <!DOCTYPE html>
  <html lang="en">
  <head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>7-Segment Display Control</title>
    <style>
      body { font-family: Arial, sans-serif; margin: 20px; padding: 0; }
      .container { max-width: 600px; margin: auto; }
      input[type="number"], button { padding: 10px; }
      button { background-color: #4CAF50; color: white; border: none; cursor: pointer; }
      button:hover { background-color: #45a049; }
      .input-group { margin-bottom: 10px; }
    </style>
  </head>
  <body>
    <div class="container">
      <h2>Update 7-Segment Display</h2>
      <form action="/update-display" method="POST">
        <div class="input-group">
          <label for="minutes">Minutes:</label>
          <input type="number" id="minutes" name="minutes" min="0" max="99" required>
        </div>
        <div class="input-group">
          <label for="seconds">Seconds:</label>
          <input type="number" id="seconds" name="seconds" min="0" max="59" required>
        </div>
        <button type="submit">Update Display</button>
      </form>
    </div>
  </body>
  </html>
  )";
  server.send(200, "text/html", html); });

  server.on("/update-display", HTTP_POST, []()
            {
  if (server.hasArg("minutes") && server.hasArg("seconds")) {
    int minutes = server.arg("minutes").toInt();
    int seconds = server.arg("seconds").toInt();
    // Example of server-side validation
    if (minutes >= 0 && minutes <= 99 && seconds >= 0 && seconds <= 59) {
      updateDisplay(minutes, seconds);
      server.sendHeader("Location", "/success.html", true); // Redirect to a success page
      server.send(302, "text/plain", "");
    } else {
      server.send(400, "text/html", "Invalid input. Please enter minutes (0-99) and seconds (0-59).");
    }
  } else {
    server.send(400, "text/html", "Bad Request: Missing minutes or seconds.");
  } });

  // API to start the clock display on the 7 segment
  server.on("/10min", HTTP_GET, []()
            {
    if (CountdownTaskHandle != NULL) {
      // Ensure any existing countdown task is stopped before starting a new one
      vTaskDelete(CountdownTaskHandle);
      CountdownTaskHandle = NULL;
    }

    // Create a new task for the countdown timer
    xTaskCreate(
    startCountdownTask, // Function to run on the task
      "Countdown", // Name of the task
      10000, // Stack size
      NULL, // Task input parameter
      1, // Priority of the task
      &CountdownTaskHandle); // Task handle
    Serial.println("Clock started");
    server.send(200, "application/json", "{\"message\":\"Clock started\"}"); });

  server.on("/stop", HTTP_GET, []()
            {
    stopCountdown();
    Serial.println("Clock stopped");
    server.send(200, "application/json", "{\"message\":\"Clock stopped\"}"); });

  server.begin();
  Serial.println("HTTP server started");
}