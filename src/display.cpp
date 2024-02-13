#include "display.h"

#include <Adafruit_LEDBackpack.h>

Adafruit_7segment matrix = Adafruit_7segment();
TaskHandle_t CountdownTaskHandle = NULL;

void startClockDisplay()
{
    // Initialize the 7 segment display
    matrix.begin(0x70);
}

void updateDisplay(int minutesLeft, int secondsLeft)
{
    matrix.writeDigitNum(0, minutesLeft / 10, false);
    matrix.writeDigitNum(1, minutesLeft % 10, true);
    matrix.writeDigitNum(3, secondsLeft / 10, false);
    matrix.writeDigitNum(4, secondsLeft % 10, false);
    matrix.writeDisplay();
}

void displayTime(unsigned long timeLeft)
{
    int minutesLeft = timeLeft / 60000;
    int secondsLeft = (timeLeft % 60000) / 1000;
    updateDisplay(minutesLeft, secondsLeft);
}

void stopCountdown()
{
    if (CountdownTaskHandle != NULL)
    {
        vTaskDelete(CountdownTaskHandle);
        CountdownTaskHandle = NULL;
    }
    matrix.clear();
    matrix.writeDisplay();
}

void startCountdownTask(void *pvParameters)
{
    const unsigned long duration = 10 * 60 * 1000; // 10 minutes
    unsigned long startTime = millis();
    unsigned long timeLeft = duration;

    while (timeLeft > 0)
    {
        unsigned long currentTime = millis();
        timeLeft = duration - (currentTime - startTime);
        displayTime(timeLeft);
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }

    stopCountdown();
    vTaskDelete(NULL); // Delete this task when done
}