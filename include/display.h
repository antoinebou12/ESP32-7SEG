#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_LEDBackpack.h>
#include <Adafruit_GFX.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

extern Adafruit_7segment matrix;
extern TaskHandle_t TimeTaskHandle;

enum TimerMode { STOPPED, COUNTDOWN, STOPWATCH };
enum TimerAction { START, STOP, PAUSE, NONE };

typedef void (*TimerCallback)(void);
typedef void (*DisplayCallback)(void);

extern unsigned long startTime;
extern int setTimeSeconds;
extern TimerMode currentMode;
extern int currentMinutes;
extern int currentSeconds;

extern SemaphoreHandle_t mutex;

void startClockDisplay();
void updateDisplay(int minutes, int seconds, DisplayCallback displayCallback);
void timeTask(void *pvParameters);
void startTimeTask(TimerMode mode, int minutes, int seconds, TimerCallback callback, DisplayCallback displayCallback);
void stopTimeTask(TimerCallback callback);
void pauseTimeTask(TimerCallback callback);
String getCurrentTimeTaskStatus();
void incrementTimeTask();
void decrementTimeTask();

#endif
