/**
 * ThingsSentralBatch Library - Basic Example
 * 
 * This example demonstrates the fundamental usage of the ThingsSentralBatch library
 * to send sensor data to the ThingsSentral IoT platform. It shows how to:
 * 1. Initialize the library with server URL and user ID
 * 2. Add different data types (float, integer, string)
 * 3. Send batched data to the server
 * 4. Implement periodic data transmission
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
const String serverURL = "http://thingssentral.io/postlong?data=";
const String userID = "your_USER_ID";

ThingsSentralBatch tsBatch(serverURL, userID, true);

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

  tsBatch.begin();
}

void loop() {
  // Add different types of data to the batch
  tsBatch.addData("temperature", float(23.4));  // Float value
  tsBatch.addData("humidity", 65);              // Integer value
  tsBatch.addData("status", "1");               // String value

  // Send the batch
  if (!tsBatch.send()) {
    Serial.println("Batch sent successfully!");
  } else {
    Serial.println("Failed to send batch");
  }

  delay(1000);

  tsBatch.addData("temperature", float(32.4));  // Float value
  tsBatch.addData("humidity", 86);              // Integer value
  tsBatch.addData("status", "0");               // String value

  // using new send method
  int error = tsBatch.send2();
  if (!error) {
    Serial.println("Batch sent successfully!");
  } else {
    Serial.println("Failed to send batch! error code: " + String(error));
  }

  // Wait before next transmission
  delay(60000);
}