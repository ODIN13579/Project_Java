#include <ArduinoJson.h>
#include <Preferences.h>
#include <time.h>
#include <esp_system.h>   // esp_random()
#include <WiFi.h>         // ESP32 kết nối WiFi để giao tiếp với MQTT Broker
#include <PubSubClient.h> // Thư viện MQTT cho ESP32
#include "DHT.h"          // Thư viện cảm biến DHT22

// SENSOR & DEVICE PIN
#define DHTPIN 15     // Chân kết nối DHT22
#define DHTTYPE DHT22 // Loại cảm biến DHT
#define LDR_PIN 34    // Chân đọc cảm biến ánh sáng
#define CO2_PIN 27    // Chân đọc cảm biến CO2
#define FAN_PIN 4     // Chân điều khiển quạt

DHT dht(DHTPIN, DHTTYPE);
bool getTimestamp(char *output, size_t outputSize);
bool isClockTrusted();
void publishState();
void applyControlLogic();
void setFanState(bool turnOn); // Tạo đối tượng DHT22

// FAN THRESHOLD
#define FAN_ON_TEMP 31.0  // Nhiệt độ >= 31°C thì bật quạt
#define FAN_OFF_TEMP 30.0 // Nhiệt độ <= 30°C thì tắt quạt

bool fanState = false; // false = quạt tắt, true = quạt bật
bool safetyOverride = true;

bool latestDhtValid = false;
bool haveValidTemperature = false;

float latestTemperature = NAN;
float latestHumidity = NAN;
int latestCo2 = 0;
int latestLight = 0;
bool latestCo2Valid = false;
bool latestLightValid = false;

unsigned long lastValidTemperatureMs = 0;
unsigned long lastSensorSampleMs = 0;
unsigned long lastState = 0;

const unsigned long SENSOR_INTERVAL = 2000;
const unsigned long SENSOR_STALE_MS = 15000;
const unsigned long STATE_INTERVAL = 5000;

String bootId;
enum ControlMode
{
  MODE_AUTO,
  MODE_MANUAL
};

ControlMode controlMode = MODE_AUTO;
// Cache command_id đã xử lý, chỉ lưu trong RAM.
const int COMMAND_CACHE_SIZE = 64;

struct CachedCommand
{
  bool used = false;
  String commandId;
  String normalizedContent;
  String originalStatus;
  String originalReason;
};

CachedCommand commandCache[COMMAND_CACHE_SIZE];
int commandCacheNext = 0;

int findCachedCommand(const String &commandId)
{
  for (int i = 0; i < COMMAND_CACHE_SIZE; i++)
  {
    if (commandCache[i].used &&
        commandCache[i].commandId == commandId)
    {
      return i;
    }
  }

  return -1;
}

bool isValidUuid(const char *s)
{
  if (s == nullptr || strlen(s) != 36)
  {
    return false;
  }

  for (int i = 0; i < 36; i++)
  {
    if (i == 8 || i == 13 || i == 18 || i == 23)
    {
      if (s[i] != '-')
      {
        return false;
      }
    }
    else
    {
      char c = s[i];

      bool isDigit = c >= '0' && c <= '9';
      bool isLowerHex = c >= 'a' && c <= 'f';
      bool isUpperHex = c >= 'A' && c <= 'F';

      if (!isDigit && !isLowerHex && !isUpperHex)
      {
        return false;
      }
    }
  }

  return true;
}

void rememberCommand(
    const String &commandId,
    const String &normalizedContent,
    const char *status,
    const char *reason)
{
  if (commandId.isEmpty())
  {
    return;
  }

  CachedCommand &entry = commandCache[commandCacheNext];

  entry.used = true;
  entry.commandId = commandId;
  entry.normalizedContent = normalizedContent;
  entry.originalStatus = status;
  entry.originalReason = reason;

  commandCacheNext =
      (commandCacheNext + 1) % COMMAND_CACHE_SIZE;
}
// WIFI CONFIGURATION
const char *WIFI_SSID = "Wokwi-GUEST";
const char *WIFI_PASSWORD = "";

// MQTT CONFIGURATION
const char *MQTT_HOST = "host.wokwi.internal"; // Địa chỉ MQTT Broker
const int MQTT_PORT = 1883;                    // Cổng MQTT Broker

// Phải lấy đúng giá trị từ Backend hoặc database của nhóm.
const char *HOUSE_ID = "1";
const char *ZONE_ID = "1";
const char *DEVICE_ID = "ESP32-HOUSE01-ZONE01";

// Tạm dùng ID ví dụ ở trên.
// Khi tích hợp, thay bằng ID thật do nhóm Backend cung cấp.

const char *HEARTBEAT_TOPIC =
    "swiftletcare/1/1/ESP32-HOUSE01-ZONE01/heartbeat";
const char *TELEMETRY_TOPIC =
    "swiftletcare/1/1/ESP32-HOUSE01-ZONE01/telemetry";
const char *COMMAND_TOPIC =
    "swiftletcare/1/1/ESP32-HOUSE01-ZONE01/command";

const char *COMMAND_ACK_TOPIC =
    "swiftletcare/1/1/ESP32-HOUSE01-ZONE01/command_ack";
const char *STATE_TOPIC =
    "swiftletcare/1/1/ESP32-HOUSE01-ZONE01/state";
const char *CFG_TOPIC =
    "swiftletcare/1/1/ESP32-HOUSE01-ZONE01/cfg";
const char *CFG_ACK_TOPIC =
    "swiftletcare/1/1/ESP32-HOUSE01-ZONE01/cfg_ack";
const char *EVENTS_TOPIC =
    "swiftletcare/1/1/ESP32-HOUSE01-ZONE01/events";

