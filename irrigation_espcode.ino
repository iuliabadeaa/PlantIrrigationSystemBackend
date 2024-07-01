#include <Arduino.h>
#include <DHT11.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// Wi-Fi CONNECTION HOTSPOT
 const char* ssid = "Ioolia's iPhone";
 const char* password = "827sjiwiwo";

// Wi-Fi CONNECTION ACASA
// const char* ssid = "DIGI-tS2F";
// const char* password = "AmR5Pz3a64";


// DB CON
#define DATABASE_URL "https://plant-irrigation-system-3700f-default-rtdb.europe-west1.firebasedatabase.app/"
#define API_KEY "AIzaSyAvd9p0Nj025LAdqNjvgdnz6obkq4SZd5U"

// DEF PINI
#define DHTPIN 4
#define DHTTYPE DHT11
#define light 33
#define soil  35
#define water 25

WiFiClient wifiClient;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

DHT11 dht11(DHTPIN);
unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

// init senzori
int air_temperature = 0;
int air_humidity = 0;

int soilValue = 0;
int brightness = 0;

bool relayState;

//salvez db path
String databasePath;


void setup_wifi() {
  delay(100);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
}


void setup() {
  Serial.begin(115200);
  setup_wifi();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if(Firebase.signUp(&config, &auth, "", "")){
    Serial.println("signUp ok");
    signupOK = true;
  }
  else 
  {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  databasePath = "Sensor";

  pinMode(soil, INPUT);
  pinMode(light, INPUT);
  pinMode(water, OUTPUT);
  //relayState=HIGH;
  relayState=true;
  if(relayState==true)
  {
    digitalWrite(water, HIGH);
  }
  else
  {
    digitalWrite(water,LOW);
  }
  dht11.setDelay(1000);


}


void loop() {
  // Read sensor data
  delay(1000);
  int result = dht11.readTemperatureHumidity(air_temperature,  air_humidity);
  delay(1000);
  brightness = analogRead(light);
  int value_brightness=map(brightness,4096,0,0,100);
  

  soilValue = analogRead(soil);
  int value_humidity_soil=map(soilValue,4096,0,0,300);



  //GET RELAY STATE FROM FB
  if (Firebase.ready() && signupOK) {
    if (Firebase.RTDB.getBool(&fbdo, databasePath + "/relay_state")) {
      relayState = fbdo.boolData();
      digitalWrite(water, relayState);
      Serial.print("Relay state read from Firebase: ");
      Serial.println(relayState ? "HIGH" : "LOW");
    } else {
      Serial.print("Failed to get relay state: ");
      Serial.println(fbdo.errorReason());
    }
  }



  // Send data to Firebase
  if(Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();

    // Store sensor data in RTDB
    if(Firebase.RTDB.setInt(&fbdo, databasePath + "/air_temperature", air_temperature))
    {
      Serial.print("Temperature: ");
      Serial.println(air_temperature);
      Serial.println(" - successfully saved");
    } 
    else 
    {
      Serial.println(fbdo.errorReason());
    }

    if(Firebase.RTDB.setInt(&fbdo, databasePath + "/air_humidity", air_humidity))
    {
      Serial.print("Humidity: ");
      Serial.println(air_humidity);
      Serial.println(" - successfully saved");
    } 
    else 
    {
      Serial.println(fbdo.errorReason());
    }

    if(Firebase.RTDB.setInt(&fbdo, databasePath + "/soil_humidity", value_humidity_soil))
    {
      Serial.print("value_humidity_soil: ");
      Serial.println(value_humidity_soil);
      Serial.println(" - successfully saved");
    } 
    else 
    {
      Serial.println(fbdo.errorReason());
    }

    if(Firebase.RTDB.setInt(&fbdo, databasePath + "/value_brightness", value_brightness))
    {
      Serial.print("value_brightness: ");
      Serial.println(value_brightness);
      Serial.println(" - successfully saved");
    } 
    else 
    {
      Serial.println(fbdo.errorReason());
    }
  }
}