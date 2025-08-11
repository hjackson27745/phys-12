
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

//Sensor pin
const int sensorPin = 33;
const int ldrPin = 34;  // GPIO 34 for photoresistor input

// Calibrated Values
const int airValue = 3664;  // Measured air value
const int waterValue = 1076;  // Measured water value

// LED pins
const int dryLED = 25; //Red LED for Dry
const int moistLED = 27; //Green LED for Moist
const int wetLED = 26; //Blue LED for Very Wet
const int humidityLED = 32; // Yellow LED
const int tempLED = 15; // clear LED for high temperature alert
const int lightLED = 12; // White LED

// Thresholds
const int dryThreshold = 45; // ≤ 45% = Dry (Red LED ON)
const int moistThreshold = 79; // Between 46%-79% = Moist (Green LED ON)
const int wetThreshold = 80; // ≥ 80% = Very Wet (Blue LED ON)
const int humidityThreshold = 50;  // ≤ 50% triggers warning with yellow LED
//Temperature thresholds 
const float tempThresholdHigh = 27.0; // Temperature too hot (°C) which is 80°F
const float tempThresholdLow = 10.0; // Temperature too cold (°C) which is 50°F

// DHT Sensor Setup
#define DHTPIN 13
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE); // Create DHT sensor object

// OLED Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Light exposure duration 
unsigned long lightStartTime = 0;
unsigned long lightExposureDuration = 0;
bool isInLight = false;

unsigned long previousMillis = 0;
const unsigned long screenInterval = 8000;  // 8 seconds
int screenIndex = 0;

void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);

// OLED Initialization
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("OLED initialization failed"));
    while (true);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  //LED pins setup
  pinMode(dryLED, OUTPUT);
  pinMode(moistLED, OUTPUT);
  pinMode(wetLED, OUTPUT);
  pinMode(humidityLED, OUTPUT);
  pinMode(tempLED, OUTPUT);
  pinMode(lightLED, OUTPUT);

  dht.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  int sensorValue = analogRead(sensorPin);
  int moisturePercent = map(sensorValue, airValue, waterValue, 0, 100);
  moisturePercent = constrain(moisturePercent, 0, 100);

  // Read Humidity + Temperature
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Read Light Level
  int ldrValue = analogRead(34);  // Read analog pin
int lightFC = map(ldrValue, 0, 4095, 0, 2000);  // Convert to foot-candles
  Serial.println(lightFC);

// Initilization of the barwidth for the OLED display
  int barWidth;

  // OLED Output for Moisture + Humidity + Temperature
  display.setCursor(0, 0);
  display.print("Moisture: ");
  display.print(moisturePercent);
  display.println("%");

  display.setCursor(0, 10);
  display.print("Humidity: ");
  display.print(humidity);
  display.println("%");

  display.setCursor(0, 10);
  display.print("Temperature: ");
  display.print(temperature);
  display.println("%");

  display.setCursor(0, 20);

  // LED Control for moisture and OLED Status
  if (moisturePercent <= dryThreshold) {
    display.println("Status: Soil is DRY");
    display.println("Water Needed!"); //Red LED
    Serial.println("Soil is DRY — Red LED ON");
    digitalWrite(dryLED, HIGH);
    digitalWrite(moistLED, LOW);
    digitalWrite(wetLED, LOW);
  } 
  else if (moisturePercent >= wetThreshold) {
    display.println("Status: Soil is VERY WET");
    display.println("Stop Watering!"); //Blue LED
    Serial.println("Soil is VERY WET — Blue LED ON");
    digitalWrite(dryLED, LOW);
    digitalWrite(moistLED, LOW);
    digitalWrite(wetLED, HIGH);
  } 
  else {
    display.println("Status: Soil is MOIST");
    display.println("All Good!"); //Green LED
    Serial.println("Soil is MOIST — Green LED ON");
    digitalWrite(dryLED, LOW);
    digitalWrite(moistLED, HIGH);
    digitalWrite(wetLED, LOW);
  }

  // Temperature LED and OLED Logic