Preferences preferences;
struct ZoneConfig
{
  uint32_t version = 1;
  float humidityOn = 70.0f;
  float humidityOff = 75.0f;
  float tempMin = 20.0f;
  float tempMax = 31.0f;
  uint32_t telemetryIntervalMs = 5000;
};
ZoneConfig zoneConfig;
unsigned long manualModeStartedMs = 0;
const unsigned long MANUAL_TIMEOUT_MS = 10UL * 60UL * 1000UL;
uint8_t dhtGoodStreak = 0;
bool dhtFaultActive = false;
bool lastDhtValidState = true;
unsigned long lastFaultPublishMs = 0;
const unsigned long SENSOR_FAULT_RETRY_MS = 10000UL;
uint32_t droppedTelemetryCount = 0;
unsigned long bootStartedMs = 0;
const char *TELEMETRY_ACK_TOPIC =
    "swiftletcare/1/1/ESP32-HOUSE01-ZONE01/telemetry_ack";
// MQTT CLIENT
WiFiClient espClient;               // Tạo kết nối TCP/IP
PubSubClient mqttClient(espClient); // Tạo MQTT Client

// TIMERS
unsigned long lastTelemetry = 0;
unsigned long lastHeartbeat = 0;

unsigned long TELEMETRY_INTERVAL = 5000;       // Configurable 1..60s
const unsigned long HEARTBEAT_INTERVAL = 2000; // Gửi heartbeat mỗi 2 giây

unsigned long messageCounter = 0; // Đếm số message đã tạo
bool ntpConfigured = false;
// WIFI CONNECT
void connectWiFi()
{
  Serial.println("Starting WiFi connection...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
}
unsigned long lastWiFiAttempt = 0;
const unsigned long WIFI_RETRY_INTERVAL = 5000;

void maintainWiFi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    return;
  }

  if (millis() - lastWiFiAttempt >= WIFI_RETRY_INTERVAL)
  {
    lastWiFiAttempt = millis();

    Serial.println("WiFi disconnected; retrying...");
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
  }
}

// OFFLINE TELEMETRY BUFFER
#define BUFFER_SIZE 120 // 10 phút ở chu kỳ 5s; RAM-only, mất khi reset
#define TELEMETRY_RETRY_INTERVAL 5000UL

// Cấu trúc lưu message khi MQTT bị mất kết nối
struct BufferedMessage
{
  String topic;
  String payload;
  unsigned long lastSentMs = 0;
};

BufferedMessage messageBuffer[BUFFER_SIZE];
int bufferCount = 0; // Số message hiện đang được lưu

// Thêm message vào buffer khi MQTT không gửi được
bool addToBuffer(const char *topic, const char *payload)
{
  if (bufferCount >= BUFFER_SIZE)
  {
    droppedTelemetryCount++;
    Serial.printf(
        "[BUFFER] FULL; newest telemetry dropped | total=%lu\n",
        (unsigned long)droppedTelemetryCount);
    return false;
  }

  messageBuffer[bufferCount].topic = topic;
  messageBuffer[bufferCount].payload = payload;
  messageBuffer[bufferCount].lastSentMs = 0;

  bufferCount++;

  Serial.printf("Telemetry buffered. Count = %d\n", bufferCount);
  return true;
}

void removeBufferedTelemetryByEventId(const String &eventId)
{
  for (int i = 0; i < bufferCount; i++)
  {
    JsonDocument doc;

    DeserializationError error =
        deserializeJson(doc, messageBuffer[i].payload);

    if (error || !doc["event_id"].is<const char *>())
    {
      continue;
    }

    if (doc["event_id"].as<String>() != eventId)
    {
      continue;
    }

    // Dịch các bản tin còn lại để lấp vị trí vừa xoá.
    for (int j = i; j < bufferCount - 1; j++)
    {
      messageBuffer[j] = messageBuffer[j + 1];
    }

    bufferCount--;

    // Giải phóng dữ liệu còn giữ ở ô cuối.
    messageBuffer[bufferCount].topic = String();
    messageBuffer[bufferCount].payload = String();
    messageBuffer[bufferCount].lastSentMs = 0;

    Serial.printf(
        "[BUFFER] Removed STORED: %s | remaining=%d\n",
        eventId.c_str(),
        bufferCount);
    return;
  }

  // ACK lặp hoặc ACK cho bản tin không còn trong buffer.
  // Không xoá bản tin khác.
  Serial.printf(
      "[BUFFER] ACK ignored; event_id not found: %s\n",
      eventId.c_str());
}

void handleTelemetryAck(byte *payload, unsigned int length)
{
  JsonDocument doc;

  DeserializationError error =
      deserializeJson(doc, payload, length);

  if (error || !doc.is<JsonObject>())
  {
    Serial.println("[TELEMETRY_ACK] Invalid JSON");
    return;
  }

  if (!doc["event_id"].is<const char *>() ||
      !doc["status"].is<const char *>() ||
      !doc["device_id"].is<const char *>())
  {
    Serial.println("[TELEMETRY_ACK] Missing or invalid fields");
    return;
  }

  String ackDeviceId = doc["device_id"].as<String>();
  if (ackDeviceId != DEVICE_ID)
  {
    Serial.println("[TELEMETRY_ACK] Ignored ACK for another device");
    return;
  }

  String eventId = doc["event_id"].as<String>();
  String status = doc["status"].as<String>();

  if (eventId.isEmpty() || status != "STORED")
  {
    Serial.println("[TELEMETRY_ACK] ACK not accepted");
    return;
  }

  removeBufferedTelemetryByEventId(eventId);
}

