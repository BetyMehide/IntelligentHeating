#include <Adafruit_Sensor.h>
#include "DHT.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <TimeLib.h>

//DEFINE VARIABLES
int pirValue; // Place to store read PIR Value
//variables for controlling information sensing time
unsigned long curMillis; 
long tempDelay = 10000; //delay for displaying temp and humidity data in milliseconds
long prevTempMillis = 0;
long motionDelay = 2000; //delay for displaying motion sensor data
long prevMotionMillis = 0;
int prevTouchValueUp = LOW;
int prevTouchValueDown = LOW;
//variables for wifi connection
const char* ssid =  "****";     // replace with your wifi ssid and wpa2 key
//const char* password =  "****";
int status = WL_IDLE_STATUS;
IPAddress server(****); //server address
const int httpPort = 5000;
HTTPClient http; 
//json variables
StaticJsonBuffer<500> JSONBuffer; //setup memory pool for the JSON object tree
JsonObject& data = JSONBuffer.createObject();
char JSONmessageBuffer[500];
//time variables
time_t t = now();

//DEFINE PINS
#define DHTPIN 15 // pin for the temp and humidity sensor
#define DHTTYPE DHT22 //the humidity sensor type
int pirPin = 16; // Input for HC-S501 (motion sensor)
#define ctsPinUp 4 //touch sensor for up
#define ctsPinDown 0 //touch sensor for down

//INIT
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor
WiFiClient client; //initialize the client library

void setup(){
  Serial.begin(9600);
  delay(10);
  
  //WIFI PORT SETUP
  Serial.println("Attempting to connect to WPA network...");
  Serial.print("SSID: ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  http.setReuse(true);
  Serial.println("");
  Serial.println("WiFi connected");

  //CLIENT-SERVER SETUP
  if (!client.connect(server, httpPort)) {  // if you get a connection, report back via serial:
    Serial.println("connection to server failed");
    delay(5000);
    return; // Make a HTTP request
  }
  Serial.println("connected to server");

  //TEMPERATURE AND HUMIDITY SENSOR SETUP
  dht.begin();

  //setup for the touch sensor
  pinMode(ctsPinUp, INPUT);
  pinMode(ctsPinDown, INPUT);

  //setup for motion sensor
  pinMode(pirPin, INPUT);
}


//MAIN DATA RECORDING
void recData(){
  data["temperature"] = dht.readTemperature(true);
  data["humidity"] = dht.readHumidity();
  data["movement"] = pirValue;
  String timev = String(month(t)) + " " + String(day()) + " " + String(weekday()) + " " + String(hour()) + " " + String(minute()); 
  Serial.println(timev);
  //JsonArray& timestamps = data.createNestedArray("timestamps");
  data["timestamps"] = timev;
  //timestamps.add(day(t));
  //timestamps.add(weekday(t));
  //timestamps.add(hour(t));
  //timestamps.add(minute(t));
}

//POST DATA TO SERVER
void clientHandling(){
  if (WiFi.status() == WL_CONNECTED) {
    //handle the json format
    char JSONmessageBuffer[500];
    data.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);

    //handle the http request
    http.begin("http://40.71.19.247:5000/post/logdata");
    http.addHeader("Content-Type", "application/json");
    int httpCode = http.POST(JSONmessageBuffer);
    String payload = http.getString();
    Serial.println(httpCode);
    Serial.println(payload);
    http.end();
  }
  else{
    Serial.println("WiFi connection error");
  }  
}

void loop()
{
  curMillis = millis();

  //TEMPERATURE AND HUMIDITY SENSOR HANDLING
  if(curMillis - prevTempMillis >= tempDelay){
    prevTempMillis = curMillis;
    Serial.print("Temperature in F = ");
    Serial.println(dht.readTemperature(true));
    Serial.print("Temperature in C =");
    Serial.println(dht.readTemperature());
    Serial.print("Humidity = ");
    Serial.println(dht.readHumidity());
  }

  //MOTION SENSOR HANDLING
  pirValue = digitalRead(pirPin);
  if (pirValue){
    if(curMillis - prevMotionMillis >= motionDelay){
      prevMotionMillis = curMillis;
      Serial.print("Movement = ");
      Serial.println(pirValue);
      //send relevant data to server
      recData();
      clientHandling();
    } 
  }
  if (!pirValue){
    prevMotionMillis = 0;  
  }
  
  //TOUCH SENSOR HANDLING
  int touchValueUp = digitalRead(ctsPinUp);
  if(touchValueUp == HIGH){
    if(prevTouchValueUp != HIGH){
      Serial.println("Up");
      //data["user"] = 1;
      recData();
      clientHandling();  
    }
    prevTouchValueUp = touchValueUp;
  }
  else{
    prevTouchValueUp = LOW;
  }
  int touchValueDown = digitalRead(ctsPinDown);
  if(touchValueDown == HIGH){
    if(prevTouchValueDown != HIGH){
      Serial.println("Down");
      //data["user"] = -1;
      recData();
      clientHandling();  
    }
    prevTouchValueDown = touchValueDown;
  }
  else{
    prevTouchValueDown = LOW;
  }
}    
