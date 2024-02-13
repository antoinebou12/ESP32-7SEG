#include <Arduino.h>
//#include "ble.h"
#include "wifi.h"
#include "web_server.h"
#include "display.h"

void setup() {
  Serial.begin(115200);
  connectToWiFi();    // Connect to WiFi
  setupWebServer();   // Setup web server
  startClockDisplay(); // Start clock display
}

void loop() {
  server.handleClient(); // Handle web server clients
}
