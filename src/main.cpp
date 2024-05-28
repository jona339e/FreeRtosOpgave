#include <Arduino.h>


// Define the LED pin
const int ledPin = 2;
const int switchPin = 27; // Switch pin

// Declare the semaphore handle
SemaphoreHandle_t ledSemaphore;

// Declare task handles
TaskHandle_t task1Handle = NULL;
TaskHandle_t task2Handle = NULL;
TaskHandle_t task3Handle = NULL;

// Task 1 Token
bool task1Token = false;

// Task 1 function
void task1(void *pvParameters) {
  while (1) {
    if (task1Token) {
      if (xSemaphoreTake(ledSemaphore, portMAX_DELAY) == pdTRUE) {
        Serial.println("Task 1 blinking LED");

        digitalWrite(ledPin, HIGH);
        vTaskDelay(3000 / portTICK_PERIOD_MS); // 3000 ms on
        digitalWrite(ledPin, LOW);
        vTaskDelay(7000 / portTICK_PERIOD_MS); // 7000 ms off
        xSemaphoreGive(ledSemaphore);

      }
    } else {
      vTaskDelay(100 / portTICK_PERIOD_MS); // Short delay when not active
    }
    vTaskPrioritySet(task2Handle, 2);
    vTaskPrioritySet(task1Handle, 1);
  }
}

// Task 2 function
void task2(void *pvParameters) {
  while (1) {
    if (xSemaphoreTake(ledSemaphore, portMAX_DELAY) == pdTRUE) {
      Serial.println("Task 2 blinking LED");
      for (int i = 0; i < 20; i++) {
        digitalWrite(ledPin, HIGH);
        vTaskDelay(500 / portTICK_PERIOD_MS); // 500 ms on
        digitalWrite(ledPin, LOW);
        vTaskDelay(500 / portTICK_PERIOD_MS); // 500 ms off
      }
      xSemaphoreGive(ledSemaphore);


    }
    vTaskPrioritySet(task1Handle, 2); // Set task 1 priority to 2
    vTaskPrioritySet(task2Handle, 1); // Set task 2 priority to 1
  }
}


// Task 3 function
void task3(void *pvParameters) {
  pinMode(switchPin, INPUT_PULLUP);
  while (1) {
    if (digitalRead(switchPin) == LOW) { // Assuming the switch is active LOW
    task1Token = !task1Token; // Toggle task 1's token
    Serial.println("Task state: " + String(task1Token));
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // Poll the switch every 100 ms
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Create the semaphore
  ledSemaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(ledSemaphore);

  // Create tasks
  xTaskCreate(task1, "Task 1", 2048, NULL, 1, &task1Handle);
  xTaskCreate(task2, "Task 2", 2048, NULL, 1, &task2Handle);
  xTaskCreate(task3, "Task 3", 2048, NULL, 999, &task3Handle);
}

void loop() {
  // Empty loop as tasks are running in FreeRTOS
}



/*
  Når token er 0 så køre task 2
  Når token er 1 så køre task 1 og 2
  Når Task en task er kørt sættes prioriteten ned til 1 og den anden op til 2

  dvs når token er 1 vil task 1 og 2 køre skiftevis
  når token er 0 vil kun task 2 køre
*/