void resendBufferedMessages()
{
  if (!mqttClient.connected())
  {
    return;
  }

  unsigned long now = millis();

  for (int i = 0; i < bufferCount; i++)
  {
    // Chưa đến thời điểm gửi lại.
    if (messageBuffer[i].lastSentMs != 0 &&
        now - messageBuffer[i].lastSentMs < TELEMETRY_RETRY_INTERVAL)
    {
      continue;
    }

    bool success = mqttClient.publish(
        messageBuffer[i].topic.c_str(),
        messageBuffer[i].payload.c_str());

    // Ghi nhận thời điểm thử gửi để tránh retry liên tục.
    messageBuffer[i].lastSentMs = now;
    if (success)
    {
      Serial.println("Telemetry published; waiting for STORED ACK");
    }
    else
    {
      Serial.println("Telemetry publish failed; keep in buffer");
    }
  }
}
// MQTT CONNECT
unsigned long lastMQTTAttempt = 0;
const unsigned long MQTT_RETRY_INTERVAL = 3000; // Thử reconnect mỗi 3 giây

// Kiểm tra và kết nối lại MQTT khi bị mất kết nối
// Kiểm tra và kết nối lại MQTT khi bị mất kết nối
void tryConnectMQTT()
{
  if (mqttClient.connected())
  {
    return;
  }

  // Chỉ thử kết nối lại sau mỗi 3 giây.
  if (millis() - lastMQTTAttempt < MQTT_RETRY_INTERVAL)
  {
    return;
  }

  lastMQTTAttempt = millis();

  Serial.print("MQTT reconnect attempt... ");

  String clientId = String(DEVICE_ID);

  if (mqttClient.connect(clientId.c_str()))
  {
    Serial.println("SUCCESS");

    mqttClient.subscribe(CFG_TOPIC, 1);
    if (mqttClient.subscribe(COMMAND_TOPIC, 1))
    {
      Serial.println("Subscribed to command topic");
    }
    else
    {
      Serial.println("Command subscription failed");
    }
    if (mqttClient.subscribe(TELEMETRY_ACK_TOPIC, 1))
    {
      Serial.println("Subscribed to telemetry ACK topic");
    }
    else
    {
      Serial.println("Telemetry ACK subscription failed");
    }
    // Gửi trạng thái hiện tại sau khi kết nối MQTT thành công.
    publishState();
  }
  else
  {
    Serial.print("FAILED, rc=");
    Serial.println(mqttClient.state());
  }
}
// TELEMETRY
// Tạo và gửi dữ liệu nhiệt độ, độ ẩm, CO2 và ánh sáng
// TELEMETRY
// Đồng bộ schema telemetry theo code nhóm trưởng.

void publishTelemetry(
    float temperature,
    float humidity,
    int co2,
    int light,
    bool dhtValid,
    bool co2Valid,
    bool lightValid)
{
  char timestamp[32];

  // Chi gui telemetry khi dong ho da duoc dong bo.
  if (!getTimestamp(timestamp, sizeof(timestamp)))
  {
    Serial.println("Telemetry skipped: clock not trusted");
    return;
  }

  messageCounter++;

  // Tao event_id rieng cho tung ban tin telemetry.
  char eventId[100];

  snprintf(
      eventId,
      sizeof(eventId),
      "%s-%08lx%08lx-%lu",
      DEVICE_ID,
      (unsigned long)esp_random(),
      (unsigned long)esp_random(),
      messageCounter);

  // Xac dinh trang thai cam bien.
  int validCount = (dhtValid ? 1 : 0) + (co2Valid ? 1 : 0) + (lightValid ? 1 : 0);

  const char *sensorStatus;

  if (validCount == 3)
  {
    sensorStatus = "OK";
  }
  else if (validCount == 0)
  {
    sensorStatus = "ERROR";
  }
  else
  {
    sensorStatus = "PARTIAL_ERROR";
  }

  char payload[512];

  // Dung JSON document de cac gia tri loi co the la null.
  JsonDocument doc;

  doc["event_id"] = eventId;
  doc["device_id"] = DEVICE_ID;
  doc["timestamp"] = timestamp;
  doc["sensor_status"] = sensorStatus;

  // DHT22: nhiet do va do am cung hop le hoac cung loi.
  if (dhtValid)
  {
    doc["temperature"] = temperature;
    doc["humidity"] = humidity;
  }
  else
  {
    doc["temperature"] = nullptr;
    doc["humidity"] = nullptr;
  }

  if (co2Valid)
  {
    doc["co2"] = co2;
  }
  else
  {
    doc["co2"] = nullptr;
  }

  if (lightValid)
  {
    doc["light"] = light;
  }
  else
  {
    doc["light"] = nullptr;
  }

  size_t payloadLength = serializeJson(doc, payload, sizeof(payload));

  if (payloadLength == 0 || payloadLength >= sizeof(payload))
  {
    Serial.println("Telemetry JSON serialization failed");
    return;
  }

  Serial.print("TELEMETRY: ");
  Serial.println(payload);

  // Luôn lưu telemetry trước để chờ Backend xác nhận STORED.
  if (!addToBuffer(TELEMETRY_TOPIC, payload))
  {
    Serial.println("Could not buffer telemetry");
    return;
  }

  // Nếu MQTT đang kết nối, thử gửi ngay.
  // Message vẫn nằm trong buffer cho đến khi nhận ACK STORED.
  resendBufferedMessages();
}
// HEARTBEAT
// Gửi trạng thái hoạt động của ESP32 định kỳ
// HEARTBEAT
// Đồng bộ schema heartbeat theo code nhóm trưởng.
void publishHeartbeat()
{
  if (!mqttClient.connected())
  {
    return;
  }

  char timestamp[32];

  if (!getTimestamp(timestamp, sizeof(timestamp)))
  {
    Serial.println("Heartbeat skipped: clock not trusted");
    return;
  }

  char payload[256];

  snprintf(
      payload,
      sizeof(payload),
      "{"
      "\"device_id\":\"%s\","
      "\"timestamp\":\"%s\""
      "}",
      DEVICE_ID,
      timestamp);

  bool sent = mqttClient.publish(
      HEARTBEAT_TOPIC,
      payload);

  if (sent)
  {
    Serial.print("HEARTBEAT published: ");
    Serial.println(payload);
  }
  else
  {
    Serial.println("HEARTBEAT publish failed");
  }
}

