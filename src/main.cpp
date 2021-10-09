#include <cstdlib>
#include <Wire.h>
#include <SSD1306Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// WIFI
#define WLAN_SSID "WIFI_NAME"
#define WLAN_PASS "WIFI_PASS"

// PINS
#define TRIGGER_PIN D6
#define ECHO_PIN D5
#define SDA_PIN D2
#define SCL_PIN D1

// CONSTANTS
#define MAX_DISTANCE 1000
#define MIN_DISTANCE 0
#define POLLING_RATE 5000

// API
#define API_URL "http://192.168.0.119:8000/log"

SSD1306Wire display(0x3c, SDA_PIN, SCL_PIN, GEOMETRY_128_32);
WiFiClient client;

long duration = 0;
long volume = 0;
float distance = 0;

void initDisplay() {
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_CENTER);

  display.drawString(64, 4, "Monitor volume");
  display.drawString(64, 16, "Water container");
  display.display();
}

void connectWifi() {
  display.clear();
  display.drawString(64, 4, "Connecting to");
  display.drawString(64, 16, String(WLAN_SSID));
  display.display();

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  display.clear();
  display.drawString(64, 4, "Connected, IP:");
  display.drawString(64, 16, WiFi.localIP().toString());
  display.display();
}

void triggerUltrasound() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
}

void displayValues() {
  display.clear();
  display.drawString(64, 4, "Distance: " + String(distance) + "mm");
  display.drawString(64, 16, "Volume: " + String(volume) + "ml");
  display.display();
}

void sendValuesToApi() {
  if(WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(client, API_URL);
    http.addHeader("Content-Type", "application/json");
    String body = "{\"duration\": " + String(duration) + ",\"distance\":" + String(distance) + ",\"volume\":" + String(volume) + "}";
    http.POST(body);
    http.end();
  }
}

void setup()
{
  Serial.begin(9600);

  initDisplay();
  delay(3000);

  connectWifi();
  delay(3000);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  display.clear();
  display.drawString(64, 12, "Initiating...");
  display.display();
  delay(2000);
}

void loop() {
  triggerUltrasound();

  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.34 / 2;
  volume = map(distance, MIN_DISTANCE, MAX_DISTANCE, 0, 1000);

  displayValues();

  sendValuesToApi();

  delay(POLLING_RATE);
}
