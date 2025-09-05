#include "thingssentralbatch.h"

ThingsSentralBatch::ThingsSentralBatch(const String &serverURL, const String &userID)
    : _serverURL(serverURL), _userID(userID), _dataCount(0), _lastError("") {}

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

int ThingsSentralBatch::count() const { return _dataCount; }

void ThingsSentralBatch::resetBuffer()
{
  _dataBuffer = "";
  _dataCount = 0;
}

void ThingsSentralBatch::set_serverURL(const String &serverURL)
{
  if (serverURL.length() > 0 && serverURL.startsWith("http"))
  {
    _serverURL = serverURL;
  }
  else
  {
    // Handle invalid URL, throw exception, or use default
    _serverURL = "http://thingssentral.io/postlong?data=";
  }
}

void ThingsSentralBatch::set_default_serverURL()
{
  _serverURL = "http://thingssentral.io/postlong?data=";
}

void ThingsSentralBatch::set_userID(const String &userID)
{
  if (userID.length() > 0)
  {
    _userID = userID;
  }
  else
  {
    _userID = "DefaultUserID";
  }
}

const String &ThingsSentralBatch::get_serverURL() const { return _serverURL; }
const String &ThingsSentralBatch::get_userID() const { return _userID; }
String ThingsSentralBatch::getLastError() const { return _lastError; }

String ThingsSentralBatch::c_str(ErrorCode code)
{
  switch (code)
  {
  case SUCCESS:
    return "Success: Operation completed successfully";
  case ERROR_NO_DATA:
    return "Error: No data to send. Add data before calling send()";
  case ERROR_WIFI_DISCONNECTED:
    return "Error: WiFi is not connected. Check your WiFi connection";
  case ERROR_INTERNET_DISCONNECTED:
    return "Error: No internet connection. Check your network connectivity";
  case ERROR_HTTP_REQUEST_FAILED:
    return "Error: HTTP request failed. Check server URL and network";
  case ERROR_SERVER_RESPONSE_INVALID:
    return "Error: Invalid server response. The server may be experiencing issues";
  case ERROR_UNKNOWN:
    return "Error: An unknown error occurred";
  default:
    return "Error: Invalid error code";
  }
}

bool ThingsSentralBatch::checkWiFiConnection()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    _lastError = "WiFi not connected";
    return false;
  }
  return true;
}

ThingsSentralBatch::ErrorCode ThingsSentralBatch::send()
{
  // Reset error message
  _lastError = "";

  // Check if there's data to send
  if (_dataCount == 0)
  {
    _lastError = "No data to send";
    return ERROR_NO_DATA;
  }

  // Check WiFi connection
  if (!checkWiFiConnection())
  {
    return ERROR_WIFI_DISCONNECTED;
  }

  String url = _serverURL + _dataBuffer; // Use the stored server URL
  Serial.println("thingssentral.h: " + url);

  HTTPClient http;
#ifdef ESP32
  http.begin(url);
#else
  WiFiClient client;
  http.begin(client, url);
#endif

  // Set timeout (10 seconds)
  http.setTimeout(10000);

  int httpCode = http.GET();
  String reply = http.getString();
  http.end();
  /*
    // Since HTTP code is unreliable for TS server, we'll focus on the reply content
    if (httpCode <= 0)
    {
      // HTTP request failed (timeout, connection refused, etc.)
      _lastError = "HTTP request failed with error: " + String(httpCode);
      return ERROR_HTTP_REQUEST_FAILED;
    }
   */
  if (reply == "")
  {
    _lastError = "Blank reply from server";
    return ERROR_SERVER_RESPONSE_INVALID;
  }

  // Add custom response validation here based on your server's actual behavior
  // For example, if your server returns "OK" on success:
  // if (reply.indexOf("OK") != -1) {
  //   resetBuffer();
  //   return SUCCESS;
  // }

  // For now, we'll assume any non-empty response is success
  // since the HTTP status code is unreliable
  resetBuffer();
  return SUCCESS;
}