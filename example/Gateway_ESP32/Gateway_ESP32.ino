#include <LittleFS.h>
#include <ArduinoJson.h>
#include <PicoMQTT.h>
#include <thingssentralbatch.h>

ThingsSentralBatch bufferInstant("http://thingssentral.io/postlong?data=", "USER123");
ThingsSentralBatch bufferBulk("http://thingssentral.io/postlong?data=", "USER123");

PicoMQTT::Server mqtt;

//////// Wifi parameter ////////////////////////////////////
String MainNetwork_SSID = "MyThingssentral";
String MainNetwork_PASS = "12345678";

// Database structure
struct database {
  bool enable;
  bool type;
  int numOfData;  // number of sensor connected to the board
  String nodeIDData[10];
  String valueData[10];
  bool onlineStatus;
  int WDTcounter;
  int WDTcounter_lasttime;
  String WDTcounterNodeID;
  String serialNumber;

  // Function to generate topic data string
  String topicData(int n) {
    return serialNumber + "/data" + String(n);
  }

  // Function to generate request data topic
  String topicReqData() {
    return serialNumber + "/ReqData";
  }

  // Function to generate request data topic
  String topicWDTcounter() {
    return serialNumber + "/WDTcounter";
  }
};

database board[9];  // Global boards array
const String boardPaths[9] = {
  "/boards/board0.json",
  "/boards/board1.json",
  "/boards/board2.json",
  "/boards/board3.json",
  "/boards/board4.json",
  "/boards/board5.json",
  "/boards/board6.json",
  "/boards/board7.json",
  "/boards/board8.json"
};

// File utilities
String readFile(fs::FS &fs, const char *path) {
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) return String();

  String content;
  while (file.available()) content += (char)file.read();
  file.close();
  return content;
}

bool writeFile(fs::FS &fs, const char *path, const char *content) {
  File file = fs.open(path, "w");
  if (!file) return false;
  bool success = file.print(content);
  file.close();
  return success;
}

// Load single board config
bool loadBoardConfig(int boardIndex) {
  String json = readFile(LittleFS, boardPaths[boardIndex].c_str());
  if (json.length() == 0) {
    Serial.printf("Failed to read board %d config\n", boardIndex);
    return false;
  }

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    Serial.printf("Board %d JSON error: %s\n", boardIndex, error.c_str());
    return false;
  }

  // Copy arrays
  for (int j = 0; j < 10; j++) {
    board[boardIndex].nodeIDData[j] = doc["nodeIDData"][j].as<String>();
    board[boardIndex].valueData[j] = doc["valueData"][j].as<String>();
  }

  // Copy scalars
  board[boardIndex].enable = doc["enable"];
  board[boardIndex].type = doc["type"];
  board[boardIndex].numOfData = doc["numOfData"];
  board[boardIndex].onlineStatus = doc["onlineStatus"];
  board[boardIndex].WDTcounter = doc["WDTcounter"];
  board[boardIndex].WDTcounter_lasttime = doc["WDTcounter_lasttime"];
  board[boardIndex].WDTcounterNodeID = doc["WDTcounterNodeID"].as<String>();
  board[boardIndex].serialNumber = doc["serialNumber"].as<String>();

  Serial.printf("Board %d loaded\n", boardIndex);
  return true;
}

// Save single board config
bool saveBoardConfig(int boardIndex) {
  JsonDocument doc;

  // Add arrays
  JsonArray nodeArray = doc.createNestedArray("nodeIDData");
  for (int j = 0; j < 10; j++) {
    nodeArray.add(board[boardIndex].nodeIDData[j]);
  }

  JsonArray valueArray = doc.createNestedArray("valueData");
  for (int j = 0; j < 10; j++) {
    valueArray.add(board[boardIndex].valueData[j]);
  }

  // Add scalars
  doc["enable"] = board[boardIndex].enable;
  doc["type"] = board[boardIndex].type;
  doc["numOfData"] = board[boardIndex].numOfData;
  doc["onlineStatus"] = board[boardIndex].onlineStatus;
  doc["WDTcounter"] = board[boardIndex].WDTcounter;
  doc["WDTcounter_lasttime"] = board[boardIndex].WDTcounter_lasttime;
  doc["WDTcounterNodeID"] = board[boardIndex].WDTcounterNodeID;
  doc["serialNumber"] = board[boardIndex].serialNumber;

  // Serialize
  String json;
  serializeJson(doc, json);

  // Save with atomic write
  String tempPath = boardPaths[boardIndex] + ".tmp";
  if (writeFile(LittleFS, tempPath.c_str(), json.c_str())) {
    LittleFS.remove(boardPaths[boardIndex]);
    LittleFS.rename(tempPath, boardPaths[boardIndex]);
    Serial.printf("Board %d saved\n", boardIndex);
    return true;
  }
  return false;
}

