#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ESP32Servo.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <time.h>

// WiFi details
const char* ssid = "Newera2G";
const char* password = "newera123456";
String serverName = "www.circuitdigest.cloud";  // Replace with your server domain
String serverPath = "/readnumberplate";        // API endpoint path
const int serverPort = 443;                    // HTTPS port
String apiKey = "bX6L7SyylBdU";                // Replace with your API keyd

String nodeRedUrl = "http://192.168.0.199:1880/capture";

// Pin Definitions
#define IR_SENSOR_OUT 15
#define IR_SENSOR_PIN 14  // GPIO pin connected to the IR sensor
#define flashLight 4      // GPIO pin for the flashlight
#define SERVO_PIN 13      // GPIO pin for servo motor control

int count = 0;            // Counter for image uploads
unsigned long timeIn = 0; // Timestamp for car entry
unsigned long timeOut = 0; // Timestamp for car exit

Servo myServo;            // Create a Servo object
int servoPosition = 0;    // Initial servo position

WiFiClientSecure client;  // Secure client for HTTPS communication

// Camera GPIO pins - adjust based on your ESP32-CAM board
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// Function prototypes
int sendPhoto();
void controlServo();

void sendToNodeRed(String numberPlate, String imageUrl, unsigned long timeIn, unsigned long timeOut) {
  HTTPClient http;
  
  http.begin(nodeRedUrl);
  http.addHeader("Content-Type", "application/json");

  String formattedTimeIn = getFormattedTimeFromMillis(timeIn);  // Convert millis to formatted time
  String formattedTimeOut = getFormattedTimeFromMillis(timeOut);  // Convert millis to formatted time
  
  String payload = "{\"number_plate\":\"" + numberPlate + 
                   "\",\"image_url\":\"" + imageUrl + 
                   "\",\"time_in\":\"" + formattedTimeIn + 
                   "\",\"time_out\":\"" + formattedTimeOut + "\"}";
  int httpResponseCode = http.POST(payload);

  if (httpResponseCode > 0) {
    Serial.println("POST Success, response:");
    Serial.println(http.getString());
  } else {
    Serial.print("POST Failed, error: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

// Function to extract a JSON string value by key
String extractJsonStringValue(const String& jsonString, const String& key) {
  int keyIndex = jsonString.indexOf(key);
  if (keyIndex == -1) {
    return "";
  }

  int startIndex = jsonString.indexOf(':', keyIndex) + 2;
  int endIndex = jsonString.indexOf('"', startIndex);

  if (startIndex == -1 || endIndex == -1) {
    return "";
  }

  return jsonString.substring(startIndex, endIndex);
}

void setup() {
  // Disable brownout detector
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  pinMode(flashLight, OUTPUT);
  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(IR_SENSOR_OUT, INPUT);
  digitalWrite(flashLight, LOW);
  

  // Initialize Servo
  myServo.attach(SERVO_PIN);
  myServo.write(0);  // Set initial position to 0 degrees

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP());
    configTime(0, 0, "pool.ntp.org");  // Set your timezone
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  } else {
    Serial.print("Current time: ");
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  }

  // Initialize Servo
  myServo.attach(SERVO_PIN);
  myServo.write(0);

  // Configure camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 5;  // Lower number means higher quality
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    delay(1000);
    ESP.restart();
  }
}
String getFormattedTimeFromMillis(unsigned long millisValue) {
  if (millisValue == 0) {
    return "0";  // Return "0" if the time is not set
  }
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "";
  }
  time_t now = time(nullptr) + (millisValue / 1000);
  localtime_r(&now, &timeinfo);
  char timeStr[50];
  strftime(timeStr, sizeof(timeStr), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  return String(timeStr);
}

