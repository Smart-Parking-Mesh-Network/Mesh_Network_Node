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

// scores
const int entranceScore = 2;  

// Struct to store received data
struct SectionData {
  int freeSpots = -1;
  int entranceScore = 0;
};

std::map<String, SectionData> receivedSpots;

// Function declarations:
void sendMessage();
int countFreeSpots();
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
// Function to broadcast free parking spots to the mesh
void sendMessage() {
  int freeSpots = countFreeSpots();

  // local Section message
  String msg = localSection + " " + String(freeSpots) + " " + String(entranceScore);

  // Append data for each section: "A 5 B 3 C 4 D 6 E 2"
  for (const auto &section : receivedSpots) {
    if (section.first != localSection && section.second.freeSpots != -1) {
      msg += " " + section.first + " " + String(section.second.freeSpots) + " " +
             String(section.second.entranceScore);
    }
  }
  mesh.sendBroadcast(msg);
  // taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5)); // Randomize broadcast interval
  Serial.println("Sending: " + msg);
}

// Function to count free parking spots
int countFreeSpots() {
  int freeSpots = 0;
  for (int pin : pins) {
    if (digitalRead(pin)) ++freeSpots;
  }
  return freeSpots;
}

// Callback for receiving messages from the mesh
void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());

  unsigned int index = 0;
  while (index < msg.length()) {
    int spaceIdx = msg.indexOf(' ', index);
    if (spaceIdx == -1) break;

    String section = msg.substring(index, spaceIdx);
    index = spaceIdx + 1;

    spaceIdx = msg.indexOf(' ', index);
    int spots = msg.substring(index, spaceIdx).toInt();
    index = spaceIdx + 1;
    
    // read entrance Score
    // Note: The End condition is for last score -> (spaceIdx == -1)
    spaceIdx = msg.indexOf(' ', index); 
    int entranceScore = (spaceIdx == -1) ? msg.substring(index).toInt() : msg.substring(index, spaceIdx).toInt();
    index = (spaceIdx == -1) ? msg.length() : spaceIdx + 1;
     
    if (section != localSection) {
      receivedSpots[section] = {spots, entranceScore};
      Serial.printf("Updated section %s: spots=%d, entrance=%d\n", section.c_str(), spots, entranceScore);
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