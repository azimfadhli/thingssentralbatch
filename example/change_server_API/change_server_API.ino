/**
 * ThingsSentralBatch Library - Dynamic Server Switching Example
 * 
 * This example demonstrates advanced usage of the ThingsSentralBatch library
 * by showing how to dynamically change the server URL during operation. It illustrates:
 * 1. Implementing multiple server endpoints (primary and backup)
 * 2. Scheduled server switching based on time intervals
 * 3. Failure-based server switching when transmission errors occur
 * 4. Building resilience into IoT applications with automatic failover
 * 
 * Compatible with both ESP32 and ESP8266 platforms.
 */

#include <thingssentralbatch.h>

// WiFi credentials
/* const char* ssid = "your_SSID";
const char* password = "your_PASSWORD"; */
const char* ssid = "MyThingssentral";
const char* password = "12345678";

// ThingsSentral configuration
const String primaryServer = "http://thingssentral.io/postlong?data=";
const String backupServer = "http://backup.thingssentral.io/postlong?data=";
const String userID = "your_USER_ID";

ThingsSentralBatch tsBatch(primaryServer, userID);

void setup() {
  Serial.begin(115200);

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
}

void loop() {
  // Add some data
  tsBatch.addData("temperature", float(23.4));  // Float value
  tsBatch.addData("humidity", 65);              // Integer value
  tsBatch.addData("status", "1");               // String value

  // Send the data
  if (tsBatch.send()) {
    Serial.println("Data sent successfully!");
  } else {
    Serial.println("Failed to send data");

    // If sending fails, try switching to backup server immediately
    tsBatch.set_serverURL(backupServer);
    Serial.println("Switched to backup server due to failure");

    // Try sending again with backup server
    if (tsBatch.send()) {
      Serial.println("Data sent successfully with backup server!");
    }
  }

  // Wait before next transmission
  delay(30000);  // Send data every 30 seconds
}