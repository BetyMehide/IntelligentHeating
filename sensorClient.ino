#include <Adafruit_Sensor.h>
#include "DHT.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
//#include <TimeLib.h>


//DEFINE VARIABLES
//Human presence sensor variables (currently PIR sensor)
int pirValue1;
int pirValue2;
int pirValue3;

//temp&humidity variables
#define DHTTYPE DHT22 //the humidity sensor type

//Button state variables
int prevButtonUpState = LOW;
int prevButtonDownState = LOW;
int ledUnchanged = 0; //memory for changing led back to default color

//Time control variables (for getting measurements at the correct times)
unsigned long curMillis; //measurement for time since the code started running
long measureDelay = 60000; //delay for sending data to server in milliseconds
long prevMeasureMillis = 0; //remember the time for the last data send
int moveInLastPeriod; //remember whether there's been any movement in the last measureDelay period
long buttonPressDelay = 1300; //delay before the button press is recognized
int validPress; //remember the recognition of a valid button press
long curPressStart = 0; //record the start time of the button press
int currentlyPressed = 0; //remember whether button is still pressed or not
long ledResponseMillis = 0; //the time when the led response started
long ledResponseDelay = 30000; //response display time

//Variables for wifi connection
const char* ssid =  "****";     // replace with your wifi ssid and wpa2 key
//const char* password =  "****";
int status = WL_IDLE_STATUS;
IPAddress server(****); //server address
const int httpPort = 5001;
HTTPClient http; 
//json variables
StaticJsonBuffer<500> JSONBuffer; //setup memory pool for the JSON object tree
JsonObject& data = JSONBuffer.createObject();
char JSONmessageBuffer[500];


//DEFINE PINS
#define DHTPIN D5 // pin for the temp and humidity sensor
// pins for the motion sensor(s) (Currently HC-S501 PIR sensor)
int pirPin1 = D6; 
int pirPin2 = D7;
int pirPin3 = D2;
//Button sesnors
int buttonUpPin = D1; 
int buttonDownPin = D0;
//LED sensors
int ledPinR = D8;
int ledPinG = D4;
int ledPinB = D3;

 
//INITIALIZATIONS
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor
WiFiClient client; //initialize the client library



void setup(){
  //GENERAL
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

  //BUTTONS SETUP
  pinMode(buttonUpPin, INPUT);
  pinMode(buttonDownPin, INPUT);

  //MOTION SENSORS SETUP
  pinMode(pirPin1, INPUT);
  pinMode(pirPin2, INPUT);
  pinMode(pirPin3, INPUT);

  //LED SETUP
  pinMode(ledPinR, OUTPUT);
  pinMode(ledPinG, OUTPUT);
  pinMode(ledPinB, OUTPUT);
  setColor(255, 255, 255); 
}



//DATA RECORDING
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
      //setColor(80, 80, 80);      
    }
    Serial.println(payload);
    http.end();
  }
  else{
    Serial.println("WiFi connection error");
  }  
} 



//LED HANDLING FUNCTION
void setColor(int red, int green, int blue){
  #ifdef COMMON_ANODE
  red = 255 - red;
  green = 255 - green;
  blue = 255 - blue;
  #endif
  analogWrite(ledPinR, red);
  analogWrite(ledPinG, green);
  analogWrite(ledPinB, blue);
  Serial.print("RGB = ");
  Serial.print(red);
  Serial.print(green);
  Serial.println(blue);
}



//MAIN LOOP FUNCTION
void loop()
{
  curMillis = millis();

  //DATA SENDING EVERY 10 MINUTES
  if(curMillis - prevMeasureMillis >= measureDelay){
    prevMeasureMillis = curMillis;
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
    validPress = 1;
  }
  //if both buttons pressed
  if(buttonUpState == HIGH && buttonDownState == HIGH && (prevButtonUpState == LOW || prevButtonDownState == LOW)){
    Serial.println("Comfortable");
    prevButtonUpState = HIGH;
    prevButtonDownState = HIGH;
    validPress = 0;
    setColor(0, 255, 0); //green
    ledResponseMillis = curMillis;
    ledUnchanged = 1;
  }
  //if too cold button pressed
  else if(buttonUpState == HIGH && prevButtonUpState == LOW && validPress){
    Serial.println("Up botton pressed");
    prevButtonUpState = HIGH;
    validPress = 0;
    setColor(0, 0, 255); //blue
    ledResponseMillis = curMillis;
    ledUnchanged = 1;
  }
  //if too warm button pressed
  else if(buttonDownState == HIGH && prevButtonDownState == LOW && validPress){
    Serial.println("Down button pressed");
    prevButtonDownState = HIGH;
    validPress = 0;
    setColor(255, 0, 0); //red
    ledResponseMillis = curMillis;
    ledUnchanged = 1;
  }
  //when done pressing the button change the previous state value to unpressed (LOW)
  if(prevButtonDownState == HIGH && buttonDownState == LOW){
    prevButtonDownState = LOW;
    currentlyPressed = 0;
  }
  if(prevButtonUpState == HIGH && buttonUpState == LOW){
    prevButtonUpState = LOW;
    currentlyPressed = 0;
  }
  //change the LED color back after it has been different for long enough
  if(curMillis - ledResponseMillis >= ledResponseDelay && ledUnchanged){
    setColor(255, 255, 255);
    ledUnchanged = 0;
  }
}
