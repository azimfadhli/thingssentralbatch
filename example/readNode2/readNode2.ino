#include <thingssentralbatch.h>
String ssid = "MyThingssentral";
String pass = "12345678";
String nodeID = "001231010101";

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  Serial.println(".");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("Wifi connected, IP ");
  Serial.println(WiFi.localIP());
}

void loop() {

  // Usage example:
  ReadResult result = readNode2(nodeID);
  if (1 || result.httpCode > 0 || result.httpCode == 404) {
    // Use result.value for just the value between pipes
    // Use result.fullResponse for the complete payload
    Serial.println("httpCode: " + String(result.httpCode));
    Serial.println("Value: " + result.value);
    Serial.println("Full response: " + result.fullResponse);
  }

  delay(10000);
}
