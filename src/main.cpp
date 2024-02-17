#include <Arduino.h>
#include "wifi.h"
#include "display.h"
#include "web_server.h"

void setup() {
    Serial.begin(115200);

    // Initialize NVS with error handling
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Attempt to load saved WiFi credentials
    String ssid, password;
    if (loadWiFiCredentials(ssid, password)) {
        if (!connectToWiFi(ssid.c_str(), password.c_str())) {
            Serial.println("Failed to connect with saved credentials. Switching to AP mode.");
            setupWiFi(); // Setup as AP if connection fails
        }
    } else {
        Serial.println("No stored WiFi credentials found. Setting up as AP.");
        setupWiFi(); // Setup as AP if no credentials found
    }

    setupWebServer(); // Initialize web server
    startClockDisplay(); // Initialize display
}

void loop() {
    // Main loop can be empty if all operations are handled by tasks or interrupts
}
