# ThingsSentralBatch Library

![Version](https://img.shields.io/badge/version-1.1.0-blue) 
![ESP32](https://img.shields.io/badge/ESP32-Supported-green) 
![ESP8266](https://img.shields.io/badge/ESP8266-Supported-green) 
![License](https://img.shields.io/badge/license-MIT-green)
![PlatformIO](https://img.shields.io/badge/PlatformIO-Compatible-brightgreen)

A lightweight and efficient Arduino library for sending batched sensor data from ESP32 and ESP8266 devices to the ThingsSentral IoT platform. This library simplifies the process of collecting multiple data points and sending them in a single HTTP request, reducing bandwidth usage and improving efficiency.

## ✨ Features

- 📦 **Batched Data Transmission**: Send multiple sensor readings in a single HTTP request
- 🔄 **Dynamic Server Switching**: Seamlessly switch between primary and backup servers
- 🛡️ **Robust Error Handling**: Comprehensive error codes and descriptive messages
- 🔧 **Multi-Data Type Support**: Send floats, integers, and strings with ease
- 📶 **Automatic Connection Management**: Handles WiFi connectivity and reconnection
- ⚡ **Low Memory Footprint**: Optimized for resource-constrained devices
- 🔌 **Dual Platform Support**: Fully compatible with both ESP32 and ESP8266

## 📦 Installation

### Arduino IDE
1. Download the latest release from the [Releases page](https://github.com/azimfadhli/thingssentralbatch/releases)
2. In the Arduino IDE, navigate to `Sketch > Include Library > Add .ZIP Library...`
3. Select the downloaded ZIP file

### PlatformIO
Add the library to your `platformio.ini`:
```ini
lib_deps = 
    https://github.com/azimfadhli/thingssentralbatch.git
```

## 📚 Dependencies

This library requires:
- **[ESP8266WiFi](https://github.com/esp8266/Arduino)** (for ESP8266)
- **[WiFiClientSecure](https://github.com/espressif/arduino-esp32)** (for ESP32)
- **[HTTPClient](https://github.com/espressif/arduino-esp32)** (included with ESP cores)

## 🚀 Quick Start

Here's a minimal example to get you started:

```cpp
#include <thingssentralbatch.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// ThingsSentral configuration
const String serverURL = "http://thingssentral.io/postlong?data=";
const String userID = "YOUR_USER_ID";

ThingsSentralBatch tsBatch(serverURL, userID);

void setup() {
  Serial.begin(115200);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
}

void loop() {
  // Add sample data
  tsBatch.addData("temperature", 23.4);   // Float value
  tsBatch.addData("humidity", 65);        // Integer value
  tsBatch.addData("status", "active");    // String value

  // using new send method
  int error = tsBatch.send2();
  if (!error) {
    Serial.println("Batch sent successfully!");
  } else {
    Serial.println("Failed to send batch! error code: " + String(error));
  }

  delay(30000); // Wait 30 seconds before next transmission
}
```

## 🔧 API Reference

### Constructor
```cpp
ThingsSentralBatch(const String &serverURL, const String &userID)
```
Creates a new instance with the specified server URL and user ID.

### Data Methods
```cpp
void addData(const String &nodeID, float value)
void addData(const String &nodeID, int value)
void addData(const String &nodeID, const String &value)
```
Adds data to the batch with the specified node ID.

### Transmission Method
```cpp
// using new send method
int send2();
```
Sends all batched data to the server.  Returns an intiger error code indicating success or failure type.

```cpp
ErrorCode send()  //deprecated
```
Sends all batched data to the server.  Returns an ErrorCode indicating success or failure type.


### Utility Methods
```cpp
void resetBuffer() // Clears all batched data
int count() const  // Returns number of data points in current batch
void set_serverURL(const String &serverURL) // Changes the target server URL
void set_userID(const String &userID) // Changes the user ID
String getLastError() const // Returns the last error message
static String errorCodeToString(ErrorCode code) // Converts error code to readable string
```

### Error Codes (deprecated)
The library provides these error codes:
- `SUCCESS`: Operation completed successfully
- `ERROR_NO_DATA`: No data to send
- `ERROR_WIFI_DISCONNECTED`: WiFi connection lost
- `ERROR_INTERNET_DISCONNECTED`: No internet connection
- `ERROR_HTTP_REQUEST_FAILED`: HTTP request failed
- `ERROR_SERVER_RESPONSE_INVALID`: Server returned invalid response
- `ERROR_UNKNOWN`: Unknown error occurred

## 📖 Examples

The library includes several examples to demonstrate its capabilities:

1. **Simple** (`simple.ino`): Basic example of sending batched data
2. **Error Handling** (`error_handling.ino`): Demonstrates comprehensive error handling
3. **Change Server API** (`change_server_API.ino`): Shows how to dynamically switch between servers

To use the examples:
1. Open the Arduino IDE
2. Navigate to `File > Examples > ThingsSentralBatch`
3. Select an example sketch
4. Update the WiFi credentials and ThingsSentral configuration
5. Upload to your ESP board

## 🔄 Data Format

The library formats data as a concatenated string:
```
userid|USER_ID@nodeID1|value1@nodeID2|value2@nodeID3|value3...
```

This string is appended to the server URL as a query parameter.

## 🛠️ Troubleshooting

### Common Issues

1. **WiFi Connection Problems**
   - Ensure correct WiFi credentials
   - Check WiFi signal strength

2. **Server Connection Issues**
   - Verify the server URL is correct
   - Check if the server is accessible from your network

3. **Data Not Being Sent**
   - Ensure you've called `addData()` before `send()`
   - Check the return value of `send()` and use `getLastError()` for details

4. **Memory Issues on ESP8266**
   - Reduce the number of data points per batch
   - Increase Arduino IDE's RX/T buffer size in Tools menu

### Debugging

Enable serial output to see detailed debug information:
```cpp
Serial.begin(115200);
// Library will output debugging information to Serial
```

## 🤝 Contributing

Contributions are welcome! Please feel free to submit issues, feature requests, or pull requests.

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🆘 Support

If you encounter any problems or have questions:
1. Check the [existing issues](https://github.com/azimfadhli/thingssentralbatch/issues)
2. Create a new issue with a detailed description
3. Contact: thingssentral.demo@gmail.com

## 🙏 Acknowledgments

- Thanks to the ESP8266 and ESP32 communities for their excellent work
- Inspired by the need for efficient IoT data transmission

---

Built with ❤️ by [ThingsSentral](https://github.com/azimfadhli/thingssentralbatch) for the IoT community.
