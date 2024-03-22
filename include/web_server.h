// web_server.h
#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <esp_system.h>

extern AsyncWebServer server;
extern AsyncWebSocket ws;

void broadcastStatus();
void sendWebSocketMessage(const String &action, const String &message);
void setupWebServer();

#endif
