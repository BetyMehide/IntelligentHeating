#include <Adafruit_Sensor.h>
#include "DHT.h"
#include <LiquidCrystal.h>
//#include <ESP8266WiFi.h>


//Define variables
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
//const char *ssid =  "Your wifi Network name";     // replace with your wifi ssid and wpa2 key
//const char *pass =  "Network password";
//WiFiClient client;

//Define the pins in use
#define DHTPIN 15 // pin for the temp and humidity sensor
#define DHTTYPE DHT22 //the humidity sensor type
int pirPin = 16; // Input for HC-S501 (motion sensor)
#define ctsPinUp 4 //touch sensor for up
#define ctsPinDown 0 //touch sensor for down

//Initializations
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor

void setup(){
  Serial.begin(9600);

  //setup for the wifi port
  while (!Serial){
    ;//wait for serial port to connect
  }

  //setup for temp and humidity sensor
  delay(10);
  dht.begin();

  //setup for the touch sensor
  pinMode(ctsPinUp, INPUT);
  pinMode(ctsPinDown, INPUT);

  //setup for motion sensor
  pinMode(pirPin, INPUT);

  //setup for wifi
  //Serial.println("Connecting to ");
  //Serial.println(ssid); 
  //WiFi.begin(ssid); 
  //while (WiFi.status() != WL_CONNECTED)
  //{
  //  delay(500);
  //  Serial.print(".");
  //}
  //Serial.println("");
  //Serial.println("WiFi connected"); 
}

void loop()
{
  curMillis = millis();

  //temperature and humidity sensor handling
  if(curMillis - prevTempMillis >= tempDelay){
    prevTempMillis = curMillis;
    Serial.print("Temperature in F = ");
    Serial.println(dht.readTemperature(true));
    Serial.print("Temperature in C =");
    Serial.println(dht.readTemperature());
    Serial.print("Humidity = ");
    Serial.println(dht.readHumidity());
  }
  
    // touch sensor handling
    int touchValueUp = digitalRead(ctsPinUp);
    if(touchValueUp == HIGH){
      if(prevTouchValueUp != HIGH){
        Serial.println("Up");  
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
      }
      prevTouchValueDown = touchValueDown;
    }
    else{
      prevTouchValueDown = LOW;
    }
      

  //motion sensor handling
  pirValue = digitalRead(pirPin);
 // if (pirValue){
    if(curMillis - prevMotionMillis >= motionDelay){
      prevMotionMillis = curMillis;
      Serial.print("Movement = ");
      Serial.println(pirValue);
    } 
 // }
  //if (!pirValue){
  //  prevMotionMillis = 0;  
  //}
}
