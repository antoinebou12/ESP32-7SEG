#include "display.h"

Adafruit_7segment matrix = Adafruit_7segment();
TaskHandle_t TimeTaskHandle = NULL;

TimerMode currentMode = STOPPED;

unsigned long startTime = 0;
int setTimeSeconds = 0;

int currentMinutes = 0;
int currentSeconds = 0;

SemaphoreHandle_t mutex;


struct Callbacks {
    TimerCallback timerCallback;
    DisplayCallback displayCallback;
};

static Callbacks callbacks;

void startClockDisplay()
{
    matrix.begin(0x70);
    matrix.clear();
    matrix.writeDisplay();
    Serial.println("Display initialized.");
    mutex = xSemaphoreCreateMutex();
    if (mutex == NULL) {
        Serial.println("Failed to create mutex");
    }
}

void updateDisplay(int minutes, int seconds, DisplayCallback displayCallback = nullptr)
{
    // Adjust seconds and minutes if seconds are 60 or morew
    minutes += seconds / 60;
    seconds %= 60;

    // Ensure the values are within the display's range
    minutes = max(0, min(minutes, 99));
    seconds = max(0, min(seconds, 59));

    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        currentMinutes = minutes;
        currentSeconds = seconds;
        xSemaphoreGive(mutex);
    } else {
        Serial.println("Failed to take mutex");
    }


    if (displayCallback) {
        displayCallback();
    }

    // Calculate the display value and show it
    int displayValue = minutes * 100 + seconds; // Convert to MMSS format
    matrix.print(displayValue, DEC);
    matrix.drawColon(true);
    matrix.writeDisplay();
}

void timeTask(void *pvParameters)
{
    Callbacks *callbacks = reinterpret_cast<Callbacks*>(pvParameters);
    for (;;)
    {
        Serial.printf("Loop start, Mode: %d\n", currentMode);
        if (currentMode == STOPPED) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        unsigned long now = millis();
        int elapsedTime = (now - startTime) / 1000;

        switch (currentMode)
        {
            case COUNTDOWN:
            {
                int remainingTime = max(setTimeSeconds - elapsedTime, 0);
                int remainingMinutes = remainingTime / 60;
                int remainingSeconds = remainingTime % 60;
                updateDisplay(remainingMinutes, remainingSeconds, callbacks->displayCallback);
                if (remainingTime <= 0)
                {
                    stopTimeTask(callbacks->timerCallback);
                }
                break;
            }
            case STOPWATCH:
            {
                int elapsedMinutes = elapsedTime / 60;
                int elapsedSeconds = elapsedTime % 60;
                updateDisplay(elapsedMinutes, elapsedSeconds, callbacks->displayCallback);
                if (elapsedTime >= setTimeSeconds) {
                    stopTimeTask(callbacks->timerCallback);
                }
                break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void startTimeTask(TimerMode mode, int minutes, int seconds, TimerCallback callback, DisplayCallback displayCallback)
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

    Callbacks* callbacks = new Callbacks;
    callbacks->timerCallback = callback;
    callbacks->displayCallback = displayCallback;

    BaseType_t taskCreated = xTaskCreatePinnedToCore(timeTask, "TimeTask", 8192, callbacks, 1, &TimeTaskHandle, 1);

    if (taskCreated != pdPASS) {
        Serial.println("Failed to create task");
    }
}

void stopTimeTask(TimerCallback callback)
{
    if (callback) callback();
    Serial.println("Time task stopped");
    if (TimeTaskHandle != NULL)
    {
        vTaskDelete(TimeTaskHandle);
        TimeTaskHandle = NULL;
    }
    matrix.clear();
    matrix.writeDisplay();
    currentMode = STOPPED;
}

void pauseTimeTask(TimerCallback callback)
{
    if (callback) callback();
    if (TimeTaskHandle != NULL)
    {
        vTaskSuspend(TimeTaskHandle);
    }
}

String getCurrentTimeTaskStatus() {
    char buffer[6]; // MM:SS format
    int minutes, seconds;

    if (xSemaphoreTake(mutex, portMAX_DELAY) == pdTRUE) {
        minutes = currentMinutes;
        seconds = currentSeconds;
        xSemaphoreGive(mutex);
    }

    sprintf(buffer, "%02d:%02d", minutes, seconds);
    return String(buffer);
}
