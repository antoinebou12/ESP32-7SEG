#include "display.h"

Adafruit_7segment matrix = Adafruit_7segment();
TaskHandle_t TimeTaskHandle = NULL;

TimerMode currentMode = STOPPED;

unsigned long startTime = 0;
int setTimeSeconds = 0;

void startClockDisplay()
{
    matrix.begin(0x70);
    matrix.clear();
    matrix.writeDisplay();
    Serial.println("Display initialized.");
}

void updateDisplay(int minutes, int seconds)
{
    // Adjust seconds and minutes if seconds are 60 or morew
    minutes += seconds / 60;
    seconds %= 60;

    // Ensure the values are within the display's range
    minutes = max(0, min(minutes, 99));
    seconds = max(0, min(seconds, 59));

    // Calculate the display value and show it
    int displayValue = minutes * 100 + seconds; // Convert to MMSS format
    matrix.print(displayValue, DEC);
    matrix.drawColon(true);
    matrix.writeDisplay();
}

void timeTask(void *pvParameters)
{
    for (;;)
    {
        unsigned long now = millis();
        int elapsedTime = (now - startTime) / 1000;

        switch (currentMode)
        {
            case COUNTDOWN:
            {
                int remainingTime = max(setTimeSeconds - elapsedTime, 0);
                int remainingMinutes = remainingTime / 60;
                int remainingSeconds = remainingTime % 60;
                updateDisplay(remainingMinutes, remainingSeconds);
                if (remainingTime <= 0)
                {
                    stopTimeTask();
                }
                break;
            }
            case STOPWATCH:
            {
                int elapsedMinutes = elapsedTime / 60;
                int elapsedSeconds = elapsedTime % 60;
                updateDisplay(elapsedMinutes, elapsedSeconds);
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void startTimeTask(TimerMode mode, int minutes, int seconds)
{
    if (TimeTaskHandle != NULL)
    {
        vTaskDelete(TimeTaskHandle);
        TimeTaskHandle = NULL;
    }

    currentMode = mode;
    startTime = millis();
    setTimeSeconds = minutes * 60 + seconds;

    Serial.println("Starting time task");

    xTaskCreate(timeTask, "TimeTask", 2048, NULL, 1, &TimeTaskHandle);
}

void stopTimeTask()
{
    if (TimeTaskHandle != NULL)
    {
        vTaskDelete(TimeTaskHandle);
        TimeTaskHandle = NULL;
    }
    matrix.clear();
    matrix.writeDisplay();
    currentMode = STOPPED;
}
