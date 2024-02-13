// ble.h
#ifndef BLE_H
#define BLE_H

#include <Arduino.h>
#include "BLEDevice.h"

void initBLE();
void connectToBLE();
String scanBLEDevices();

#endif
