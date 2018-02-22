#include <Adafruit_Sensor.h>
#include "DHT.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
//#include <TimeLib.h>

//DEFINE VARIABLES
int pirValue1; // Place to store the PIR (movement) sensor Value
int pirValue2;
int pirValue3;
int moveInLastPeriod; // Variable to remember whether there's been any movement in the last 10 minutes
int prevButtonUpState = LOW;
int prevButtonDownState = LOW;
//variables for controlling information sensing time
unsigned long curMillis; 
long tempDelay = 100000; //delay for sending temp and humidity data in milliseconds
long prevTempMillis = 0;
int pressDelay;
long buttonPressDelay = 1200; //delay for reading the button presses
long curPressStart = 5000000;
int currentlyPressed = 0;
//variables for wifi connection
const char* ssid =  "*****";     // replace with your wifi ssid and wpa2 key
//const char* password =  "****";
int status = WL_IDLE_STATUS;
IPAddress server(*****); //server address
const int httpPort = 5001;
HTTPClient http; 
//json variables
StaticJsonBuffer<500> JSONBuffer; //setup memory pool for the JSON object tree
JsonObject& data = JSONBuffer.createObject();
char JSONmessageBuffer[500];

//DEFINE PINS
#define DHTPIN 16 // pin for the temp and humidity sensor
#define DHTTYPE DHT22 //the humidity sensor type
int pirPin1 = 12; // Input for HC-S501 (motion sensor)
int pirPin2 = 13;
int pirPin3 = 15;
int buttonUpPin = 4; //touch sensor for up
int buttonDownPin = 2; //touch sensor for down

//INIT
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor
WiFiClient client; //initialize the client library

void setup(){
  Serial.begin(9600);
  delay(10);
  pinMode(LED_BUILTIN, OUTPUT);
  
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

  //BUTTONS SETUP
  pinMode(buttonUpPin, INPUT);
  pinMode(buttonDownPin, INPUT);

  //MOTION SENSORS SETUP
  pinMode(pirPin1, INPUT);
  pinMode(pirPin2, INPUT);
  pinMode(pirPin3, INPUT);
}


//MAIN DATA RECORDING
void recData(){
  data["temperature"] = dht.readTemperature(true);
  data["humidity"] = dht.readHumidity();
  data["movement"] = moveInLastPeriod;
    Serial.println("Data sending in process....");
    Serial.print("Temperature in F = ");
    Serial.println(dht.readTemperature(true));
    Serial.print("Temperature in C =");
    Serial.println(dht.readTemperature());
    Serial.print("Humidity = ");
    Serial.println(dht.readHumidity());
    Serial.print("Move in the last time period = ");
    Serial.println(moveInLastPeriod);
  
  //String timev = String(month(t)) + " " + String(day(t)) + " " + String(weekday(t)) + " " + String(hour(t)) + " " + String(minute(t)); 
  //Serial.println(timev);
  //JsonArray& timestamps = data.createNestedArray("timestamps");
  //data["timestamps"] = timev;
}

//POST DATA TO SERVER
void clientHandling(){
  if (WiFi.status() == WL_CONNECTED) {
    //handle the json format
    char JSONmessageBuffer[500];
    data.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);

    //handle the http request
    http.begin("http://40.71.19.247:5001/post/logdata");
    http.addHeader("Content-Type", "application/json");
    Serial.println("Data sending in process....");
    int httpCode = http.POST(JSONmessageBuffer);
    String payload = http.getString();
    Serial.print("Server response = ");
    Serial.println(httpCode);
    if (httpCode != 200){
      Serial.println("failed to send data");
      digitalWrite(LED_BUILTIN, LOW);      
    }
    else{
      digitalWrite(LED_BUILTIN, HIGH);
    }
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

  //DATA SENDING EVERY 10 MINUTES
  if(curMillis - prevTempMillis >= tempDelay){
    prevTempMillis = curMillis;
    recData();
    clientHandling();
    moveInLastPeriod = 0;
  }

  //MOTION SENSOR HANDLING
  pirValue1 = digitalRead(pirPin1);
  pirValue2 = digitalRead(pirPin2);
  pirValue3 = digitalRead(pirPin3);
  if (pirValue1 || pirValue2 || pirValue3){
    moveInLastPeriod = 1;
    Serial.print(pirValue1);
    Serial.print(pirValue2);
    Serial.println(pirValue3);
  }
  
  //BUTTONS HANDLING
  int buttonUpState = digitalRead(buttonUpPin);
  int buttonDownState = digitalRead(buttonDownPin);
  //control for button pressing time
  if ((buttonUpState == HIGH || buttonDownState == HIGH) && currentlyPressed == 0){
    curPressStart = curMillis;
    currentlyPressed = 1;
  }
  if (curMillis - curPressStart >= buttonPressDelay && currentlyPressed == 1){
    pressDelay = 1;
  }
  //if both buttons pressed
  if(buttonUpState == HIGH && buttonDownState == HIGH && (prevButtonUpState == LOW || prevButtonDownState == LOW)){
    Serial.println("Comfortable");
    prevButtonUpState = HIGH;
    prevButtonDownState = HIGH;
    pressDelay = 0;
  }
  //if too cold button pressed
  else if(buttonUpState == HIGH && prevButtonUpState == LOW && pressDelay == 1){
    Serial.println("Up botton pressed");
    prevButtonUpState = buttonUpState;
    pressDelay = 0;
  }
  //if too warm button pressed
  else if(buttonDownState == HIGH && prevButtonDownState == LOW && pressDelay == 1){
    Serial.println("Down button pressed");
    prevButtonDownState = buttonDownState;
    pressDelay = 0;
  }

  if(prevButtonDownState == HIGH && buttonDownState == LOW){
    prevButtonDownState = LOW;
    currentlyPressed = 0;
  }
  if(prevButtonUpState == HIGH && buttonUpState == LOW){
    prevButtonUpState = LOW;
    currentlyPressed = 0;
  }
}    
