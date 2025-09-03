#include "thingssentralbatch.h"

ThingsSentralBatch::ThingsSentralBatch(const String &serverURL, const String &userID)
    : _serverURL(serverURL), _userID(userID), _dataCount(0) {}

void ThingsSentralBatch::addData(const String &nodeID, float value)
{
  addData(nodeID, String(value));
}

void ThingsSentralBatch::addData(const String &nodeID, int value)
{
  addData(nodeID, String(value));
}

void ThingsSentralBatch::addData(const String &nodeID, const String &value)
{
  if (_dataCount == 0)
  {
    _dataBuffer = "userid|" + _userID;
  }
  _dataBuffer += "@" + nodeID + "|" + value;
  _dataCount++;
}

int ThingsSentralBatch::count() const
{
  return _dataCount;
}

void ThingsSentralBatch::resetBuffer()
{
  _dataBuffer = "";
  _dataCount = 0;
}

void ThingsSentralBatch::set_serverURL(const String &serverURL = "")
{
  if (serverURL == "")
    _serverURL = "http://thingssentral.io/postlong?data=";
  else
    _serverURL = serverURL;
}

bool ThingsSentralBatch::send()
{
  if (_dataCount == 0)
    return false;

  String url = _serverURL + _dataBuffer; // Use the stored server URL
  Serial.println("thingssentral.h: " + url);

  HTTPClient http;
#ifdef ESP32
  http.begin(url);
#else
  WiFiClient client;
  http.begin(client, url);
#endif

  int httpCode = http.GET();
  String reply = http.getString();
  http.end();

  if (reply == "")
  {
    Serial.println("thingssentral.h: error, blank reply from server. Check connection.");
    return "error";
  }

  bool success = (httpCode == HTTP_CODE_OK);
  // if (success) resetBuffer();
  resetBuffer();
  return success;
}