#include "thingssentralbatch.h"

// helper func
void ThingsSentralBatch::write_File(const char *path, const String &message)
{
  File file = LittleFS.open(path, "w");
  if (file)
  {
    file.print(message);
    file.close();
  }
}

String ThingsSentralBatch::read_File(const char *path)
{
  File file = LittleFS.open(path, "r");
  if (file)
  {
    String data = file.readString();
    file.close();
    return data;
  }
  return ""; // Return empty if read fails
}

ThingsSentralBatch::ThingsSentralBatch(const String &serverURL, const String &userID, bool enablePersistent)
    : _serverURL(serverURL), _userID(userID), _dataCount(0), _lastError(""), _enablePersistent(enablePersistent) {}

void ThingsSentralBatch::begin()
{
  if (_enablePersistent)
  {
    // Mount file system
    if (!LittleFS.begin())
    {
      Serial.println("Mount failed");
      return;
    }
    // --- READ ---
    _dataBuffer = read_File("/tsb_databuffer.txt");
    _dataCount = read_File("/tsb_datacount.txt").toInt();
  }
}

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
  if (nodeID == "" || value == "")
  {
    Serial.println("nodeID or value is null");
    return;
  }

  if (_dataCount == 0)
  {
    _dataBuffer = "userid|" + _userID;
  }

  if (_dataCount > _bufferLimit - 1)
  {
    int firstAtPos = _dataBuffer.indexOf('@');
    if (firstAtPos == -1)
      return; // No '@' found

    int secondAtPos = _dataBuffer.indexOf('@', firstAtPos + 1);

    if (secondAtPos == -1)
    {
      // Only one segment after '@', remove everything from first '@'
      _dataBuffer.remove(firstAtPos);
    }
    else
    {
      // Remove the content between the first and second '@'
      _dataBuffer.remove(firstAtPos, secondAtPos - firstAtPos);
    }
    _dataCount--;
  }
  _dataBuffer += "@" + nodeID + "|" + value;
  _dataCount++;

  if (_enablePersistent)
  {
    // --- WRITE ---
    write_File("/tsb_databuffer.txt", _dataBuffer);
    write_File("/tsb_datacount.txt", String(_dataCount));
  }
}

int ThingsSentralBatch::count() const { return _dataCount; }

void ThingsSentralBatch::resetBuffer()
{
  _dataBuffer = "";
  _dataCount = 0;

  if (_enablePersistent)
  {
    // --- WRITE ---
    write_File("/tsb_databuffer.txt", _dataBuffer);
    write_File("/tsb_datacount.txt", String(_dataCount));
  }
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
    _serverURL = API_LINK_SENT_DEFAULT;
  }
}

