#ifndef thingssentralbatch_h
#define thingssentralbatch_h

// Platform detection
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <HTTPClient.h>
#else
#error "Unsupported platform. This library only supports ESP8266 and ESP32"
#endif

class ThingsSentralBatch
{
public:
  enum ErrorCode
  {
    SUCCESS = 0,
    ERROR_NO_DATA,
    ERROR_WIFI_DISCONNECTED,
    ERROR_INTERNET_DISCONNECTED,
    ERROR_HTTP_REQUEST_FAILED,
    ERROR_SERVER_RESPONSE_INVALID,
    ERROR_UNKNOWN
  };

  ThingsSentralBatch(const String &serverURL, const String &userID);
  void addData(const String &nodeID, float value);
  void addData(const String &nodeID, int value);
  void addData(const String &nodeID, const String &value);
  ErrorCode send();
  void resetBuffer();
  int count() const;

  void set_default_serverURL();
  void set_serverURL(const String &serverURL);
  void set_userID(const String &serverURL);
  const String &get_serverURL() const;
  const String &get_userID() const;

  // Enhanced error handling methods
  String getLastError() const;
  static String errorCodeToString(ErrorCode code); // Converts error code t

private:
  String _serverURL;
  String _userID;
  String _dataBuffer;
  int _dataCount;
  String _lastError;

  bool checkWiFiConnection();
};

#endif