void loop() {
  if (digitalRead(IR_SENSOR_PIN) == LOW) {
    timeIn = millis();  // Capture time when car enters
    timeOut = 0;        // Reset timeOut when entering
    String formattedTimeIn = getFormattedTimeFromMillis(timeIn);
    Serial.println("Car detected IN! Capturing photo...");
    Serial.println("Time In: " + formattedTimeIn);
    
    digitalWrite(flashLight, HIGH);
    delay(500);
    openGate();  
    int status = sendPhoto();  // Capture and send photo
    digitalWrite(flashLight, LOW);

    while (digitalRead(IR_SENSOR_PIN) == LOW) {
      delay(100);
    }
    closeGate();  
    if (status == -1) {
      Serial.println("Image Capture Failed");
    } else if (status == -2) {
      Serial.println("Server Connection Failed");
    }
    delay(2000);
  }

  // Detect car exit
  if (digitalRead(IR_SENSOR_OUT) == LOW) {
    timeOut = millis();  // Capture time when car exits
    timeIn = 0;          // Reset timeIn when exiting
    String formattedTimeOut = getFormattedTimeFromMillis(timeOut);
    Serial.println("Car detected OUT! Capturing photo...");
    Serial.println("Time Out: " + formattedTimeOut);

    digitalWrite(flashLight, HIGH);
    delay(500);
    openGate();  // Open the gate
    int status = sendPhoto();  // Capture and send photo
    digitalWrite(flashLight, LOW);

    while (digitalRead(IR_SENSOR_OUT) == LOW) {
      delay(100);
    }
    closeGate();  // Close the gate after car leaves

    if (status == -1) {
      Serial.println("Image Capture Failed");
    } else if (status == -2) {
      Serial.println("Server Connection Failed");
    }
    delay(2000);
  }
}



void openGate() {
  Serial.println("Opening gate...");
  myServo.write(90); // Rotate servo to 90 degrees
  delay(2000);       // Keep the gate open for 2 seconds
}

// Function to close the gate
void closeGate() {
  Serial.println("Closing gate...");
  myServo.write(0); // Rotate servo back to 0 degrees
  delay(2000);      // Ensure the gate has time to close
}

// Function to capture and send photo to the server
int sendPhoto() {
  camera_fb_t* fb = NULL;

  // Capture image
  delay(100);
  fb = esp_camera_fb_get();
  delay(100);

  if (!fb) {
    Serial.println("Camera capture failed");
    return -1;
  }

  // Connect to server
  Serial.println("Connecting to server:" + serverName);
  client.setInsecure();  // Skip certificate validation for simplicity

  if (client.connect(serverName.c_str(), serverPort)) {
    Serial.println("Connection successful!");
    delay(300);
    Serial.println("Data Uploading !");

    // Increment count and prepare file name
    count++;
    String filename = apiKey + ".jpeg";

    // Prepare HTTP POST request
    String head = "--CircuitDigest\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"" + filename + "\"\r\nContent-Type: image/jpeg\r\n\r\n";
    String tail = "\r\n--CircuitDigest--\r\n";
    uint32_t imageLen = fb->len;
    uint32_t extraLen = head.length() + tail.length();
    uint32_t totalLen = imageLen + extraLen;

    client.println("POST " + serverPath + " HTTP/1.1");
    client.println("Host: " + serverName);
    client.println("Content-Length: " + String(totalLen));
    client.println("Content-Type: multipart/form-data; boundary=CircuitDigest");
    client.println("Authorization:" + apiKey);
    client.println();
    client.print(head);

    // Send image data in chunks
    uint8_t* fbBuf = fb->buf;
    size_t fbLen = fb->len;
    for (size_t n = 0; n < fbLen; n += 1024) {
      if (n + 1024 < fbLen) {
        client.write(fbBuf, 1024);
        fbBuf += 1024;
      } else {
        size_t remainder = fbLen % 1024;
        client.write(fbBuf, remainder);
      }
    }

    client.print(tail);

    // Clean up
    esp_camera_fb_return(fb);
    Serial.println("Waiting For Response!");

    // Wait for server response
    String response;
    long startTime = millis();
    while (client.connected() && millis() - startTime < 5000) {
      if (client.available()) {
        char c = client.read();
        response += c;
      }
    }

    // Extract and display NPR data from response
    String NPRData = extractJsonStringValue(response, "\"number_plate\"");
    String imageLink = extractJsonStringValue(response, "\"view_image\"");

    Serial.print("Response: ");
    Serial.println(response);

    client.stop();

    // Pass the timeIn and timeOut values to the sendToNodeRed function
    sendToNodeRed(NPRData, imageLink, timeIn, timeOut);

    return 0;
  } else {
    Serial.println("Connection failed");
    return -2;
  }
}