#include <dht.h>
#include <LiquidCrystal.h>
#include <Keypad.h>
dht DHT;

//Define variables
int pirValue; // Place to store read PIR Value
unsigned long curMillis;
long tempDelay = 10000;
long prevTempMillis = 0;
long motionDelay = 2000;
long prevMotionMillis = 0;
long prevScreenMillis = 0;
const byte ROWS = 4; 
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

//Define the pins in use
#define DHT11_PIN A0 // pin for the temp and humidity sensor
int pirPin = A1; // Input for HC-S501 (motion sensor)
byte rowPins[ROWS] = {7, 6, 5, 4}; //row pinouts of the keypad
byte colPins[COLS] = {8, 9, 10, 11}; //column pinouts of the keypad
LiquidCrystal lcd(13, 12, 3, 2, 1, 0); //screen pins

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup(){
  //setup for temp and humidity sensor
  Serial.begin(9600);

  //setup for motion sensor
  pinMode(pirPin, INPUT);

  //setup screen
  lcd.begin(16,2); //16 by 2 character display
}

void loop()
{
  curMillis = millis();

  //temperature and humidity sensor handling
  if(curMillis - prevTempMillis >= tempDelay){
    prevTempMillis = curMillis;
    int chk = DHT.read11(DHT11_PIN);
    Serial.print("Temperature = ");
    Serial.println(DHT.temperature);
    Serial.print("Humidity = ");
    Serial.println(DHT.humidity);
  }

  //motion sensor handling
  pirValue = digitalRead(pirPin);
  if (pirValue){
    if(curMillis - prevMotionMillis >= motionDelay){
      prevMotionMillis = curMillis;
      Serial.print("Movement = ");
      Serial.println(pirValue);
    } 
  }
  if (!pirValue){
    prevMotionMillis = 0;  
  }

  //keyboard handling
  char key = keypad.getKey();
  if (key){
    Serial.print("Key pressed = ");
    Serial.println(key);
  }

  //screen handling
  if(curMillis - prevScreenMillis >= tempDelay){
    prevScreenMillis = curMillis;
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Humidity = ");
    lcd.print(DHT.temperature);
    lcd.setCursor(0,1);
    lcd.print("Temp = ");
    lcd.print(DHT.humidity);
  }
}
