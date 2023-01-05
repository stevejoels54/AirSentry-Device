#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "DHT.h"

// wifi credentials
const char* ssid = "wifi-name";
const char* password = "wifi-password";


// Constants
#define DHTPIN 4
#define AIRSENSORPIN 35
#define DEVICE_ID 1
#define DHTTYPE DHT11

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number that can't be stored in an int.
unsigned long lastTime = 0;
unsigned long timerDelay = 5000; // Timer set to 5 seconds (5000)

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);  
  dht.begin(); 
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());   
  delay(4000);  //wait 4 seconds after inializing
}

void postDataToServer(float val1, float val2, float val3) { // airquality, temperature, humidity
  if ((millis() - lastTime) > timerDelay) {  //Post data every after 5 seconds 
      Serial.println("Posting JSON data to server...");
      // Block until we are able to connect to the WiFi access point
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;   
        http.begin("https://stevejoels.pythonanywhere.com/readings/"); //server url  
        http.addHeader("Content-Type", "application/json");         
        
        StaticJsonDocument<64> doc;

        doc["air"] = val1;
        doc["temperature"] = val2;
        doc["humidity"] = val3;
        doc["device_id"] = DEVICE_ID;

        char data[128];
        serializeJson(doc, data);

        int httpResponseCode = http.POST(data);
    
        if(httpResponseCode == 201){
          String response = http.getString();                       
          Serial.println(httpResponseCode);   
          Serial.println(response);
        }
        else if(httpResponseCode > 201){
          Serial.println(httpResponseCode);
          Serial.println("Server error");
        } 
        else {
        Serial.println("Network error");
        }
      }
       else {
      Serial.println("WiFi Disconnected");
      }
      lastTime = millis();
      }
}

void loop() { 
  // Read humidity
  float humidity = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float temperature = dht.readTemperature();
  temperature = temperature-5;
  // Read air quality
  float air_quality = map(analogRead(AIRSENSORPIN), 0, 4095, 0, 500);

  if (isnan(humidity) || isnan(temperature) || isnan(air_quality))
  {
    // check if readings are not empty
    Serial.println(F("Sensor Error, check sensors!"));
  }
  else {
    postDataToServer(air_quality, temperature, humidity);
    
  }
}