void publishCommandAck(
    const String &commandId,
    const String &status,
    const String &reason,
    bool isDuplicate = false,
    const String &originalStatus = "",
    const String &originalReason = "")
{
  JsonDocument doc;

  doc["device_id"] = DEVICE_ID;
  doc["command_id"] = commandId;
  doc["actuator"] = "FAN";
  doc["status"] = status;
  doc["reason"] = reason;
  doc["mode"] = controlMode == MODE_AUTO ? "AUTO" : "MANUAL";
  doc["state"] = fanState ? "ON" : "OFF";
  doc["gpio_state"] = digitalRead(FAN_PIN) == HIGH ? "HIGH" : "LOW";
  doc["safetyOverride"] = safetyOverride;

  char timestamp[25];

  if (getTimestamp(timestamp, sizeof(timestamp)))
  {
    doc["timestamp"] = timestamp;
  }
  else
  {
    doc["timestamp"] = nullptr;
  }

  if (isDuplicate)
  {
    doc["originalStatus"] = originalStatus;
    doc["originalReason"] = originalReason;
  }

  char payload[512];

  size_t length = serializeJson(doc, payload, sizeof(payload));

  if (length == 0 || length >= sizeof(payload))
  {
    Serial.println("[COMMAND_ACK] Serialization failed");
    return;
  }

  Serial.println("========== COMMAND ACK ==========");
  Serial.print("[COMMAND_ACK] command_id: ");
  Serial.println(commandId);

  Serial.print("[COMMAND_ACK] status: ");
  Serial.println(status);

  Serial.print("[COMMAND_ACK] reason: ");
  Serial.println(reason);

  Serial.print("[COMMAND_ACK] payload: ");
  Serial.println(payload);

  if (!mqttClient.connected())
  {
    Serial.println("[COMMAND_ACK] MQTT disconnected; ACK not published");
    return;
  }

  bool published = mqttClient.publish(
      COMMAND_ACK_TOPIC,
      (const uint8_t *)payload,
      length,
      false);

  Serial.println(
      published
          ? "[COMMAND_ACK] Publish successful"
          : "[COMMAND_ACK] Publish failed");
}

