#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <time.h>

// Firebase helpers
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Wi-Fi credentials
#define WIFI_SSID "ooredoo_9B246A"
#define WIFI_PASSWORD "qyiixorv"

// Firebase credentials
#define API_KEY "AIzaSyArW_E21jl5o_HFr18fjfhBfiLj3NBU1gg"
#define DATABASE_URL "https://smarttraffic-815ad-default-rtdb.europe-west1.firebasedatabase.app/" 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long readDataPrevMillis = 0;
bool signupOK = false;

// Road structure
struct RoadData {
  bool accident;
  int density;
  int greenTime;
  int id;
  String timestamp;
};

// Store 4 roads
RoadData roads[4];

// LED pins (Update as needed)
int roadLights[4][3] = {
  {18, 19, 21},  // Road A: Green, Orange, Red
  {13, 12, 14},  // Road B
  {26, 25, 33},  // Road C (Updated per your request)
  {22, 23, 5}     // Road D
};

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Setup pins
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      pinMode(roadLights[i][j], OUTPUT);
      digitalWrite(roadLights[i][j], LOW);
    }
  }

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected: " + WiFi.localIP().toString());

  // Sync time (required for Firebase SSL)
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Syncing time");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println(" done!");

  // Firebase config
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  config.token_status_callback = tokenStatusCallback;
  

  Firebase.reconnectWiFi(true);

  // Anonymous sign-in
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase anonymous signup OK");
    signupOK = true;
  } else {
    Serial.printf("Firebase signup failed: %s\n", config.signer.signupError.message.c_str());
  }

  Firebase.begin(&config, &auth);
}

void turnOnGreen(int road) {
  // Turn off all LEDs
  for (int i = 0; i < 4; i++) {
    digitalWrite(roadLights[i][0], LOW); // Green
    digitalWrite(roadLights[i][1], LOW); // Orange
    digitalWrite(roadLights[i][2], HIGH); // Red
  }

  // Green ON for selected road
  digitalWrite(roadLights[road][0], HIGH); // Green
  digitalWrite(roadLights[road][2], LOW);  // Red
}

void readRoadData(int roadNumber) {
  String roadPath = "smartTraffic/road_" + String(roadNumber);
  RoadData &r = roads[roadNumber - 1];

  if (Firebase.RTDB.getBool(&fbdo, roadPath + "/accident"))
    r.accident = fbdo.boolData();

  if (Firebase.RTDB.getInt(&fbdo, roadPath + "/density"))
    r.density = fbdo.intData();

  if (Firebase.RTDB.getInt(&fbdo, roadPath + "/green_time"))
    r.greenTime = fbdo.intData();

  if (Firebase.RTDB.getInt(&fbdo, roadPath + "/id"))
    r.id = fbdo.intData();

  if (Firebase.RTDB.getString(&fbdo, roadPath + "/timestamp"))
    r.timestamp = fbdo.stringData();

  Serial.println("Road " + String(roadNumber) + ": Density=" + r.density + ", GreenTime=" + r.greenTime + ", Accident=" + r.accident);
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - readDataPrevMillis > 10000 || readDataPrevMillis == 0)) {
    readDataPrevMillis = millis();

    Serial.println("\nReading Firebase data...");

    for (int i = 1; i <= 4; i++) {
      readRoadData(i);
    }

    for (int i = 0; i < 4; i++) {
      if (!roads[i].accident) {
        Serial.println("Granting green light to Road " + String(i + 1));
        turnOnGreen(i);
        delay(roads[i].greenTime * 1000); // Green time in seconds
      }
    }
  }
}