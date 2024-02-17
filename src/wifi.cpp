#include "wifi.h"
#include "config.h"

TaskHandle_t scanResultsTaskHandle = NULL;
SemaphoreHandle_t scanResultsMutex = NULL;
String scanResults = "";

void resetWiFiCredentials()
{
  nvs_handle_t nvsHandle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvsHandle);
  if (err == ESP_OK)
  {
    err = nvs_erase_all(nvsHandle); // Erase all keys
    err = nvs_commit(nvsHandle);    // Commit changes
    nvs_close(nvsHandle);
  }
  if (err == ESP_OK)
  {
    Serial.println("WiFi credentials reset. Restarting in 2 seconds...");
    delay(2000); // Delay to allow message to be sent over serial
    ESP.restart();
  }
  else
  {
    Serial.println("Failed to reset WiFi credentials.");
  }
}

void initiateResetWiFiCredentials()
{
  xTaskCreate([](void *pvParameters)
              {
                resetWiFiCredentials();
                vTaskDelete(NULL); // Delete this task once done
              },
              "ResetWiFiCredsTask", 2048, NULL, 1, NULL);
}

bool loadWiFiCredentials(String &ssid, String &password)
{
  nvs_handle_t nvsHandle;
  esp_err_t err = nvs_open("storage", NVS_READONLY, &nvsHandle);
  if (err != ESP_OK)
    return false;

  size_t requiredSize;
  char *storedSsid = nullptr;
  char *storedPassword = nullptr;

  // Read the SSID
  err = nvs_get_str(nvsHandle, "ssid", NULL, &requiredSize);
  if (err == ESP_OK && requiredSize > 0)
  {
    storedSsid = new char[requiredSize];
    nvs_get_str(nvsHandle, "ssid", storedSsid, &requiredSize);
    ssid = String(storedSsid);
    delete[] storedSsid;
  }

  // Read the Password
  err = nvs_get_str(nvsHandle, "password", NULL, &requiredSize);
  if (err == ESP_OK && requiredSize > 0)
  {
    storedPassword = new char[requiredSize];
    nvs_get_str(nvsHandle, "password", storedPassword, &requiredSize);
    password = String(storedPassword);
    delete[] storedPassword;
  }

  nvs_close(nvsHandle);
  return !ssid.isEmpty() && !password.isEmpty();
}

void saveWiFiCredentials(const char *ssid, const char *password)
{
  nvs_handle_t nvsHandle;
  ESP_ERROR_CHECK(nvs_open("storage", NVS_READWRITE, &nvsHandle));
  ESP_ERROR_CHECK(nvs_set_str(nvsHandle, "ssid", ssid));
  ESP_ERROR_CHECK(nvs_set_str(nvsHandle, "password", password));
  ESP_ERROR_CHECK(nvs_commit(nvsHandle));
  nvs_close(nvsHandle);
}

bool connectToWiFi(const char *ssid, const char *password)
{
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi network: ");
  Serial.println(ssid);

  if (WiFi.waitForConnectResult() == WL_CONNECTED)
  {
    Serial.println("WiFi connected successfully");
    Serial.print("Assigned IP Address: ");
    Serial.println(WiFi.localIP());
    saveWiFiCredentials(ssid, password);
    if (MDNS.begin("ESPTimer"))
    {
      MDNS.addService("http", "tcp", 80);
      Serial.println("mDNS responder started");
    }
    else
    {
      Serial.println("Error setting up MDNS responder!");
    }
    return true;
  }

  Serial.println("Failed to connect to WiFi. Please check your credentials");
  return false;
}

void scanNetworksTask(void *pvParameters)
{
  for (;;)
  {
    if (WiFi.getMode() == WIFI_MODE_STA)
    {
      Serial.println("ESP32 in STA mode, stopping network scan");
      vTaskDelete(NULL); // Properly delete the task if not needed
    }

    Serial.println("Starting network scan...");
    int n = WiFi.scanNetworks();
    Serial.println("Scan done");

    String tempResults;
    for (int i = 0; i < n; ++i)
    {
      tempResults += "<li>" + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + "dBm)</li>";
    }

    if (tempResults.isEmpty())
    {
      tempResults = "<li>No networks found!</li>";
    }

    if (xSemaphoreTake(scanResultsMutex, (TickType_t)10) == pdTRUE)
    {
      scanResults = tempResults;
      xSemaphoreGive(scanResultsMutex);
    }

    vTaskDelay(pdMS_TO_TICKS(60000)); // Wait for 1 minute before next scan
  }
}

void startScanNetworksTask()
{
  if (scanResultsTaskHandle == NULL)
  {
    scanResultsMutex = xSemaphoreCreateMutex();
    xTaskCreate(scanNetworksTask, "ScanNetworksTask", 4096, NULL, 1, &scanResultsTaskHandle);
  }
}

void setupWiFi()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESPTimer");
  Serial.print("Access Point Started, IP Address: ");
  Serial.println(WiFi.softAPIP());

  if (MDNS.begin("ESPTimer"))
  {
    MDNS.addService("http", "tcp", 80);
    Serial.println("mDNS responder started");
  }
  else
  {
    Serial.println("Error setting up MDNS responder!");
  }

  startScanNetworksTask();
}