void publishState()
{
  if (!mqttClient.connected())
  {
    return;
  }

  char timestamp[32];
  bool clockTrusted = getTimestamp(timestamp, sizeof(timestamp));

  JsonDocument doc;

  doc["device_id"] = DEVICE_ID;
  doc["mode"] = controlMode == MODE_AUTO ? "AUTO" : "MANUAL";
  doc["state"] = fanState ? "ON" : "OFF";
  doc["gpio_state"] = digitalRead(FAN_PIN) == HIGH ? "HIGH" : "LOW";
  doc["safetyOverride"] = safetyOverride;

  if (clockTrusted)
  {
    doc["timestamp"] = timestamp;
  }
  else
  {
    doc["timestamp"] = nullptr;
  }

  doc["boot_id"] = bootId;

  char payload[384];
  size_t length = serializeJson(doc, payload, sizeof(payload));

  if (length == 0 || length >= sizeof(payload))
  {
    Serial.println("STATE serialization failed");
    return;
  }

  mqttClient.publish(STATE_TOPIC, payload, false);
}
void setFanState(bool turnOn)
{
  bool changed = fanState != turnOn;

  fanState = turnOn;

  digitalWrite(
      FAN_PIN,
      turnOn ? HIGH : LOW);

  Serial.printf(
      "FAN=%s GPIO=%s\n",
      fanState ? "ON" : "OFF",
      digitalRead(FAN_PIN) == HIGH ? "HIGH" : "LOW");

  if (changed)
  {
    publishState();
  }
}
void applyControlLogic()
{
  bool fresh =
      latestDhtValid &&
      haveValidTemperature &&
      (millis() - lastValidTemperatureMs <= SENSOR_STALE_MS);
  if (!fresh)
  {
    bool overrideChanged = !safetyOverride;

    safetyOverride = true;
    setFanState(true);

    if (overrideChanged)
    {
      publishState();
    }

    return;
  }

  bool overrideChanged = safetyOverride;
  safetyOverride = false;

  if (controlMode == MODE_AUTO)
  {
    if (!fanState && latestTemperature >= FAN_ON_TEMP)
    {
      setFanState(true);
    }
    else if (fanState && latestTemperature <= FAN_OFF_TEMP)
    {
      setFanState(false);
    }
  }

  if (overrideChanged)
  {
    publishState();
  }
}
time_t utcToEpoch(struct tm *t)
{
  // Convert calendar date to days since 1970-01-01.
  int year = t->tm_year + 1900;
  unsigned int month = t->tm_mon + 1;
  int day = t->tm_mday;

  year -= (month <= 2);

  const int era = (year >= 0 ? year : year - 399) / 400;
  const unsigned int yearOfEra = (unsigned int)(year - era * 400);
  const unsigned int dayOfYear =
      (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
  const unsigned int dayOfEra =
      yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;

  int days = era * 146097 + (int)dayOfEra - 719468;

  return (time_t)days * 86400 + (time_t)t->tm_hour * 3600 + (time_t)t->tm_min * 60 + t->tm_sec;
}
bool isValidUtcDate(
    int year,
    int month,
    int day,
    int hour,
    int minute,
    int second)
{
  if (year < 1970 ||
      month < 1 || month > 12 ||
      hour < 0 || hour > 23 ||
      minute < 0 || minute > 59 ||
      second < 0 || second > 59)
  {
    return false;
  }

  const int daysInMonth[] = {
      31, 28, 31, 30, 31, 30,
      31, 31, 30, 31, 30, 31};

  int maxDay = daysInMonth[month - 1];

  bool leapYear =
      (year % 400 == 0) ||
      (year % 4 == 0 && year % 100 != 0);

  if (month == 2 && leapYear)
  {
    maxDay = 29;
  }

  return day >= 1 && day <= maxDay;
}
bool hasStrictUtcFormat(const char *s)
{
  if (s == nullptr || strlen(s) != 20)
  {
    return false;
  }

  const int positions[] = {4, 7, 10, 13, 16, 19};
  const char expected[] = {'-', '-', 'T', ':', ':', 'Z'};

  for (int i = 0; i < 6; i++)
  {
    if (s[positions[i]] != expected[i])
    {
      return false;
    }
  }

  for (int i = 0; i < 19; i++)
  {
    bool separator = false;

    for (int j = 0; j < 6; j++)
    {
      if (i == positions[j])
      {
        separator = true;
        break;
      }
    }

    if (!separator && (s[i] < '0' || s[i] > '9'))
    {
      return false;
    }
  }

  return true;
}
bool parseUtcExpiry(const char *value, time_t &expiry)
{
  if (!hasStrictUtcFormat(value))
  {
    return false;
  }

  int year, month, day;
  int hour, minute, second;
  char trailing;

  // Chỉ chấp nhận định dạng UTC: YYYY-MM-DDTHH:MM:SSZ
  int fields = sscanf(
      value,
      "%4d-%2d-%2dT%2d:%2d:%2dZ%c",
      &year, &month, &day,
      &hour, &minute, &second,
      &trailing);

  if (fields != 6)
  {
    return false;
  }

  if (!isValidUtcDate(year, month, day, hour, minute, second))
  {
    return false;
  }
  struct tm timeInfo = {};
  timeInfo.tm_year = year - 1900;
  timeInfo.tm_mon = month - 1;
  timeInfo.tm_mday = day;
  timeInfo.tm_hour = hour;
  timeInfo.tm_min = minute;
  timeInfo.tm_sec = second;

  // Chuyển thời gian UTC thành Unix timestamp.
  expiry = utcToEpoch(&timeInfo);

  return expiry > 0;
}
void rejectAndCache(
    const String &commandId,
    const String &normalizedContent,
    const char *reason)
{
  publishCommandAck(commandId, "REJECTED", reason);
  rememberCommand(
      commandId,
      normalizedContent,
      "REJECTED",
      reason);
}

void publishCfgAck(uint32_t version, const char *status, const char *reason)
{
  if (!mqttClient.connected())
    return;
  char timestamp[32] = "";
  getTimestamp(timestamp, sizeof(timestamp));
  JsonDocument d;
  d["device_id"] = DEVICE_ID;
  d["version"] = version;
  d["status"] = status;
  d["reason"] = reason;
  d["timestamp"] = timestamp;
  char out[320];
  size_t n = serializeJson(d, out, sizeof(out));
  if (n && n < sizeof(out))
    mqttClient.publish(CFG_ACK_TOPIC, out, false);
  Serial.printf("[CFG_ACK] version=%lu status=%s reason=%s\n", (unsigned long)version, status, reason);
}
void saveZoneConfig()
{
  preferences.begin("swiftlet", false);
  preferences.putUInt("ver", zoneConfig.version);
  preferences.putFloat("hon", zoneConfig.humidityOn);
  preferences.putFloat("hoff", zoneConfig.humidityOff);
  preferences.putFloat("tmin", zoneConfig.tempMin);
  preferences.putFloat("tmax", zoneConfig.tempMax);
  preferences.putUInt("tele", zoneConfig.telemetryIntervalMs);
  preferences.end();
}
void loadZoneConfig()
{
  preferences.begin("swiftlet", true);
  zoneConfig.version = preferences.getUInt("ver", 1);
  zoneConfig.humidityOn = preferences.getFloat("hon", 70);
  zoneConfig.humidityOff = preferences.getFloat("hoff", 75);
  zoneConfig.tempMin = preferences.getFloat("tmin", 20);
  zoneConfig.tempMax = preferences.getFloat("tmax", 31);
  zoneConfig.telemetryIntervalMs = preferences.getUInt("tele", 5000);
  preferences.end();
  if (zoneConfig.humidityOn < 0 || zoneConfig.humidityOff > 100 || zoneConfig.humidityOn >= zoneConfig.humidityOff || zoneConfig.tempMin < -10 || zoneConfig.tempMax > 60 || zoneConfig.tempMin >= zoneConfig.tempMax || zoneConfig.telemetryIntervalMs < 1000 || zoneConfig.telemetryIntervalMs > 60000)
    zoneConfig = ZoneConfig();
  TELEMETRY_INTERVAL = zoneConfig.telemetryIntervalMs;
}
void handleConfigMessage(byte *payload, unsigned int length)
{
  JsonDocument d;
  if (deserializeJson(d, payload, length) != DeserializationError::Ok || !d.is<JsonObject>())
  {
    publishCfgAck(0, "REJECTED", "INVALID_JSON");
    return;
  }
  if (!d["version"].is<uint32_t>())
  {
    publishCfgAck(0, "REJECTED", "INVALID_VERSION");
    return;
  }
  uint32_t v = d["version"].as<uint32_t>();
  if (v < zoneConfig.version)
  {
    publishCfgAck(v, "STALE", "OLDER_VERSION");
    return;
  }
  if (v == zoneConfig.version)
  {
    publishCfgAck(v, "ALREADY_CURRENT", "SAME_VERSION");
    return;
  }
  if (!d["humidity_on"].is<float>() && !d["humidity_on"].is<int>())
  {
    publishCfgAck(v, "REJECTED", "INVALID_HUMIDITY_ON");
    return;
  }
  if (!d["humidity_off"].is<float>() && !d["humidity_off"].is<int>())
  {
    publishCfgAck(v, "REJECTED", "INVALID_HUMIDITY_OFF");
    return;
  }
  if (!d["temp_min"].is<float>() && !d["temp_min"].is<int>())
  {
    publishCfgAck(v, "REJECTED", "INVALID_TEMP_MIN");
    return;
  }
  if (!d["temp_max"].is<float>() && !d["temp_max"].is<int>())
  {
    publishCfgAck(v, "REJECTED", "INVALID_TEMP_MAX");
    return;
  }
  if (!d["telemetry_interval_ms"].is<uint32_t>())
  {
    publishCfgAck(v, "REJECTED", "INVALID_TELEMETRY_INTERVAL");
    return;
  }
  ZoneConfig c = zoneConfig;
  c.version = v;
  c.humidityOn = d["humidity_on"].as<float>();
  c.humidityOff = d["humidity_off"].as<float>();
  c.tempMin = d["temp_min"].as<float>();
  c.tempMax = d["temp_max"].as<float>();
  c.telemetryIntervalMs = d["telemetry_interval_ms"].as<uint32_t>();
  if (c.humidityOn < 0 || c.humidityOn > 100 || c.humidityOff < 0 || c.humidityOff > 100 || c.humidityOn >= c.humidityOff || c.tempMin < -10 || c.tempMax > 60 || c.tempMin >= c.tempMax || c.telemetryIntervalMs < 1000 || c.telemetryIntervalMs > 60000)
  {
    publishCfgAck(v, "REJECTED", "RANGE_OR_THRESHOLD_INVALID");
    return;
  }
  zoneConfig = c;
  TELEMETRY_INTERVAL = c.telemetryIntervalMs;
  saveZoneConfig();
  publishCfgAck(v, "APPLIED", "OK");
}
void publishSensorEvent(const char *type, const char *sensor, const char *reason)
{
  if (!mqttClient.connected())
    return;
  char timestamp[32] = "";
  getTimestamp(timestamp, sizeof(timestamp));
  JsonDocument d;
  d["device_id"] = DEVICE_ID;
  d["event_type"] = type;
  d["sensor"] = sensor;
  d["reason"] = reason;
  d["timestamp"] = timestamp;
  char out[320];
  size_t n = serializeJson(d, out, sizeof(out));
  if (n && n < sizeof(out))
    mqttClient.publish(EVENTS_TOPIC, out, false);
  Serial.printf("[EVENT] %s sensor=%s reason=%s\n", type, sensor, reason);
}

void mqttCallback(
    char *topic,
    byte *payload,
    unsigned int length)
{
  if (length == 0 || length > 512)
  {
    Serial.println("Rejected: invalid payload length");
    return;
  }

  // Xử lý ACK telemetry trước.
  if (strcmp(topic, TELEMETRY_ACK_TOPIC) == 0)
  {
    handleTelemetryAck(payload, length);
    return;
  }
  if (strcmp(topic, CFG_TOPIC) == 0)
  {
    handleConfigMessage(payload, length);
    return;
  }

  // Các phần còn lại chỉ xử lý command.
  if (strcmp(topic, COMMAND_TOPIC) != 0)
  {
    Serial.println("Ignored message from unexpected topic");
    return;
  }

  JsonDocument doc;

  DeserializationError error =
      deserializeJson(doc, payload, length);

  if (error || !doc.is<JsonObject>())
  {
    Serial.println("Rejected: invalid JSON");
    return;
  }

  // Kiểm tra command_id trước tiên.
  // Không thể ACK nếu không có command_id dạng chuỗi hợp lệ.
  if (!doc["command_id"].is<const char *>())
  {
    Serial.println(
        "[COMMAND] Invalid command_id type; ACK omitted");
    return;
  }

  const char *rawCommandId = doc["command_id"];

  if (!isValidUuid(rawCommandId))
  {
    Serial.println(
        "[COMMAND] Invalid UUID format; ACK omitted");
    return;
  }

  String commandId = rawCommandId;

  Serial.print("[COMMAND] Received command_id: ");
  Serial.println(commandId);

  // Kiểm tra các trường bắt buộc còn lại.
  bool requiredFieldsValid =
      doc["action"].is<const char *>() &&
      doc["actuator"].is<const char *>() &&
      doc["expiresAt"].is<const char *>();

  if (!requiredFieldsValid)
  {
    Serial.println(
        "[COMMAND] REJECTED: INVALID_PAYLOAD");

    publishCommandAck(
        commandId,
        "REJECTED",
        "INVALID_PAYLOAD");

    return;
  }

  String action = doc["action"].as<String>();
  String actuator = doc["actuator"].as<String>();
  String expiresAt = doc["expiresAt"].as<String>();

  Serial.print("[COMMAND] action: ");
  Serial.println(action);

  Serial.print("[COMMAND] actuator: ");
  Serial.println(actuator);

  Serial.print("[COMMAND] expiresAt: ");
  Serial.println(expiresAt);

  Serial.println("========== COMMAND RECEIVED ==========");
  Serial.print("[COMMAND] command_id: ");
  Serial.println(commandId);

  Serial.print("[COMMAND] action: ");
  Serial.println(action);

  Serial.print("[COMMAND] actuator: ");
  Serial.println(actuator);

  Serial.print("[COMMAND] expiresAt: ");
  Serial.println(expiresAt);
  bool hasMode = !doc["targetMode"].isUnbound();
  bool hasState = !doc["targetState"].isUnbound();
  String targetMode = "";
  String targetState = "";

  bool targetModeTypeValid =
      !hasMode || doc["targetMode"].is<const char *>();

  bool targetStateTypeValid =
      !hasState || doc["targetState"].is<const char *>();

  if (targetModeTypeValid && hasMode)
  {
    targetMode = doc["targetMode"].as<String>();
  }

  if (targetStateTypeValid && hasState)
  {
    targetState = doc["targetState"].as<String>();
  }

  if (!targetModeTypeValid)
  {
    publishCommandAck(
        commandId,
        "REJECTED",
        "INVALID_TARGET_MODE");
    return;
  }

  if (!targetStateTypeValid)
  {
    publishCommandAck(
        commandId,
        "REJECTED",
        "INVALID_TARGET_STATE");
    return;
  }
  // Kiểm tra cache trước khi kiểm tra hết hạn.
  // Lệnh gửi lại cùng ID và cùng nội dung phải nhận kết quả cũ.// Chỉ cho phép đúng các trường trong hợp đồng.
  for (JsonPair kv : doc.as<JsonObject>())
  {
    const char *key = kv.key().c_str();

    bool allowed =
        strcmp(key, "command_id") == 0 ||
        strcmp(key, "action") == 0 ||
        strcmp(key, "actuator") == 0 ||
        strcmp(key, "expiresAt") == 0 ||
        strcmp(key, "targetMode") == 0 ||
        strcmp(key, "targetState") == 0;

    if (!allowed)
    {
      publishCommandAck(
          commandId,
          "REJECTED",
          "INVALID_PAYLOAD");
      return;
    }
  }
  String normalizedContent =
      action + "|" + actuator + "|" +
      targetMode + "|" + targetState + "|" +
      expiresAt;
  int cachedIndex = findCachedCommand(commandId);

  if (cachedIndex >= 0)
  {
    CachedCommand &cached = commandCache[cachedIndex];

    if (cached.normalizedContent == normalizedContent)
    {
      publishCommandAck(
          commandId,
          "DUPLICATE",
          "ALREADY_PROCESSED",
          true,
          cached.originalStatus,
          cached.originalReason);

      Serial.println("Duplicate command: original result retained");
    }
    else
    {
      // Không ghi đè cache gốc khi ID bị dùng cho nội dung khác.
      publishCommandAck(
          commandId,
          "REJECTED",
          "COMMAND_ID_CONFLICT");

      Serial.println("Command ID conflict");
    }

    return;
  }
  // 1. Kiểm tra action trước.
  if (action != "SET_MODE" && action != "SET_STATE")
  {
    rejectAndCache(
        commandId,
        normalizedContent,
        "INVALID_ACTION");
    return;
  }

  // 2. Kiểm tra actuator sau.
  if (actuator != "FAN")
  {
    rejectAndCache(
        commandId,
        normalizedContent,
        "INVALID_ACTUATOR");
    return;
  }

  // 3. Mỗi action phải có đúng một trường target tương ứng.
  if ((action == "SET_MODE" && (!hasMode || hasState)) ||
      (action == "SET_STATE" && (!hasState || hasMode)))
  {
    publishCommandAck(
        commandId,
        "REJECTED",
        "INVALID_PAYLOAD");
    return;
  }
  if (action == "SET_MODE")
  {
    if (targetMode != "AUTO" && targetMode != "MANUAL")
    {
      rejectAndCache(
          commandId, normalizedContent, "INVALID_TARGET_MODE");
      return;
    }
  }

  if (action == "SET_STATE")
  {
    if (targetState != "ON" && targetState != "OFF")
    {
      rejectAndCache(
          commandId, normalizedContent, "INVALID_TARGET_STATE");
      return;
    }
  }
  if (!isClockTrusted())
  {
    rejectAndCache(
        commandId, normalizedContent, "CLOCK_UNTRUSTED");
    return;
  }

  time_t expiryTime = 0;

  if (!parseUtcExpiry(expiresAt.c_str(), expiryTime))
  {
    rejectAndCache(
        commandId, normalizedContent, "INVALID_EXPIRY");
    return;
  }

  time_t now = time(nullptr);

  if (expiryTime <= now)
  {
    rejectAndCache(
        commandId, normalizedContent, "EXPIRED");
    return;
  }

  if (expiryTime > now + 15)
  {
    rejectAndCache(
        commandId, normalizedContent, "INVALID_EXPIRY");
    return;
  }
  // Xử lý lệnh SET_MODE.
  if (action == "SET_MODE")
  {
    controlMode =
        (targetMode == "AUTO") ? MODE_AUTO : MODE_MANUAL;
    manualModeStartedMs = (controlMode == MODE_MANUAL) ? millis() : 0;

    if (controlMode == MODE_AUTO)
    {
      applyControlLogic();
    }

    // Publish trạng thái sau khi logic điều khiển đã ổn định.
    publishState();
    publishCommandAck(commandId, "APPLIED", "OK");

    rememberCommand(
        commandId,
        normalizedContent,
        "APPLIED",
        "OK");

    return;
  }

  // SET_STATE chỉ được thực hiện trong MANUAL.
  if (controlMode != MODE_MANUAL)
  {
    rejectAndCache(
        commandId,
        normalizedContent,
        "MODE_NOT_MANUAL");
    return;
  }

  // Kiểm tra trạng thái đích.
  if (targetState != "ON" && targetState != "OFF")
  {
    rejectAndCache(
        commandId,
        normalizedContent,
        "INVALID_TARGET_STATE");
    return;
  }

  // Không cho phép tắt quạt khi có safety override.
  if (targetState == "OFF" && safetyOverride)
  {
    rejectAndCache(
        commandId,
        normalizedContent,
        "SAFETY_OVERRIDE");
    return;
  }

  setFanState(targetState == "ON");

  publishCommandAck(commandId, "APPLIED", "OK");

  rememberCommand(
      commandId,
      normalizedContent,
      "APPLIED",
      "OK");
}
// SETUP
void setup()
{
  bootId = String((uint32_t)esp_random(), HEX) +
           String((uint32_t)esp_random(), HEX);
  Serial.begin(115200);
  loadZoneConfig();
  bootStartedMs = millis();

  // Khởi động cảm biến DHT22
  dht.begin();

  // Cấu hình chân điều khiển quạt
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, LOW);

  Serial.println();
  Serial.println("==============================");
  Serial.println("ESP32 Swiftlet Firmware");
  Serial.println("Starting sensors...");
  Serial.println("==============================");

  // Kết nối WiFi
  connectWiFi();

  // Cấu hình MQTT Broker
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCallback(mqttCallback);
  if (!mqttClient.setBufferSize(1024))
  {
    Serial.println("ERROR: MQTT buffer allocation failed");
  }
}

