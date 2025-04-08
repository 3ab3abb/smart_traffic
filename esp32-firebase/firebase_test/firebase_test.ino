#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "ooredoo_9B246A"
#define WIFI_PASSWORD "qyiixorv"

// Insert Firebase project API Key
#define API_KEY "AIzaSyArW_E21jl5o_HFr18fjfhBfiLj3NBU1gg"

// Insert RTDB URL
#define DATABASE_URL "https://smarttraffic-815ad-default-rtdb.europe-west1.firebasedatabase.app/" 

// Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long readDataPrevMillis = 0;
bool signupOK = false;

// Structure to hold road data
struct RoadData {
  bool accident;
  int density;
  int greenTime;
  int id;
  String timestamp;
};

// Array to store road data for all four roads
RoadData roads[4];

void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;
  
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  
  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase signup OK");
    signupOK = true;
  }
  else {
    Serial.printf("Firebase signup failed: %s\n", config.signer.signupError.message.c_str());
  }
  
  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void readRoadData(int roadNumber) {
  String roadPath = "smartTraffic/road_" + String(roadNumber);
  
  // Read accident status
  if (Firebase.RTDB.getBool(&fbdo, roadPath + "/accident")) {
    roads[roadNumber-1].accident = fbdo.boolData();
    Serial.println("Road " + String(roadNumber) + " accident: " + String(roads[roadNumber-1].accident));
  } else {
    Serial.println("Failed to read accident data for road " + String(roadNumber));
    Serial.println("REASON: " + fbdo.errorReason());
  }
  
  // Read density
  if (Firebase.RTDB.getInt(&fbdo, roadPath + "/density")) {
    roads[roadNumber-1].density = fbdo.intData();
    Serial.println("Road " + String(roadNumber) + " density: " + String(roads[roadNumber-1].density));
  } else {
    Serial.println("Failed to read density data for road " + String(roadNumber));
  }
  
  // Read green_time
  if (Firebase.RTDB.getInt(&fbdo, roadPath + "/green_time")) {
    roads[roadNumber-1].greenTime = fbdo.intData();
    Serial.println("Road " + String(roadNumber) + " green time: " + String(roads[roadNumber-1].greenTime));
  } else {
    Serial.println("Failed to read green_time data for road " + String(roadNumber));
  }
  
  // Read ID
  if (Firebase.RTDB.getInt(&fbdo, roadPath + "/id")) {
    roads[roadNumber-1].id = fbdo.intData();
    Serial.println("Road " + String(roadNumber) + " ID: " + String(roads[roadNumber-1].id));
  } else {
    Serial.println("Failed to read ID data for road " + String(roadNumber));
  }
  
  // Read timestamp
  if (Firebase.RTDB.getString(&fbdo, roadPath + "/timestamp")) {
    roads[roadNumber-1].timestamp = fbdo.stringData();
    Serial.println("Road " + String(roadNumber) + " timestamp: " + roads[roadNumber-1].timestamp);
  } else {
    Serial.println("Failed to read timestamp data for road " + String(roadNumber));
  }
  
  Serial.println("-----------------------------------");
}

void loop() {
  if (Firebase.ready() && signupOK && (millis() - readDataPrevMillis > 5000 || readDataPrevMillis == 0)) {
    readDataPrevMillis = millis();
    
    Serial.println("Reading data from Firebase...");
    
    // Read data for all four roads
    for (int i = 1; i <= 4; i++) {
      readRoadData(i);
    }
    
    // Here you can add your logic based on the road data
    // For example, determine which road has highest priority, etc.
    
    Serial.println("=================================");
  }
}