#include <DHT.h> // Temperature and humidity sensor library

#include <ESP8266WiFi.h>;
 
#include <WiFiClient.h>; 
 
#include <ThingSpeak.h>; // self-explanatory

#include <PubSubClient.h> // MQTT
 
const char* ssid = "wifimcu"; //Your Network SSID
 
const char* password = "zoccazocca"; //Your Network Password

const char* mqtt_server = "test.mosquitto.org";

// Setup DHT11 pins (D4)
#define DHT11_PIN 2
DHT DHT(DHT11_PIN,DHT11);
//-----------------------

// Thingspeak channel data
// ID 1092202
// Write key WRVX1WEF8WHP5M1W
 
WiFiClient espclient;

// MQTT setup
PubSubClient client(espclient);
long lastMsg = 0;
long lastTsMsg = 0;
char msg[100];
int value = 0;

// Thingspeak channel setup
unsigned long myChannelNumber = 1092202; //Your Channel Number (Without Brackets)
 
const char * myWriteAPIKey = "WRVX1WEF8WHP5M1W"; //Your Write API Key

// Called when a message is received, currently testing
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

// Reconnecting to MQTT broker
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopicTestFabio", "hello world");
      // ... and resubscribe
      client.subscribe("fromFabioTesting");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
pinMode(BUILTIN_LED, OUTPUT);
Serial.begin(115200);

// Starting the sensor
DHT.begin();

// Connecting to WiFi
WiFi.begin(ssid, password);
// Connecting to Thingspeak
ThingSpeak.begin(espclient);
// Connecting to broker
client.setServer(mqtt_server, 1883);
client.setCallback(callback);
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

  // Send MQTT Message
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, 100, "T: %.2f, H: %.2f", tempC, humidity);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("outTopicTestFabio", msg);
  }
  if(now - lastTsMsg > 30000){
    lastTsMsg = now;
    ThingSpeak.writeField(myChannelNumber, 1,tempC, myWriteAPIKey); //Update temperature in ThingSpeak
  }
  
  delay(3000);
}
