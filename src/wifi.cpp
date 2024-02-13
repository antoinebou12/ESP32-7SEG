#include "wifi.h"
#include "config.h"


void connectToWiFi()
{
  Serial.print("Connecting to WiFi network: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected successfully");
  Serial.print("Assigned IP Address: ");
  Serial.println(WiFi.localIP());
}

String scanNetworks() {
  String html;
  int n = WiFi.scanNetworks();
  html += "<h2>WiFi Networks</h2><ul>";
  for (int i = 0; i < n; ++i) {
    html += "<li>" + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + " dBm)</li>";
  }
  html += "</ul>";
  return html;
}