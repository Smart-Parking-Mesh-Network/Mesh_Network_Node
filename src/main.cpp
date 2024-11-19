#include <Arduino.h>
#include <painlessMesh.h>

// Defines
#define MESH_PREFIX     "ParkingMesh"
#define MESH_PASSWORD   "password123"
#define MESH_PORT       5555

// Variables
const int pins[] = {0, 2, 4, 5, 9, 10, 12, 13, 14, 16};
int freeSpots = 0;
const String localSection = "A"; // Set to "A" for node A, "B" for node B,...
painlessMesh mesh;

// Function declarations:
void countFreeSpots();
void sendMessage();
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);

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

// Function definitions:

// Function to count free parking spots
void countFreeSpots() {
  freeSpots = 0;
  for (int pin : pins) {
    if (digitalRead(pin)) ++freeSpots;
  }
}

// Function to broadcast free parking spots to the mesh
void sendMessage() {
  Serial.println("Message broadcasted");
}
// Callback for receiving messages from the mesh
void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Received from %u: msg=%s\n", from, msg.c_str());
}

// Callback for new connections
void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New connection established, nodeId = %u\n", nodeId);
}

// Callback for changed connections
void changedConnectionCallback() {
  Serial.println("Connections updated");
}

// Callback for adjusted node time
void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Node time adjusted. Offset = %d\n", offset);
}