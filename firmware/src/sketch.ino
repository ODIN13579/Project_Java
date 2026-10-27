#include <WiFi.h>         // ESP32 kết nối WiFi để giao tiếp với MQTT Broker
#include <PubSubClient.h> // Thư viện MQTT cho ESP32
#include "DHT.h"          // Thư viện cảm biến DHT22
#include <time.h>
#include <esp_system.h>

// SENSOR & DEVICE PIN
#define DHTPIN 15     // Chân kết nối DHT22
#define DHTTYPE DHT22 // Loại cảm biến DHT
#define LDR_PIN 34    // Chân đọc cảm biến ánh sáng
#define CO2_PIN 27    // Chân đọc cảm biến CO2
#define FAN_PIN 4     // Chân điều khiển quạt

DHT dht(DHTPIN, DHTTYPE); // Tạo đối tượng DHT22

// FAN THRESHOLD
#define FAN_ON_TEMP 31.0  // Nhiệt độ >= 31°C thì bật quạt
#define FAN_OFF_TEMP 30.0 // Nhiệt độ <= 30°C thì tắt quạt

bool fanState = false; // false = quạt tắt, true = quạt bật

// WIFI CONFIGURATION
const char *WIFI_SSID = "Wokwi-GUEST";
const char *WIFI_PASSWORD = "";

// MQTT CONFIGURATION
const char *MQTT_HOST = "host.wokwi.internal"; // Địa chỉ MQTT Broker
const int MQTT_PORT = 1883;                    // Cổng MQTT Broker

const char *DEVICE_ID = "ESP32-HOUSE01-ZONE01";

// Topic gửi dữ liệu cảm biến
const char *TELEMETRY_TOPIC =
    "swiftletcare/1/1/ESP32-HOUSE01-ZONE01/telemetry";

// Topic gửi trạng thái hoạt động của thiết bị
const char *HEARTBEAT_TOPIC =
    "swiftletcare/1/1/ESP32-HOUSE01-ZONE01/heartbeat";

// MQTT CLIENT
WiFiClient espClient;               // Tạo kết nối TCP/IP
PubSubClient mqttClient(espClient); // Tạo MQTT Client

// TIMERS
unsigned long lastTelemetry = 0;
unsigned long lastHeartbeat = 0;

const unsigned long TELEMETRY_INTERVAL = 5000; // Gửi telemetry mỗi 5 giây
const unsigned long HEARTBEAT_INTERVAL = 2000; // Gửi heartbeat mỗi 2 giây

unsigned long messageCounter = 0; // Đếm số message đã tạo

