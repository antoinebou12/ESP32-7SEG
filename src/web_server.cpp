#include "web_server.h"
#include "web_page.h"
#include "wifi.h"
#include "display.h"

AsyncWebServer server(80);

void handleTimerAction(AsyncWebServerRequest *request, String action, String mode, int minutes, int seconds)
{
  if (action == "start")
  {
    TimerMode timerMode = (mode == "stopwatch") ? STOPWATCH : COUNTDOWN;
    startTimeTask(timerMode, minutes, seconds);
    request->send(200, "text/plain", "Timer started or resumed.");
  }
  else if (action == "stop")
  {
    stopTimeTask();
    request->send(200, "text/plain", "Timer stopped.");
  }
  else
  {
    request->send(400, "text/plain", "Invalid action.");
  }
}

void setupWebServer()
{

  server.on("/reset-wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Resetting WiFi credentials. Device will restart.");
      initiateResetWiFiCredentials(); // Trigger the reset asynchronously
  });

  server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    String ssid = "";
    String password = "";

    if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
      ssid = request->getParam("ssid", true)->value();
      password = request->getParam("password", true)->value();

      connectToWiFi(ssid.c_str(), password.c_str());

      if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Failed to connect to Wi-Fi");
        request->redirect("/configure-wifi");
      } else {
        Serial.println("Connected to Wi-Fi");
        request->send(200, "text/plain", "Connected to Wi-Fi");
      }
    } else {
      request->redirect("/configure-wifi");
    } });

  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (!scanResults.isEmpty()) {
          request->send(200, "text/html", scanResults);
      } else {
          Serial.println("Scanning networks...");
          request->send(503, "text/plain", "Scanning in progress, please try again later.");
      }
  });


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println(WiFi.getMode() & WIFI_MODE_AP ? "AP mode" : "STA mode");
    if (WiFi.getMode() & WIFI_MODE_AP) {
      // Serve the Wi-Fi configuration page
      request->send(200, "text/html", AP_HTML);
    } else {
      request->send(200, "text/html", STA_HTML);
    } });

  server.on("/stop", HTTP_POST, [](AsyncWebServerRequest *request) {
    stopTimeTask();
    request->send(200, "text/plain", "Stopwatch stopped"); });

  server.on("/timer-action", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("action", true) || !request->hasParam("mode", true) || 
      !request->hasParam("minutes", true) || !request->hasParam("seconds", true)) {
      request->send(400, "text/plain", "Missing parameters.");
      return;
    }
    if (request->hasParam("action", true) && request->hasParam("mode", true) && request->hasParam("minutes", true) && request->hasParam("seconds", true))
    {
      String action = request->getParam("action", true)->value();
      String mode = request->getParam("mode", true)->value();
      int minutes = request->getParam("minutes", true)->value().toInt();
      int seconds = request->getParam("seconds", true)->value().toInt();
      handleTimerAction(request, action, mode, minutes, seconds);
    }
    else
    {
      request->send(400, "text/plain", "Missing parameters.");
    } });

  server.on("/update-display", HTTP_POST, [](AsyncWebServerRequest *request) {
    String message;
    if (request->hasParam("minutes", true) && request->hasParam("seconds", true))
    {
      // Assuming you have functions to parse the parameters
      request->getParam("minutes", true)->value().toInt();
      request->getParam("seconds", true)->value().toInt();

      // Extract minutes and seconds from the POST request
      int minutes = request->getParam("minutes", true)->value().toInt();
      int seconds = request->getParam("seconds", true)->value().toInt();

      // Update the display with the extracted minutes and seconds
      updateDisplay(minutes, seconds);

      message = "Display updated successfully.";
      request->send(200, "text/plain", message);
    } else {
      message = "Error: Missing minutes or seconds.";
      request->send(400, "text/plain", message);
    } });
  server.begin(); // Start server
}