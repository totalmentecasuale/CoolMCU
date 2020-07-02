# CoolMCU
Personal project for the Internet of Things course @ Politecnico di Milano, A.Y 2019/2020

## Project concept

Use a ESP8266 WiFi board to:

* Read the temperature and humidity values in a room
* Send these values to Thingspeak for easy real-time visualization
* Publish these values on a dedicated MQTT topic
* Subscribe to specific MQTT topics for remote control
* Control the AC present in the room via an IR LED and unit-specific IR codes.

## Hardware

* ESP8266 NodeMCU board
* Breadboard
* Jumpers
* DHT11 (or any temperature and humidity sensor)
* 2N2222 Transistor
* IR LED
* microUSB cable

## Software

* Arduino IDE
* The .ino sketch in this repo
* MQTT client

## Build
![](report/pinout.png)

The board was put on a breadboard together with the DHT11 sensor and the IR LED according to their own pinout.
The IR LED was connected to D2 (Pin 4) through a transistor, in order to guarantee an adequate current flow to power it.
The DHT11 sensor was connected to D4 (Pin 2) directly.

## Sketch

### setup()

All the services are initialised:

* Serial port for monitoring and debugging
* DHT sensor, to start reading temperature and humidity
* Wifi, to connect to the Internet
* Thingspeak, to send data and visualise it
* MQTT, to receive and send readings
* AC, to control the air conditioning unit

### loop()
It repeats every ... seconds.
* Temperature and humidity are read and printed to serial port at the beginning of each cycle
* The node checks if the temperature is above 24 and turns the AC on using the dedicated function. There is a manual override boolean in order to allow me to turn it on or off remotely.
* If 2s have passed from the last MQTT messages published, a new one for each value is crafted and published to the channels `/room/temp` and `room/hum` respectively.
* If 30s have passed from the last Thingspeak message (to avoid getting them rejected for being too frequent) another one is sent to [this channel](https://thingspeak.com/channels/1092202).
* After 8hr the manual override is disabled automatically.

### MQTT topics
* Publish (from the mote point of view)
  * `/room/temp`, Temperature values
  * `/room/hum`, Humidity values


* Subscribe
  * `room/AC/toggle`, turning on (1) or off (0) the AC unit
  * `room/AC/set_temp`, setting the temperature (number)
  * `room/AC/set_powerful`, toggling the powerful mode (1 or 0)
  * `room/AC/set_mode`, Heat (`h`), Cool (`c`), Dry (`d`), Auto (`a`) mode

Between parenthesis there are the payloads the node is able to recognise and convert into a IR remote command for the AC unit.

## Dependencies
* Sensor:
  * `<DHT.h>` Temperature and humidity sensor library


* WiFi:
  * `<ESP8266WiFi.h>`
  * `<WiFiClient.h>`


* ThingSpeak:
  * `<ThingSpeak.h>`


* MQTT:
  * `<PubSubClient.h>`


* Remote control:
  * `<Arduino.h>`
  * `<IRremoteESP8266.h>`
  * `<IRsend.h>`
  * `<ir_Panasonic.h>` Model specific library

## Improvements

* Google Assistant integration
