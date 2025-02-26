#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>

// 初始化 LCD
LiquidCrystal_I2C lcd(0x27, 16, 1); // 设置 I2C 地址、列和行

// 红外传感器引脚
const int IR_SENSOR_1 = 32; // Zone A
const int IR_SENSOR_2 = 33; // Zone B
const int IR_SENSOR_3 = 34; // Zone C

// Wi-Fi 配置
const char* ssid = "Newera2G"; // 替换为您的 Wi-Fi 名称
const char* password = "newera123456"; // 替换为您的 Wi-Fi 密码

// Node-RED 服务器地址
const char* serverUrl = "http://192.168.0.199:1880/parking"; // 替换为您的 Node-RED 服务器地址

// 车位总数
const int TOTAL_SPACES = 3;
int availableSpaces = TOTAL_SPACES; // 当前可用车位数量

// 定义每个区域的状态
bool zoneStatus[3] = {false, false, false}; // Zone A, Zone B, Zone C (false = available, true = occupied)

void setup() {
  Serial.begin(115200);  // 初始化串口
  // 初始化 LCD
  lcd.init();
  lcd.backlight();

  // 初始化红外传感器
  pinMode(IR_SENSOR_1, INPUT);
  pinMode(IR_SENSOR_2, INPUT);
  pinMode(IR_SENSOR_3, INPUT);

  // 显示初始状态
  lcd.setCursor(0, 0);
  lcd.print("Available:");
  lcd.setCursor(10, 0);
  lcd.print(availableSpaces);

  // 连接 Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED && retryCount < 30) { // Retry for 15 seconds
    delay(500);
    Serial.print(".");
    retryCount++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi. Check credentials or network availability.");
    Serial.println("WiFi Status Code: " + String(WiFi.status()));
    return; // Prevent further execution if Wi-Fi fails
  }
}

void loop() {
  // Reconnect to Wi-Fi if disconnected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Reconnecting...");
    WiFi.begin(ssid, password);
    int retryCount = 0;
    while (WiFi.status() != WL_CONNECTED && retryCount < 30) {
      delay(500);
      Serial.print(".");
      retryCount++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWiFi Reconnected");
    } else {
      Serial.println("\nFailed to reconnect to WiFi.");
    }
  }

  // 检测传感器状态并更新区域状态
  zoneStatus[0] = digitalRead(IR_SENSOR_1) == LOW; // Zone A
  zoneStatus[1] = digitalRead(IR_SENSOR_2) == LOW; // Zone B
  zoneStatus[2] = digitalRead(IR_SENSOR_3) == LOW; // Zone C

  // 计算可用车位数量
  int occupiedSpaces = 0;
  for (int i = 0; i < 3; i++) {
    if (zoneStatus[i]) occupiedSpaces++;
  }
  int newAvailableSpaces = TOTAL_SPACES - occupiedSpaces;

  // 防止负值显示
  if (newAvailableSpaces < 0) newAvailableSpaces = 0;

  // 更新 LCD 显示
  lcd.setCursor(10, 0);
  lcd.print("  "); // 清除旧数据
  lcd.setCursor(10, 0);
  lcd.print(newAvailableSpaces);

  if (newAvailableSpaces == 0) {
    lcd.setCursor(0, 1);
    lcd.print("Parking: FULL ");
  } else {
    lcd.setCursor(0, 1);
    lcd.print("Parking: OPEN ");
  }

  // 在串口显示剩余车位数量
  Serial.print("Available Spaces: ");
  Serial.println(newAvailableSpaces);

  // 发送数据到 Node-RED
  sendToNodeRED(newAvailableSpaces, zoneStatus);

  delay(500); // 每 500 毫秒更新一次
}

void sendToNodeRED(int spaces, bool zoneStatus[]) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverUrl); // Connect to Node-RED server
    http.addHeader("Content-Type", "application/json"); // Set HTTP header

    // Create JSON data
    String jsonData = "{\"availableSpaces\":" + String(spaces) + ",";
    jsonData += "\"zones\":["; // Array for each zone's status
    jsonData += String(zoneStatus[0] ? "0" : "1") + ","; // Zone A
    jsonData += String(zoneStatus[1] ? "0" : "1") + ","; // Zone B
    jsonData += String(zoneStatus[2] ? "0" : "1") + "]}"; // Zone C
    Serial.print("Sending data to Node-RED: ");
    Serial.println(jsonData);

    // Send HTTP POST request
    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);
      if (httpResponseCode == 200) {
        Serial.println("Data sent successfully to Node-RED!");
      } else {
        Serial.println("Unexpected response from server.");
      }
    } else {
      Serial.print("Error sending data, HTTP Response Code: ");
      Serial.println(httpResponseCode);
    }

    http.end(); // End HTTP request
  } else {
    Serial.println("WiFi not connected");
  }
}