if (temperature < tempThresholdLow) {
  digitalWrite(tempLED, HIGH);  // Turn ON White LED
  display.println("TEMP TOO LOW!");
  Serial.println("TEMP TOO LOW! White LED ON");
} 
else if (temperature > tempThresholdHigh) {
  digitalWrite(tempLED, HIGH);  // Turn ON White LED
  display.println("TEMP TOO HIGH!");
  Serial.println("TEMP TOO HIGH! White LED ON");
} 
else {
  digitalWrite(tempLED, LOW);   // Turn OFF White LED
  display.println("Temperature OK");
  Serial.println("Temperature Normal! White LED OFF");
}

  // Humidity LED ; yellow LED
  display.setCursor(0, 35);
  if (!isnan(humidity)) {
    if (humidity <= humidityThreshold) {
      display.println("ALERT: Humidity LOW ");
      digitalWrite(humidityLED, HIGH);
    } else {
      display.println("Humidity is OK");
      digitalWrite(humidityLED, LOW);
    }
  } else {
    display.println("Humidity Error");
    digitalWrite(humidityLED, LOW);
  }

  // OLED Screen Cycling Logic
unsigned long now = millis();
if (now - previousMillis >= screenInterval) {
  previousMillis = now;
  screenIndex = (screenIndex + 1) % 5;  // 0 to 4
}

// OLED Display Output
display.clearDisplay();
display.setCursor(0, 0);

  switch (screenIndex) {
    case 0:
      display.print("Soil Moisture: ");
      display.print(moisturePercent);
      display.println("%");
      if (moisturePercent <= dryThreshold) {
    display.println("Status: Soil is DRY");
    display.println("Water Needed!"); //Red LED
  } 
  else if (moisturePercent >= wetThreshold) {
    display.println("Status: Soil is VERY WET");
    display.println("Stop Watering!"); //Blue LED
  } 
  else {
    display.println("Status: Soil is MOIST");
    display.println("All Good!"); //Green LED
  }
  // Progress bar for moisture
  barWidth = map(moisturePercent, 0, 100, 0, SCREEN_WIDTH - 2);
    display.drawRect(0, 45, SCREEN_WIDTH, 8, SSD1306_WHITE); // Border
    display.fillRect(1, 46, barWidth, 6, SSD1306_WHITE); // Fill
    display.print("Moisture Level");
    break;

    case 1: //Humidity
      display.print("Humidity: ");
    display.print(humidity);
    display.println("%");
    if (isnan(humidity)) {
      display.println("Sensor Error");
    } else if (humidity <= humidityThreshold) {
      display.println("WARNING: Low");
    } else {
      display.println("Humidity OK");
    }
      break;

    case 2:
  display.print("Temperature: ");
  if (isnan(temperature)) {
    display.println("Error");
  } else {
    display.print(temperature);
    display.println(" C");
  }
  // Warnings
  if (temperature < tempThresholdLow) {
    display.println("TEMP TOO LOW!");
  } else if (temperature > tempThresholdHigh) {
    display.println("TEMP TOO HIGH!");
  } else {
    display.println("Temperature OK");
  }
  // Progress Bar
  barWidth = map(temperature, 0, 50, 0, SCREEN_WIDTH - 2);
  barWidth = constrain(barWidth, 0, SCREEN_WIDTH - 2);
  display.drawRect(0, 45, SCREEN_WIDTH, 8, SSD1306_WHITE); // Border
  display.fillRect(1, 46, barWidth, 6, SSD1306_WHITE); // Fill
  break;

  case 3: 
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.print("Light Level: ");
  display.print(lightFC);
  display.println(" FC");

//Show ONLY the correct category and corresponding plants
  if (lightFC < 1000) {
    display.println("Too Dark");
    digitalWrite(lightLED, LOW); // Turn OFF LED
  } 
  else if (lightFC <= 1200) {
    display.println("Low Light Plant");
    display.println("Plants: ZZ, Snake, Peace lilies, etc.");
    digitalWrite(lightLED, millis() % 3000 < 2000 ? HIGH : LOW); // Blink LED
  } 
  else if (lightFC <= 1450) {
    display.println("Med Light Plant");
    display.println("Plants: Pothos, Dracaena, Anthurium, etc.");
    digitalWrite(lightLED, millis() % 3000 < 2000 ? HIGH : LOW); // Blink LED
  } 
  else {
    display.println("High Light Plant");
    display.println("Plants: Cactus, Aloe Vera, Jade plants, etc.");
    digitalWrite(lightLED, millis() % 3000 < 2000 ? HIGH : LOW); // Blink LED
  }

  display.setCursor(0, 40);
  if (isInLight) {
    int minutes = lightExposureDuration / 60000;
    display.print("Sunlight: ");
    display.print(minutes);
    display.println(" min");
  } else {
    display.println("Currently: Dark");
  }
  break;
  }

  display.display();
  delay(500);
}