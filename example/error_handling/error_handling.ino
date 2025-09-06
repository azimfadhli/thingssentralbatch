#include <thingssentralbatch.h>

const char* ssid = "MyThingssentral";
const char* password = "12345678";
const String serverURL = "http://thingssentral.io/postlong?data=";
const String userID = "your_USER_ID";

ThingsSentralBatch tsBatch(serverURL, userID);

void setup() {
  Serial.begin(115200);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
}

void loop() {
  // Add sample data
  tsBatch.addData("temperature", float(23.4));
  tsBatch.addData("humidity", 65);
  tsBatch.addData("status", "active");

  // Send data and handle errors
  ThingsSentralBatch::ErrorCode result = tsBatch.send();
  
  switch (result) {
    case ThingsSentralBatch::SUCCESS:
      Serial.println("Success: " + tsBatch.getLastError());
      break;
    case ThingsSentralBatch::ERROR_NO_DATA:
      Serial.println("Error: No data to send");
      break;
    case ThingsSentralBatch::ERROR_WIFI_DISCONNECTED:
      Serial.println("Error: WiFi disconnected");
      WiFi.reconnect();
      break;
    case ThingsSentralBatch::ERROR_HTTP_REQUEST_FAILED:
      Serial.println("Error: HTTP failed - " + tsBatch.getLastError());
      break;
    case ThingsSentralBatch::ERROR_SERVER_RESPONSE_INVALID:
      Serial.println("Error: Invalid server response");
      break;
    default:
      Serial.println("Unknown error: " + String(result));
      break;
  }

  // Display error code and description
  Serial.println("Error code: " + String(result));
  Serial.println("Description: " + ThingsSentralBatch::errorCodeToString(result));
  Serial.println("-------------------");

  delay(30000);
}