void ThingsSentralBatch::set_default_serverURL()
{
  _serverURL = API_LINK_SENT_DEFAULT;
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

void ThingsSentralBatch::set_bufferLimit(int value)
{
  if (value > 0 && value <= BUFFER_LIMIT_MAX)
  {

    Serial.println("value > 0 && value <= BUFFER_LIMIT_MAX");
    _bufferLimit = value;
  }
  else
  {
    Serial.println("else BUFFER_LIMIT_DEFAULT");
    _bufferLimit = BUFFER_LIMIT_DEFAULT;
  }
}

void ThingsSentralBatch::set_timeOut(int value)
{
  if (value > 0 /* && value <= BUFFER_LIMIT_MAX */)
  {
    _timeOut = value;
  }
  else
  {
    _timeOut = TIMEOUT_DEFAULT;
  }
}

const String &ThingsSentralBatch::get_serverURL() const { return _serverURL; }
const String &ThingsSentralBatch::get_userID() const { return _userID; }
String ThingsSentralBatch::getLastError() const { return _lastError; }

String ThingsSentralBatch::errorCodeToString(ErrorCode code)
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

  // Set timeout (2 seconds)
  http.setTimeout(_timeOut);

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

int ThingsSentralBatch::send2()
{
  // Reset error message
  _lastError = "";

  // Check if there's data to send
  if (_dataCount == 0)
  {
    _lastError = "No data to send";
    return 1000; // error code no data in buffer
  }

  // Check WiFi connection
  if (!checkWiFiConnection())
  {
    return 2000; // error code wifi disconnected
  }

  HTTPClient http;
  http.setTimeout(_timeOut);

  Serial.println("thingssentral.h: URL:" + _serverURL + _dataBuffer);

#ifdef ESP32
  if (http.begin(_serverURL + _dataBuffer))
#else
  WiFiClient client;
  if (http.begin(client, _serverURL + _dataBuffer))
#endif
  {
    int _httpCode = http.GET();
    // result.httpCode = _httpCode;

    if (_httpCode > 0)
    {
      Serial.printf("[HTTP] GET... code: %d\n", _httpCode);

      if (_httpCode == HTTP_CODE_OK || _httpCode == HTTP_CODE_MOVED_PERMANENTLY || _httpCode == 404)
      {
        String payload = http.getString();
        http.end();

        if (payload != "")
        {
          Serial.println("[read2] Server response: " + payload);
          resetBuffer();
          return 0; // return 0 success!
        }
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(_httpCode).c_str());
      return _httpCode;
    }
  }
  else
  {
    Serial.println("[HTTP] Unable to connect");
    return 3000; // error code: http cant even begin
  }

  return 4000; // error code: unknown error
}

ThingsSentralBatch::ErrorCode ThingsSentralBatch::readNode(const String &nodeID)
{
  /*   // Reset error message
    _lastError = "";
   */
  // Check if there's data to send
  if (nodeID == "")
  {
    // _lastError = "No data to send";
    return ERROR_BLANK_NODE_ID;
  }

  // Check WiFi connection
  if (!checkWiFiConnection())
  {
    return ERROR_WIFI_DISCONNECTED;
  }

  String url = String(API_LINK_READ_DEFAULT) + String("tokenid|") + _userID + String("@NodeId|") + nodeID; // Use the stored server URL
  Serial.println("thingssentral.h READ: " + url);

  HTTPClient http;
#ifdef ESP32
  http.begin(url);
#else
  WiFiClient client;
  http.begin(client, url);
#endif

  // Set timeout (2 seconds)
  http.setTimeout(_timeOut);

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
  // resetBuffer();
  return SUCCESS;
}

// Legacy codes
String TSuserID = "00123";
String APIlinkRead = "http://thingssentral.io/ReadNode?Params=tokenid|" + TSuserID + "@NodeId|";
String APIlinkSent = "http://thingssentral.io/postlong?data=userid|" + TSuserID;

String IRAM_ATTR GET(String completedLink)
{
  HTTPClient http;
  http.setTimeout(2500);

#ifdef ESP32
  http.begin(completedLink);
#else
  WiFiClient client;
  http.begin(client, completedLink);
#endif

  http.GET();
  String reply = http.getString();
  http.end();
  return reply;
}

String IRAM_ATTR readNode(String __NodeID, bool _fullReply)
{
  Serial.println(APIlinkRead + __NodeID);
  String _reply = GET(APIlinkRead + __NodeID);
  if (_reply == "")
  {
    Serial.println("thingssentral.h: error, blank reply from server. Check connection. : " + __NodeID);
    return "error";
  }

  String _valueOnly = _reply.substring(_reply.indexOf("|") + 1,
                                       _reply.indexOf("|", _reply.indexOf("|") + 1));
  Serial.println("thingssentral.h: readNode: " + _valueOnly);

  if (_fullReply)
  {
    return _reply;
  }
  else
  {
    return _valueOnly;
  }
}

String IRAM_ATTR sendNode(String _NodeID1, String _Data1,
                          String _NodeID2, String _Data2, // OPTIONAL PARAM
                          String _NodeID3, String _Data3, // OPTIONAL PARAM
                          String _NodeID4, String _Data4) // OPTIONAL PARAM
{
  String completedLink = APIlinkSent;
  completedLink.concat(String("@") + _NodeID1 + "|" + String(_Data1));
  if (_NodeID2 != "")
    completedLink.concat(String("@") + _NodeID2 + "|" + String(_Data2));
  if (_NodeID3 != "")
    completedLink.concat(String("@") + _NodeID3 + "|" + String(_Data3));
  if (_NodeID4 != "")
    completedLink.concat(String("@") + _NodeID4 + "|" + String(_Data4));

  String _reply = GET(completedLink);
  if (_reply == "")
  {
    Serial.println("thingssentral.h: error, blank reply from server. Check connection.");
    return "error";
  }

  Serial.println("thingssentral.h: sendNode: " + _reply);
  return _reply;
}

// new readNode
ReadResult IRAM_ATTR readNode2(String __NodeID)
{
  ReadResult result = {0, "", ""};
  WiFiClient client;
  HTTPClient http;
  http.setTimeout(2500);

  Serial.println("thingssentral.h: URL:" + APIlinkRead + __NodeID);

  if (http.begin(client, APIlinkRead + __NodeID))
  {
    int _httpCode = http.GET();
    result.httpCode = _httpCode;

    if (_httpCode > 0)
    {
      Serial.printf("[HTTP] GET... code: %d\n", _httpCode);

      if (_httpCode == HTTP_CODE_OK || _httpCode == HTTP_CODE_MOVED_PERMANENTLY || _httpCode == 404)
      {
        String payload = http.getString();

        if (payload != "")
        {
          result.fullResponse = payload;

          // Extract value between first and second pipe
          int firstPipe = payload.indexOf("|");
          int secondPipe = payload.indexOf("|", firstPipe + 1);

          if (firstPipe != -1 && secondPipe != -1)
          {
            result.value = payload.substring(firstPipe + 1, secondPipe);
          }

          /* Serial.println("thingssentral.h: readNode full: " + result.fullResponse);
          Serial.println("thingssentral.h: readNode value: " + result.value); */
        }
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(_httpCode).c_str());
    }

    http.end();
  }
  else
  {
    Serial.println("[HTTP] Unable to connect");
  }

  return result;
}