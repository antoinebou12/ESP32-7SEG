#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>

extern Adafruit_7segment matrix;
extern TaskHandle_t TimeTaskHandle;

enum TimerMode { STOPPED, COUNTDOWN, STOPWATCH };

extern unsigned long startTime;
extern int setTimeSeconds;

void startClockDisplay();
void updateDisplay(int minutes, int seconds);
void timeTask(void *pvParameters);
void startTimeTask(TimerMode mode, int minutes, int seconds);
void stopTimeTask();
void incrementTimeTask();
void decrementTimeTask();

#endif
