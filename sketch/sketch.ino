#include <DHT.h>

#include <ESP8266WiFi.h>;
 
#include <WiFiClient.h>;
 
#include <ThingSpeak.h>;
 
const char* ssid = "wifimcu"; //Your Network SSID
 
const char* password = "zoccazocca"; //Your Network Password

// Setup DHT11 pins (D4)
#define DHT11_PIN 2
DHT DHT(DHT11_PIN,DHT11);
//-----------------------

// Thingspeak channel data
// ID 1092202
// Write key WRVX1WEF8WHP5M1W
 
WiFiClient client;
 
unsigned long myChannelNumber = 1092202; //Your Channel Number (Without Brackets)
 
const char * myWriteAPIKey = "WRVX1WEF8WHP5M1W"; //Your Write API Key

void setup()
{
Serial.begin(115200);

// Starting the sensor
DHT.begin();

// Connecting to WiFi
WiFi.begin(ssid, password);
// Connecting to Thingspeak
ThingSpeak.begin(client);
}
void loop()
{

  // Reading Temperature and Humidity
float tempC = DHT.readTemperature();
float humidity = DHT.readHumidity();

Serial.print("Temperature (C) = ");
Serial.println(tempC);
Serial.print("Humidity = ");
Serial.println(humidity);
ThingSpeak.writeField(myChannelNumber, 1,tempC, myWriteAPIKey); //Update temperature in ThingSpeak
delay(30000);
}
