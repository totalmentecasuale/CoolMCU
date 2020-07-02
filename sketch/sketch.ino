#include <DHT.h> // Temperature and humidity sensor library

#include <ESP8266WiFi.h>;

#include <WiFiClient.h>;

#include <ThingSpeak.h>; // self-explanatory

#include <PubSubClient.h> // MQTT

// IR Remote libraries
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Panasonic.h>

// Network variables
const char* ssid = "foofoo"; //Your Network SSID
const char* password = "barbar"; //Your Network Password

const char* mqtt_server = "test.mosquitto.org";

// Setup DHT11 pins (D4)
#define DHT11_PIN 2
DHT DHT(DHT11_PIN,DHT11);

// IR Remote LED setup
const uint16_t kIrLed = 4;  // ESP8266 GPIO pin to use. Recommended: 4 (D2).
IRPanasonicAc ac(kIrLed);  // Set the GPIO used for sending messages.

// MQTT setup
WiFiClient espclient;
PubSubClient client(espclient);

long lastMsg = 0;
long lastTsMsg = 0;
char msg_t[50];
char msg_h[50];
char ACmsg[100];

// Thingspeak channel setup
unsigned long myChannelNumber = 1092202; //Your Channel Number (Without Brackets)

const char * myWriteAPIKey = "foobar"; //Your Write API Key

bool manual_or = false;
bool state_changed = false;

// Functions-------------------------------------------

// Turn on and off the AC
void OnAC(){
  ac.on();
  ac.send();
  }

void OffAC(){
  ac.off();
  ac.send();
  }

void resetAC(){
  Serial.println("Setting default state for A/C.");
  ac.on();
  ac.setFan(kPanasonicAcFanAuto);
  ac.setMode(kPanasonicAcDry);
  ac.setTemp(23);
  ac.setSwingVertical(kPanasonicAcSwingVAuto);
  ac.setSwingHorizontal(kPanasonicAcSwingHAuto);
  ac.setPowerful(false);
  }

// Called when a message is received, currently testing
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if((strcmp(topic, "room/AC/toggle")==0)){
    manual_or = true;
    if ((char)payload[0] == '1'){
       OnAC();
       state_changed = true;
      } else if ((char)payload[0] == '0'){
       OffAC();
       state_changed = true;
      }
    }

  if((strcmp(topic, "room/AC/set_temp")==0)){
    payload[length] = '\0';
    String s = String((char*)payload);
    uint8_t temperature = s.toInt();
    temperature = constrain(temperature, kPanasonicAcMinTemp, kPanasonicAcMaxTemp);
    Serial.print("Setting temp\n");
    ac.setTemp(temperature);
    state_changed = true;
    }

  if((strcmp(topic, "room/AC/set_powerful")==0)){
    if ((char)payload[0] == '1'){
       ac.setPowerful(true);
       state_changed = true;
      } else if ((char)payload[0] == '0'){
       ac.setPowerful(false);
       state_changed = true;
      }
    }

    if((strcmp(topic, "room/AC/set_mode")==0)){
      switch((char)payload[0]){
        case 'a':
          ac.setMode(kPanasonicAcAuto);
          break;
        case 'c':
          ac.setMode(kPanasonicAcCool);
          break;
        case 'h':
          ac.setMode(kPanasonicAcHeat);
          break;
        case 'd':
          ac.setMode(kPanasonicAcDry);
          break;
        }
        state_changed = true;
      }
    if((strcmp(topic, "room/AC/reset_AC")==0)){
        resetAC();
        state_changed = true;
      }
    if((strcmp(topic, "room/AC/manual_or")==0)){
        if ((char)payload[0] == '1'){
          manual_or = true;
        } else if ((char)payload[0] == '0'){
          manual_or = false;
        }
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
      client.publish("announcements", "Connected, let's chill");
      // ... and resubscribe
      client.subscribe("room/AC/toggle");
      client.subscribe("room/AC/set_temp");
      client.subscribe("room/AC/set_powerful");
      client.subscribe("room/AC/set_mode");
      client.subscribe("room/AC/reset_AC");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Printing AC status
void printState() {
  Serial.println("Panasonic A/C remote is in the following state:");
  Serial.printf("  %s\n", ac.toString().c_str());
  snprintf(ACmsg, 100, "AC State: %s\n", ac.toString().c_str());
  Serial.println();
}

//-----------------------------------------------
void setup()
{
pinMode(BUILTIN_LED, OUTPUT);
Serial.begin(115200);

// Init sensor
DHT.begin();

// Init WiFi
WiFi.begin(ssid, password);

//Init Thingspeak
ThingSpeak.begin(espclient);

// Init MQTT
client.setServer(mqtt_server, 1883);
client.setCallback(callback);

delay(269);
// Init AC Remote control
ac.begin();
// AC default state
  Serial.println("Default state of the remote.");
  printState();
  Serial.println("Setting desired state for A/C.");
  ac.setModel(kPanasonicRkr);
  ac.on();
  ac.setFan(kPanasonicAcFanAuto);
  ac.setMode(kPanasonicAcDry);
  ac.setTemp(23);
  ac.setSwingVertical(kPanasonicAcSwingVAuto);
  ac.setSwingHorizontal(kPanasonicAcSwingHAuto);
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

  if(tempC >= 24 && manual_or == false){
    OnAC();
    }
  // Send MQTT Message
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

// Sending a command only if a change was added
if(state_changed){
    #if SEND_PANASONIC_AC
    Serial.println("Sending IR command to A/C ...");
    ac.send();
    #endif  // SEND_PANASONIC_AC

    state_changed = false;
  }
  printState();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    snprintf (msg_t, 50, "%.2f", tempC);
    snprintf (msg_h, 50, "%.2f", humidity);
    Serial.print("Publish message, Temp: ");
    Serial.println(msg_t);
    Serial.print("Publish message, Hum: ");
    Serial.println(msg_h);
    client.publish("room/temp", msg_t);
    client.publish("room/hum", msg_h);
    // Publish current remote status
    client.publish("room/AC/status", ACmsg);
  }
  if(now - lastTsMsg > 30000){
    lastTsMsg = now;
    ThingSpeak.writeField(myChannelNumber, 1,tempC, myWriteAPIKey); //Update temperature in ThingSpeak
  }

  delay(5000);
}
