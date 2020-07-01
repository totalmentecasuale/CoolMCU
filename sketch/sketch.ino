#include <DHT.h>

// Setup DHT11 pins (D4)
#define DHT11_PIN 2
DHT DHT(DHT11_PIN,DHT11);

void setup()
{
Serial.begin(115200);
DHT.begin();
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
delay(3000);
}
