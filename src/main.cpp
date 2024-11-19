#include <Arduino.h>

// Variables
const int pins[] = {0, 2, 4, 5, 9, 10, 12, 13, 14, 16};
int freeSpots = 0;

// Function declarations:
void countFreeSpots();

// Setup function
void setup() {
  Serial.begin(115200);

  // Set pins as input with pull-up resistors
  for (int pin : pins) {
    pinMode(pin, INPUT_PULLUP);
  }
}  

void loop() {
  countFreeSpots();
  Serial.println(freeSpots);
  delay(1000);
}

// function definitions:

// Function to count free parking spots
void countFreeSpots() {
  freeSpots = 0;
  for (int pin : pins) {
    if (digitalRead(pin)) ++freeSpots;
  }
}