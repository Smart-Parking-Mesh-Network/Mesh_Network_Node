#include <Arduino.h>
#include <painlessMesh.h>

// Defines
#define MESH_PREFIX     "ParkingMesh"
#define MESH_PASSWORD   "password123"
#define MESH_PORT       5555

// Variables
const int pins[] = {0, 2, 4, 5, 9, 10, 12, 13, 14, 16};
const String localSection = "A"; // Set to "A" for node A, "B" for node B,...
painlessMesh mesh;
Scheduler userScheduler;
String sendMsg = "", lastReceivedSection = "";
int lastReceivedSpots = -1; 

// Function declarations:
int countFreeSpots();
void sendMessage();
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);

// Task for sending messages
Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);

// Setup function
void setup() {
  Serial.begin(115200);
  // Set pins as input with pull-up resistors
  for (int pin : pins) {
    pinMode(pin, INPUT_PULLUP);
  }

  // Mesh initialization
  mesh.setDebugMsgTypes(ERROR | STARTUP); // Debug messages
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  // Add and enable tasks
  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}  

void loop() {
  mesh.update();
}

// Function definitions:

// Function to count free parking spots
int countFreeSpots() {
  int freeSpots = 0;
  for (int pin : pins) {
    if (digitalRead(pin)) ++freeSpots;
  }
  return freeSpots;
}

// Function to broadcast free parking spots to the mesh
void sendMessage() {
  sendMsg = localSection + " " + String(countFreeSpots());
  if (lastReceivedSection != "" && lastReceivedSpots != -1) {
    sendMsg += " " + lastReceivedSection + " " + String(lastReceivedSpots);
  }
  mesh.sendBroadcast(sendMsg);
  sendMsg = "";
  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5)); // Randomize broadcast interval
  Serial.println("Message broadcasted");
}
// Callback for receiving messages from the mesh
void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
  int index = msg.indexOf(' ');
  if (index != -1) {
    String receivedSection = msg.substring(0, index);
    int receivedSpots = msg.substring(index + 1).toInt();

    if (receivedSection != localSection && 
        (receivedSection != lastReceivedSection || receivedSpots != lastReceivedSpots)) {
      lastReceivedSection = receivedSection;
      lastReceivedSpots = receivedSpots;
    }
  }
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