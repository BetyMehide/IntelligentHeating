#include <dht.h>
#include <dht.h>
dht DHT;
#define DHT11_PIN 7

int ledPin = 13;  // LED on Pin 13 of Arduino
int pirPin = 8; // Input for HC-S501

int pirValue; // Place to store read PIR Value

void setup(){
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT);
  pinMode(pirPin, INPUT);
 
  digitalWrite(ledPin, LOW);
}
void loop()
{
int chk = DHT.read11(DHT11_PIN);
Serial.print("Temperature = ");
Serial.println(DHT.temperature);
Serial.print("Humidity = ");
Serial.println(DHT.humidity);
pirValue = digitalRead(pirPin);
Serial.print("Movement = ");
Serial.print(pirValue);
digitalWrite(ledPin, pirValue);
delay(2000);
}
