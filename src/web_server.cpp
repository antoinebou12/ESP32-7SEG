#include "web_server.h"
#include "web_page.h"
#include "wifi.h"
#include "display.h"

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void sendWebSocketMessage(const String& action, const String& message) {
    JsonDocument doc;
    doc["action"] = action;
    doc["message"] = message;

    String jsonString;
    serializeJson(doc, jsonString);

    ws.textAll(jsonString.c_str());
}


void handleTimerAction(AsyncWebServerRequest *request, String action, String mode, int minutes, int seconds)
{
  JsonDocument doc;
  if (action == "start")
  {
    TimerMode timerMode = (mode == "stopwatch") ? STOPWATCH : COUNTDOWN;
    startTimeTask(timerMode, minutes, seconds, [](){
          JsonDocument msgDoc;
          msgDoc["type"] = "timerStarted";
          msgDoc["data"]["minutes"] = currentMinutes;
          msgDoc["data"]["seconds"] = currentSeconds;
          msgDoc["data"]["mode"] = currentMode == COUNTDOWN ? "countdown" : "stopwatch";
          String jsonString;
          serializeJson(msgDoc, jsonString);
          sendWebSocketMessage("timerStarted", jsonString);
        },
        [](){
          JsonDocument msgDoc;
          msgDoc["type"] = "displayUpdated";
          msgDoc["data"]["minutes"] = currentMinutes;
          msgDoc["data"]["seconds"] = currentSeconds;
          String jsonString;
          serializeJson(msgDoc, jsonString);
          sendWebSocketMessage("displayUpdated", jsonString);
        });
    doc["message"] = "Timer started or resumed.";
    sendWebSocketMessage("timerStarted", "Timer started successfully.");
  }
  else if (action == "stop" || action == "pause")
  {
    if(action == "stop") {
        stopTimeTask([]() {
          JsonDocument msgDoc;
          msgDoc["type"] = "timerStopped";
          String jsonString;
          serializeJson(msgDoc, jsonString);
          sendWebSocketMessage("timerStopped", jsonString);
        });

        doc["message"] = "Timer stopped.";
    } else if(action == "pause") {
        pauseTimeTask([]() {
          msgDoc["type"] = "timerPause";
          msgDoc["data"]["minutes"] = currentMinutes;
          msgDoc["data"]["seconds"] = currentSeconds;
          String jsonString;
          serializeJson(msgDoc, jsonString);
          sendWebSocketMessage("timerPause", jsonString);
        });
        doc["message"] = "Timer paused.";
    }
  }
  else
  {
    request->send(400, "application/json", "{\"error\":\"Invalid action.\"}");
  }
  String response;
  serializeJson(doc, response);
  request->send(200, "application/json", response);
}


void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        Serial.println("WebSocket client connected");
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.println("WebSocket client disconnected");
    } else if (type == WS_EVT_DATA) {
        // Handle incoming data
        AwsFrameInfo *info = (AwsFrameInfo*)arg;
        if (info->final && info->index == 0 && info->len == len) {
            data[len] = 0; // Ensure data is null-terminated
            // Parse JSON data
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, data);
            if (!error) {
                String action = doc["action"];
                if (action == "start" || action == "stop" || action == "pause") {
                    String mode = doc["mode"];
                    int minutes = doc["minutes"];
                    int seconds = doc["seconds"];
                } else if (action == "status") {
                } else {
                    Serial.println("Invalid action");
                }
            }
        }
    }
}


void setupWebServer()
{
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/reset-wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
      JsonDocument doc;
      doc["message"] = "Resetting Wi-Fi credentials...";
      initiateResetWiFiCredentials(); // Trigger the reset asynchronously
      String response;
      serializeJson(doc, response);
      request->send(200, "application/json", response);
  });

  server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
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
        request->redirect("/");
      }
    } else {
      request->redirect("/configure-wifi");
    } });

  server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request) {
      JsonDocument doc;
      if (!scanResults.isEmpty()) {
          request->send(200, "text/html", scanResults);
      } else {
          Serial.println("Scanning networks...");
          startScanNetworksTask();
          doc["message"] = "Scanning networks...";
      }

      String response;
      serializeJson(doc, response);
      request->send(200, "application/json", response);
  });


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println(WiFi.getMode() & WIFI_MODE_AP ? "AP mode" : "STA mode");
    if (WiFi.getMode() & WIFI_MODE_AP) {
      // Serve the Wi-Fi configuration page
      request->send(200, "text/html", AP_HTML);
    } else {
      request->send(200, "text/html", STA_HTML);
    } });

  server.on("/current-time", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    String currentTime = getCurrentTimeTaskStatus();
    doc["message"] = currentTime;
    sendWebSocketMessage("currentTime", currentTime);
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  server.on("/timer-action", HTTP_POST, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    if (!request->hasParam("action", true) || !request->hasParam("mode", true) ||
      !request->hasParam("minutes", true) || !request->hasParam("seconds", true)) {
      doc["error"] = "Missing parameters.";
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
      doc["error"] = "Missing parameters.";
    }

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
    });

  server.on("/update-display", HTTP_POST, [](AsyncWebServerRequest *request) {
    String message;
    JsonDocument doc;
    if (request->hasParam("minutes", true) && request->hasParam("seconds", true))
    {
      // Assuming you have functions to parse the parameters
      request->getParam("minutes", true)->value().toInt();
      request->getParam("seconds", true)->value().toInt();

      // Extract minutes and seconds from the POST request
      int minutes = request->getParam("minutes", true)->value().toInt();
      int seconds = request->getParam("seconds", true)->value().toInt();

      // Update the display with the extracted minutes and seconds
      updateDisplay(minutes, seconds, []() {
          JsonDocument msgDoc;
          msgDoc["type"] = "displayUpdated";
          msgDoc["data"]["minutes"] = currentMinutes;
          msgDoc["data"]["seconds"] = currentSeconds;

          String jsonString;
          serializeJson(msgDoc, jsonString);
          sendWebSocketMessage("displayUpdated", jsonString);
      });

      doc["message"] = "Display updated.";
    } else {
      doc["error"] = "Missing parameters.";
    }
    serializeJson(doc, message);
    request->send(200, "application/json", message);
    });

  server.on("/device-info", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["IP Address"] = WiFi.localIP().toString();
    doc["WiFi Status"] = WiFi.status() == WL_CONNECTED ? "Connected" : "Not Connected";
    doc["Mode"] = WiFi.getMode() & WIFI_MODE_AP ? "AP" : "STA";
    doc["SSID"] = WiFi.SSID();
    doc["MAC Address"] = WiFi.macAddress();
    doc["Signal Strength"] = String(WiFi.RSSI()) + " dBm";
    doc["Gateway"] = WiFi.gatewayIP().toString();
    doc["Subnet Mask"] = WiFi.subnetMask().toString();
    doc["DNS Server"] = WiFi.dnsIP().toString();
    doc["Hostname"] = WiFi.getHostname();
    doc["Serial/Code Version"] = "v1.0.0";
    doc["ESP32 Chip Model"] = ESP.getChipModel();
    doc["ESP32 Chip Revision"] = ESP.getChipRevision();
    doc["Flash Chip Size"] = ESP.getFlashChipSize();
    doc["Free Heap Space"] = ESP.getFreeHeap();

    sendWebSocketMessage("deviceInfo", "Device information sent.");

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  server.on("/docs", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", DOCS_HTML);
  });

  server.begin(); // Start server

  Serial.println("HTTP server started");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST");
}