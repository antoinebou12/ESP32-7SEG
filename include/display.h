#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>

extern Adafruit_7segment matrix;
extern TaskHandle_t CountdownTaskHandle;

void startClockDisplay();
void updateDisplay(int minutesLeft, int secondsLeft);
void displayTime(unsigned long timeLeft);
void startCountdownTask(void *pvParameters);
void stopCountdown();

#endif
