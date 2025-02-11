#include <WiFi.h>
#include <WiFiClient.h>
#include <ThingSpeak.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"

// LCD Configuration
LiquidCrystal_I2C lcd(0x27, 20, 4);

// Load Cell Configuration
#define DOUT 23
#define CLK 19
HX711 scale;

// Buzzer Configuration
#define BUZZER 25

// WiFi Configuration
char ssid[] = "mywifi";
char pass[] = "1234567890";

// ThingSpeak Configuration
unsigned long myChannelNumber = 123456; 
const char *myWriteAPIKey = "6QTAXQUUDLBCBJWN";
WiFiClient client;

// Variables
int liter;
int val;
float weight;
float calibration_factor = 102500; 
unsigned long lastUploadTime = 0;
const unsigned long uploadInterval = 16000; 

// Function Declarations
void checkWiFi();
void measureWeight();
void sendToThingSpeak();

void setup() {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  pinMode(BUZZER, OUTPUT);

  scale.begin(DOUT, CLK);
  Serial.println("Remove all weight from scale");
  scale.set_scale();
  scale.tare(); 
  Serial.println("Zero factor: " + String(scale.read_average()));

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  ThingSpeak.begin(client);
}

void loop() {
  checkWiFi();
  measureWeight();
  if (millis() - lastUploadTime >= uploadInterval) {
    sendToThingSpeak();
    lastUploadTime = millis();
  }
}

//  Function Definitions 

// ✅ Check WiFi and reconnect if needed
void checkWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    WiFi.disconnect();
    WiFi.reconnect();
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 10) {
      delay(1000);
      Serial.print(".");
      attempts++;
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Reconnected to WiFi!");
    } else {
      Serial.println("WiFi reconnect failed!");
    }
  }
}

// ✅ Read weight from HX711 and display on LCD
void measureWeight() {
  scale.set_scale(calibration_factor);
  weight = scale.get_units(5);
  if (weight < 0) {
    weight = 0.00;
  }
  liter = weight * 1000;
  val = map(liter, 0, 505, 0, 100);

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("IOT Based IV Bag");
  lcd.setCursor(2, 1);
  lcd.print("Monitoring System");
  lcd.setCursor(1, 2);
  lcd.print("IV Bottle = ");
  lcd.print(liter);
  lcd.print(" mL");
  lcd.setCursor(1, 3);
  lcd.print("IV Bag Percent=");
  lcd.print(val);
  lcd.print("%");

  Serial.print("IV Bottle: ");
  Serial.print(liter);
  Serial.println(" mL");
  Serial.print("IV Bag Percent: ");
  Serial.print(val);
  Serial.println("%");
}

// Send data to ThingSpeak
void sendToThingSpeak() {
  ThingSpeak.setField(1, liter);
  ThingSpeak.setField(2, val);
  int status = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (status == 200) {
    Serial.println("Data sent to ThingSpeak");
  } else {
    Serial.println("Error sending data to ThingSpeak");
  }
}