// WIFI CONNECT
void connectWiFi()
{
  Serial.print("Connecting to WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);

  // Chờ đến khi ESP32 kết nối WiFi thành công
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// OFFLINE TELEMETRY BUFFER
#define BUFFER_SIZE 20 // Bộ nhớ tạm chứa tối đa 20 message

// Cấu trúc lưu message khi MQTT bị mất kết nối
struct BufferedMessage
{
  String topic;
  String payload;
};

BufferedMessage messageBuffer[BUFFER_SIZE];

int bufferHead = 0;  // Vị trí message đầu tiên trong buffer
int bufferTail = 0;  // Vị trí thêm message mới
int bufferCount = 0; // Số message hiện đang được lưu

// Thêm message vào buffer khi MQTT không gửi được
bool addToBuffer(const char *topic, const char *payload)
{
  if (bufferCount >= BUFFER_SIZE)
  {
    Serial.println("BUFFER FULL - message dropped");
    return false;
  }

  messageBuffer[bufferTail].topic = topic;
  messageBuffer[bufferTail].payload = payload;

  bufferTail = (bufferTail + 1) % BUFFER_SIZE;
  bufferCount++;

  Serial.print("BUFFERED message. Count = ");
  Serial.println(bufferCount);

  return true;
}

// Gửi lại các message đã lưu trong buffer
// khi MQTT kết nối trở lại
void resendBufferedMessages()
{
  if (!mqttClient.connected())
  {
    return;
  }

  while (bufferCount > 0)
  {
    Serial.print("RESEND: ");
    Serial.println(messageBuffer[bufferHead].payload);

    bool success = mqttClient.publish(
        messageBuffer[bufferHead].topic.c_str(),
        messageBuffer[bufferHead].payload.c_str());

    // Nếu gửi thất bại thì giữ message trong buffer
    if (!success)
    {
      Serial.println("RESEND FAILED - keep in buffer");
      return;
    }

    Serial.println("RESEND SUCCESS");

    bufferHead = (bufferHead + 1) % BUFFER_SIZE;
    bufferCount--;

    Serial.print("BUFFER REMAINING = ");
    Serial.println(bufferCount);
  }
}

// MQTT CONNECT
unsigned long lastMQTTAttempt = 0;
const unsigned long MQTT_RETRY_INTERVAL = 3000; // Thử reconnect mỗi 3 giây

// Kiểm tra và kết nối lại MQTT khi bị mất kết nối
void tryConnectMQTT()
{
  if (mqttClient.connected())
  {
    return;
  }

  // Không thử kết nối liên tục, chỉ thử sau mỗi 3 giây
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
  }
  else
  {
    Serial.print("FAILED, rc=");
    Serial.println(mqttClient.state());
  }
}

// TELEMETRY
bool getUtcTimestamp(char *output, size_t size)
{
  time_t now = time(nullptr);

  // Chưa đồng bộ được thời gian thì chưa gửi bản tin.
  if (now < 1700000000)
  {
    Serial.println("Waiting for NTP time...");
    return false;
  }

  struct tm utc;
  gmtime_r(&now, &utc);
  return strftime(output, size, "%Y-%m-%dT%H:%M:%SZ", &utc) > 0;
}
// Tạo và gửi dữ liệu nhiệt độ, độ ẩm, CO2 và ánh sáng
void publishTelemetry(float temperature, float humidity, int co2, int light)
{
  char timestamp[32];

  if (!getUtcTimestamp(timestamp, sizeof(timestamp)))
  {
    return;
  }

  messageCounter++;

  char eventId[80];
  snprintf(eventId, sizeof(eventId), "%s-%08lx%08lx-%lu",
           DEVICE_ID,
           (unsigned long)esp_random(),
           (unsigned long)esp_random(),
           messageCounter);

  char payload[512];
  snprintf(
      payload, sizeof(payload),
      "{\"event_id\":\"%s\","
      "\"device_id\":\"%s\","
      "\"timestamp\":\"%s\","
      "\"temperature\":%.2f,"
      "\"humidity\":%.2f,"
      "\"co2\":%d,"
      "\"light\":%d,"
      "\"sensor_status\":\"OK\"}",
      eventId, DEVICE_ID, timestamp,
      temperature, humidity, co2, light);

  if (mqttClient.connected() &&
      mqttClient.publish(TELEMETRY_TOPIC, payload))
  {
    Serial.print("TELEMETRY published: ");
    Serial.println(payload);
  }
  else
  {
    addToBuffer(TELEMETRY_TOPIC, payload);
  }

  // Nếu MQTT mất kết nối hoặc publish thất bại
  // thì lưu message vào buffer
  addToBuffer(TELEMETRY_TOPIC, payload);
}

// HEARTBEAT
// Gửi trạng thái hoạt động của ESP32 định kỳ
void publishHeartbeat()
{
  if (!mqttClient.connected())
  {
    return;
  }

  char timestamp[32];

  if (!getUtcTimestamp(timestamp, sizeof(timestamp)))
  {
    return;
  }

  char payload[256];
  snprintf(
      payload, sizeof(payload),
      "{\"device_id\":\"%s\",\"timestamp\":\"%s\"}",
      DEVICE_ID, timestamp);

  bool sent = mqttClient.publish(HEARTBEAT_TOPIC, payload);
  Serial.println(sent ? "HEARTBEAT published" : "HEARTBEAT failed");
}

// SETUP
void setup()
{
  Serial.begin(115200);

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
  configTime(0, 0, "pool.ntp.org", "time.google.com");

  // Cấu hình MQTT Broker
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setBufferSize(1024);
}

// LOOP
void loop()
{

  // MQTT CONNECTION
  // Kiểm tra kết nối MQTT
  // Nếu mất kết nối thì thử kết nối lại
  tryConnectMQTT();

  if (mqttClient.connected())
  {
    mqttClient.loop();

    // Nếu có message đang nằm trong buffer
    // thì gửi lại khi MQTT đã kết nối
    resendBufferedMessages();
  }

  // READ SENSORS
  // Đọc nhiệt độ và độ ẩm từ DHT22
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Đọc giá trị ánh sáng
  int light = analogRead(LDR_PIN);

  // Đọc giá trị raw từ cảm biến CO2
  int co2Raw = analogRead(CO2_PIN);

  // Chuyển đổi giá trị ADC 0-4095 sang khoảng 400-2000 ppm
  int co2 = map(co2Raw, 0, 4095, 400, 2000);

  // SENSOR CHECK
  // Kiểm tra DHT22 có đọc dữ liệu hợp lệ hay không
  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("DHT22: ERROR");
  }
  else
  {

    Serial.print("Temperature: ");
    Serial.print(temperature, 2);
    Serial.println(" C");

    Serial.print("Humidity: ");
    Serial.print(humidity, 2);
    Serial.println(" %");

    // AUTO HYSTERESIS
    // Nếu nhiệt độ >= 31°C thì bật quạt
    if (!fanState && temperature >= FAN_ON_TEMP)
    {
      fanState = true;

      digitalWrite(FAN_PIN, HIGH);

      Serial.println("AUTO: FAN ON");
    }

    // Nếu nhiệt độ <= 30°C thì tắt quạt
    if (fanState && temperature <= FAN_OFF_TEMP)
    {
      fanState = false;

      digitalWrite(FAN_PIN, LOW);

      Serial.println("AUTO: FAN OFF");
    }
  }

  // Hiển thị CO2
  Serial.print("CO2: ");
  Serial.print(co2);
  Serial.println(" ppm");

  // Hiển thị ánh sáng
  Serial.print("Light: ");
  Serial.println(light);

  // Hiển thị trạng thái quạt
  Serial.print("Fan: ");
  Serial.println(fanState ? "ON" : "OFF");

  Serial.println("-------------------------");

  // TELEMETRY EVERY 5 SEC
  // Cứ mỗi 5 giây gửi dữ liệu cảm biến
  if (millis() - lastTelemetry >= TELEMETRY_INTERVAL)
  {
    lastTelemetry = millis();

    // Chỉ gửi telemetry nếu DHT22 đọc dữ liệu hợp lệ
    if (!isnan(humidity) && !isnan(temperature))
    {
      publishTelemetry(
          temperature,
          humidity,
          co2,
          light);
    }
  }

  // HEARTBEAT EVERY 2 SEC
  // Cứ mỗi 2 giây gửi heartbeat
  if (millis() - lastHeartbeat >= HEARTBEAT_INTERVAL)
  {
    lastHeartbeat = millis();

    publishHeartbeat();
  }

  // Delay 200ms trước vòng lặp tiếp theo
  delay(200);
}