// Initialize filesystem and boards
bool initFilesystem() {
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS mount failed");
    return false;
  }

  // Create boards directory if needed
  if (!LittleFS.exists("/boards")) {
    LittleFS.mkdir("/boards");
  }
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\nStarting IoT Gateway...");

  if (!initFilesystem()) {
    Serial.println("FS init failed! Stopping.");
    while (1)
      ;
  }

  // Load all boards
  for (int i = 0; i < 9; i++) {
    if (!loadBoardConfig(i)) {
      Serial.printf("Creating default config for board %d\n", i);
      // Initialize with safe defaults
      for (int j = 0; j < 10; j++) {
        board[i].nodeIDData[j] = "null";
        board[i].valueData[j] = "0";
      }
      board[i].enable = false;
      board[i].type = 1;
      board[i].onlineStatus = false;
      board[i].serialNumber = "DEFAULT" + String(i);
      saveBoardConfig(i);
    }
  }

  /*--------------------------------------------------*/
  /*---------------------- WiFi ----------------------*/
  /*--------------------------------------------------*/
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(MainNetwork_SSID, MainNetwork_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("Wifi connedted, IP");
  Serial.println(WiFi.localIP());


  /*--------------------------------------------------*/
  /*-------------------- Soft AP ---------------------*/
  /*--------------------------------------------------*/
  // Setup Hotspot ssid & pass as MAC address
  String macAddress = WiFi.macAddress();
  macAddress.replace(":", "");
  String last4Digits = macAddress.substring(macAddress.length() - 4);
  String remainingDigits = macAddress.substring(0, macAddress.length() - 4);

  String ssid = "Gateway-" + last4Digits;  // ESP's Hotspot SSID
  String password = remainingDigits;       // ESP's Hotspot password

  WiFi.softAP(ssid, password);  // Set the SSID and password for the hotspot

  Serial.println("Access Point Started");
  Serial.println("SSID: " + ssid);
  Serial.println("PASS: " + password);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());


  /*--------------------------------------------------*/
  /*------------------------ MQTT --------------------*/
  /*--------------------------------------------------*/
  // Subscribe to a topic pattern and attach a callback
  mqtt.subscribe(board[0].serialNumber + "/#", [](const char *topic, const char *payload) {
    Serial.printf("Received message in topic '%s': %s\n", topic, payload);

    String topicS = topic;
    String payloadS = payload;
    int n = 0;

    for (int i = 0; i <= board[n].numOfData; i++) {
      if (topicS == board[n].topicData(i)) {
        if (board[n].type == 0) {
          bufferInstant.addData(board[n].nodeIDData[n], board[n].valueData[n]);
        } else if (board[n].type == 1) {
          bufferBulk.addData(board[n].nodeIDData[n], board[n].valueData[n]);
        }
      }
    }
  });

  mqtt.begin();
  Serial.println("mqtt.begin()");
}

// Non-blocking timer function
bool milis(unsigned long &previousTime, unsigned long interval) {
  unsigned long currentTime = millis();

  // Handle millis() overflow (occurs after ~50 days)
  if (currentTime < previousTime) {
    previousTime = currentTime;
    return false;
  }

  if (currentTime - previousTime >= interval) {
    previousTime = currentTime;  // Reset timer
    return true;
  }
  return false;
}

unsigned long task1Timer = 0, task2Timer = 0;

void loop() {
  // task 1
  if (milis(task1Timer, 1000)) {
    if (bufferInstant.count() > 0) {
      bufferInstant.send();
    }
  }
  
  // task 2
  if (milis(task2Timer, 5000)) {
    bufferBulk.send();  // Runs every 5000ms
  }
}