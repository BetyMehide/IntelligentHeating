#include <dht.h>
#include <Keypad.h>
dht DHT;
#define DHT11_PIN 7

int ledPin = 13;  // LED on Pin 13 of Arduino
int pirPin = 8; // Input for HC-S501
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
int pirValue; // Place to store read PIR Value
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}

};
byte rowPins[ROWS] = {5, 4, 3, 2}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {9, 8, 7, 6}; //connect to the column pinouts of the keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

void setup(){
  Serial.begin(9600);
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);
 
  digitalWrite(ledPin, LOW);
}
void loop()
{
int chk = DHT.read11(DHT11_PIN);
//Serial.print("Temperature = ");
//Serial.println(DHT.temperature);
//Serial.print("Humidity = ");
//Serial.println(DHT.humidity);
pirValue = digitalRead(pirPin);
//Serial.print("Movement = ");
//Serial.print(pirValue);
digitalWrite(ledPin, pirValue);
delay(1000);
char key = keypad.getKey();
  if (key){
    Serial.println("Key pressed = ");
    Serial.println(key);
  }
}




