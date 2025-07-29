
#include <WiFi.h>
#include <Firebase_Client.h> 
#include <ArduinoJson.h>


// Our WiFi credentials
#define WIFI_SSID "MAKERSPACE"                     // input home wifi name
#define WIFI_PASSWORD "12345678"                  // password of wifi ssid

// Our Firebase credentials
#define API_KEY "AIzaSyB3pOoyKCrTZ6eX1uIjMQAr2PlG3mj_eXA"
#define DATABASE_URL "https://control-led-27-default-rtdb.firebaseio.com/" //project name address from firebase id

// RGB LED pins
#define RED_PIN    25
#define GREEN_PIN  26
#define BLUE_PIN   27

// PWM channels
#define RED_CH     0
#define GREEN_CH   1
#define BLUE_CH    2

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

void setup() {
  Serial.begin(115200);

  // PWM setup for RGB pins
  ledcAttachChannel(RED_PIN, 5000, 8, RED_CH);
  ledcAttachChannel(GREEN_PIN, 5000, 8, GREEN_CH);
  ledcAttachChannel(BLUE_PIN, 5000, 8, BLUE_CH);

// Connect to Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" connected.");

// Firebase setup
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  }

void loop() {
  if (Firebase.RTDB.getJSON(&fbdo, "/ledColor")) {
    if (fbdo.dataType() == "json") {
      FirebaseJson& json = fbdo.to<FirebaseJson>();
      FirebaseJsonData r,g,b;
      json.get(r, "r");
      json.get(g, "g");
      json.get(b, "b");

      uint8_t rVal, gVal, bVal;

      //turn firebase json values into integers
      rVal = r.to<uint8_t>();
      gVal =  g.to<uint8_t>();
      bVal = b.to<uint8_t>();

      Serial.printf("RGB from Firebase: R=%u G=%u B=%u\n", rVal, gVal, bVal);

      setColor(rVal, gVal, bVal);
    }

  } else {
    Serial.print("Firebase error: ");
    Serial.println(fbdo.errorReason());
  }

  delay(1000); //every second
}

void setColor(int r, int g, int b) {
  ledcWriteChannel(RED_CH, r);
  ledcWriteChannel(GREEN_CH, g);
  ledcWriteChannel(BLUE_CH, b);
} 