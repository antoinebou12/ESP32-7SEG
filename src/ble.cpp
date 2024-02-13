#include "ble.h"

void initBLE() {
  BLEDevice::init("MyESP32");
}

String scanBLEDevices() {
    BLEDevice::init(""); // Initialize BLE to scan for devices
    BLEScan* pBLEScan = BLEDevice::getScan(); //Create new scan
    pBLEScan->setActiveScan(true); //Active scan uses more power, but gets results faster
    BLEScanResults foundDevices = pBLEScan->start(5, false); //Scan for 5 seconds
    String html = "<h2>BLE Devices Found:</h2><ul>";
    for (int i = 0; i < foundDevices.getCount(); i++) {
        BLEAdvertisedDevice device = foundDevices.getDevice(i);
        html += "<li>" + String(device.getName().c_str()) + " - " + String(device.getAddress().toString().c_str()) + "</li>";
    }
    html += "</ul>";
    pBLEScan->clearResults(); // clear results to release memory
    return html;
}
