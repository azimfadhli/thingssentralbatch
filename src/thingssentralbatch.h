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

#define BUFFER_LIMIT_MAX 500
#define BUFFER_LIMIT_DEFAULT 250
#define TIMEOUT_DEFAULT 2500
#define API_LINK_SENT_DEFAULT "http://thingssentral.io/postlong?data="
#define API_LINK_READ_DEFAULT "http://thingssentral.io/ReadNode?Params="

// Legacy Code
extern String TSuserID;
extern String APIlinkRead;
extern String APIlinkSent;
String IRAM_ATTR GET(String completedLink);
String IRAM_ATTR readNode(String __NodeID, bool _fullReply = false);
String IRAM_ATTR sendNode(String _NodeID1, String _Data1,
                          String _NodeID2 = "", String _Data2 = "",  // OPTIONAL PARAM
                          String _NodeID3 = "", String _Data3 = "",  // OPTIONAL PARAM
                          String _NodeID4 = "", String _Data4 = ""); // OPTIONAL PARAM

// new readNode
typedef struct
{
  int httpCode;
  String value;
  String fullResponse;
} ReadResult;

ReadResult IRAM_ATTR readNode2(String __NodeID);

// send Node class
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
    ERROR_UNKNOWN,
    ERROR_BLANK_NODE_ID
  };

  ThingsSentralBatch(const String &serverURL, const String &userID);
  void addData(const String &nodeID, float value);
  void addData(const String &nodeID, int value);
  void addData(const String &nodeID, const String &value);
  ErrorCode send();
  int send2();
  void resetBuffer();
  int count() const;
  ErrorCode readNode(const String &nodeID);

  void set_default_serverURL();
  void set_serverURL(const String &serverURL);
  void set_userID(const String &userID);
  void set_bufferLimit(int value);
  void set_timeOut(int value);
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
  int _bufferLimit = BUFFER_LIMIT_DEFAULT;
  int _timeOut = TIMEOUT_DEFAULT;

  bool checkWiFiConnection();
};

#endif