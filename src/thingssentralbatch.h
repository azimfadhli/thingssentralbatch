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

class ThingsSentralBatch {
  public:
    ThingsSentralBatch(const String& serverURL, const String& userID);
    void addData(const String& nodeID, float value);
    void addData(const String& nodeID, int value);
    void addData(const String& nodeID, const String& value);
    bool send();
    void resetBuffer();
    void set_serverURL(const String& serverURL);
    int count() const;

  private:
    String _serverURL;
    String _userID;
    String _dataBuffer;
    int _dataCount;
};

#endif