// LOOP
void loop()
{
  if (controlMode == MODE_MANUAL && manualModeStartedMs && millis() - manualModeStartedMs >= MANUAL_TIMEOUT_MS)
  {
    controlMode = MODE_AUTO;
    manualModeStartedMs = 0;
    Serial.println("[MODE] MANUAL timeout -> AUTO");
    applyControlLogic();
  }
  maintainWiFi();
  if (WiFi.status() == WL_CONNECTED && !ntpConfigured)
  {
    configTime(0, 0, "pool.ntp.org", "time.google.com");
    ntpConfigured = true;
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    tryConnectMQTT();
  }
  if (mqttClient.connected())
  {
    mqttClient.loop();
    resendBufferedMessages();

    if (millis() - lastState >= STATE_INTERVAL)
    {
      lastState = millis();
      publishState();
    }
  }

  // 2. Đọc cảm biến định kỳ mỗi 2 giây
  if (millis() - lastSensorSampleMs >= SENSOR_INTERVAL)
  {
    lastSensorSampleMs = millis();

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    int light = analogRead(LDR_PIN);
    int co2Raw = analogRead(CO2_PIN);
    int co2 = map(co2Raw, 0, 4095, 400, 2000);

    bool dhtValid = !isnan(humidity) && !isnan(temperature);
    latestCo2 = co2;
    latestLight = light;
    latestCo2Valid = true;   // Cảm biến CO2 đang được mô phỏng bằng ADC.
    latestLightValid = true; // Cảm biến ánh sáng đang được mô phỏng bằng ADC.

    // Hai cảm biến này hiện đang được mô phỏng.
    bool co2Valid = true;
    bool lightValid = true;

    latestDhtValid = dhtValid;
    if (dhtValid)
    {
      if (dhtGoodStreak < 3)
        dhtGoodStreak++;
      if (dhtFaultActive && dhtGoodStreak >= 3)
      {
        dhtFaultActive = false;
        publishSensorEvent("SENSOR_RECOVERED", "DHT22", "THREE_GOOD_READINGS");
      }
    }
    else
    {
      dhtGoodStreak = 0;
      if (!dhtFaultActive)
      {
        dhtFaultActive = true;
        publishSensorEvent("SENSOR_FAULT", "DHT22", "INVALID_OR_NAN_READING");
      }
    }

    if (dhtValid)
    {
      latestTemperature = temperature;
      latestHumidity = humidity;
      haveValidTemperature = true;
      lastValidTemperatureMs = millis();

      Serial.printf(
          "Temperature: %.2f C | Humidity: %.2f %%\n",
          temperature,
          humidity);
    }
    else
    {
      Serial.println("DHT22: ERROR");
    }

    Serial.printf("CO2: %d ppm\n", co2);
    Serial.printf("Light: %d\n", light);

    // Luôn chạy logic an toàn, kể cả khi DHT22 bị lỗi.
    applyControlLogic();

    Serial.printf(
        "Mode: %s | Fan: %s | Safety: %s\n",
        controlMode == MODE_AUTO ? "AUTO" : "MANUAL",
        fanState ? "ON" : "OFF",
        safetyOverride ? "ON" : "OFF");
  }

  // Lịch telemetry độc lập với chu kỳ đọc cảm biến (mỗi 5 giây).
  // Dữ liệu cảm biến lỗi sẽ được publish dưới dạng null bởi publishTelemetry().
  if (millis() - lastTelemetry >= TELEMETRY_INTERVAL)
  {
    lastTelemetry = millis();
    publishTelemetry(
        latestTemperature,
        latestHumidity,
        latestCo2,
        latestLight,
        latestDhtValid && haveValidTemperature &&
            (millis() - lastValidTemperatureMs <= SENSOR_STALE_MS),
        latestCo2Valid,
        latestLightValid);
  }

  static unsigned long lastResourceLog = 0;
  if (millis() - lastResourceLog >= 60000UL)
  {
    lastResourceLog = millis();
    Serial.printf("[RESOURCE] uptime_ms=%lu free_heap=%u buffer=%d/%d dropped=%lu\\n",
                  (unsigned long)millis(), (unsigned int)ESP.getFreeHeap(),
                  bufferCount, BUFFER_SIZE, (unsigned long)droppedTelemetryCount);
  }

  // 6. Gửi heartbeat mỗi 2 giây
  static unsigned long resourceLogAt = 0;
  if (millis() - resourceLogAt >= 60000UL)
  {
    resourceLogAt = millis();
    Serial.printf("[RESOURCE] uptime_ms=%lu free_heap=%u buffer=%d/%d dropped=%lu\n", (unsigned long)millis(), (unsigned int)ESP.getFreeHeap(), bufferCount, BUFFER_SIZE, (unsigned long)droppedTelemetryCount);
  }
  if (millis() - lastHeartbeat >= HEARTBEAT_INTERVAL)
  {
    lastHeartbeat = millis();
    publishHeartbeat();
  }

  delay(200);
}
void setupClock()
{
  configTime(0, 0, "pool.ntp.org", "time.google.com");
}

bool getTimestamp(char *output, size_t outputSize)
{
  time_t now = time(nullptr);

  // Không dùng thời gian nếu đồng hồ chưa được đồng bộ.
  if (now < 1760000000)
  {
    return false;
  }

  struct tm timeInfo;
  gmtime_r(&now, &timeInfo);

  strftime(
      output,
      outputSize,
      "%Y-%m-%dT%H:%M:%SZ",
      &timeInfo);

  return true;
}
bool isClockTrusted()
{
  time_t now = time(nullptr);

  // Chỉ coi đồng hồ đáng tin sau khi đã đồng bộ.
  return now >= 1760000000;
}