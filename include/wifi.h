// wifi.h
#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include "nvs_flash.h"
#include "nvs.h"


extern TaskHandle_t scanResultsTaskHandle;
extern SemaphoreHandle_t scanResultsMutex;
extern String scanResults;

bool connectToWiFi(const char *ssid, const char *password);
void scanNetworksTask(void *pvParameters);
void startScanNetworksTask();
void saveWiFiCredentials(const char *ssid, const char *password);
bool loadWiFiCredentials(String &ssid, String &password);
void resetWiFiCredentialsTask(void *pvParameters);
void initiateResetWiFiCredentials();
void setupWiFi();

#endif
