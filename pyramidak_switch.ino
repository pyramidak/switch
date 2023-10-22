#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "Memory.h"
#include "cWiFi.h"
#include "Relays.h"
#include "DigitalPotentiometer.h"
#include "TrafficLights.h"
#include "StepperMotor.h"
#include "Thermistors.h"
#include "AmpereMeter.h"
#include "Soundmeter.h"
#include "AnalogPercent.h"

WiFiServer server(80);
Memory mem;
cWiFi wi_fi(&mem);
Relays relay; 
DigitalPotentiometer potentiometer;
TrafficLights semafor;
StepperMotor motor;
Thermistors termistor;
AmpereMeter ampere;
Soundmeter sound(&potentiometer);
AnalogPercent analog;

//Original idea of pyramidak firmware and its program was created by Zdeněk Jantač (pyramidak).
//GITHUB: https://github.com/pyramidak/switch
/////////////////////////////////////////////////////////////////////////////////
/// What is new
/// 3.0.2 AC712 update
/// 3.0.1 float temperature correction
/// 3.0.0 saving setting improvement
///////////////////////////////
////    Settings block    /////
///////////////////////////////
String deviceName      = "switch-box";
String firmwareVersion = "3.0.2";
  // Update settings //
String OTA_password    = "";
String update_server   = "192.168.0.100";
  // MQTT settings //
String mqtt_server     = "192.168.0.181";
String mqtt_user       = "";
String mqtt_password   = "";
  // DEVICE settings
int device        = 1; //1 = switch, 2 = two switches, 3 = light, 4 = two lights, 5 = digital potentiometer, 6 = traffic lights, 7 = sensors only, 8 = six switches, 9 = motor
int sensorAnalog  = 0; //0-none, switch 1-NO ON, 2-NO ON/OFF, 3-NO TOGGLE, 4-NC ON, 5-NC ON/OFF, 6-NC TOGGLE, 7-potentiometer, 8-lightmeter, 9-moisture, 10-soundmeter KY-037, 11-current ACS712-5A
int analogSW      = 1; //0-none, 1-switch1, 2-switch2, 3-both (what switch analog sensor controls)
int sensorDigital1= 0; //0-none, switch 1-NO ON, 2-NO ON/OFF, 3-NO TOGGLE, 4-NC ON, 5-NC ON/OFF, 6-NC switch TOGGLE, thermometer 7-DHT11, 8-DHT22, 9-DS18B20
int sensorDigital2= 0; //0-none, switch 1-NO ON, 2-NO ON/OFF, 3-NO TOGGLE, 4-NC ON, 5-NC ON/OFF, 6-NC switch TOGGLE
int potentio;      //analog sensor state
bool digital1;     //digital sensor state
bool digital2;     //second digital sensor state
int sun;           //0 = off //1 = above horizon //2 = below horizon 
int threshold1= 50;//light threshold 
int sleepMode;     //0-OFF, 1-ON when digital sensor ON, 2-ON when digital sensor OFF
int updateStart;   //web server update start: 0 - manual only, 1 - mqtt command
bool mqtt_reconnectNeeded; 
String newWIFIpass, newMQTTpass, newOTApass;
////////////////////////////////
////    PINs declaration    ////
////////////////////////////////
const int pinD1 = D1;   //D1=05
const int pinD2 = D2;   //D2=04
#define pinA  A0 //analog sensor
#define pinAP D0 //WiFi AP switch
////////////////////////////////
//// Report to serial port  ////
////////////////////////////////
void report(String msg, bool offset = false) {
  if(offset) {Serial.println("");}
  Serial.println(msg);
}
///////////////////////////////
////      Setup block     /////
///////////////////////////////
void setup() {
  Serial.begin(115200); //9600, 115200
  delay(200);
  eeprom_begin();
  pinMode(LED_BUILTIN, OUTPUT); // onboard Led light
  pinMode(pinAP, INPUT);  // Wifi AP
  pinMode(pinA, INPUT);   // Analog Sensor  
  if (device != 8) {
    pinMode(pinD1, INPUT);  // Digital Sensor 1
    pinMode(pinD2, INPUT);  // Digital Sensor 2
  }
  potentio = analogRead(pinA) / 4.01;
  if (sensorAnalog == 7) {relay.light.brightness = potentio;}

  if ((device >= 1 and device <= 4) or device == 8) {
    relay.device = device;
    relay.motionSensor = IsMotionSensor();
    relay.begin();
  }

  if (device == 5) {potentiometer.begin();}
  if (device == 9) {motor.begin();}

  termistor.begin(sensorDigital1);
  ampere.begin(sensorAnalog);
  analog.begin(sensorAnalog);

  if (mem.read(3) == 0 or wi_fi.ssid == "" or wi_fi.password == "" or (wi_fi.switchAP == 1 and digitalRead(pinAP) == HIGH)) {
    wi_fi.beginAP(); 
  } else {
    wi_fi.begin(deviceName);
  }
  update_Arduino_begin();
  mqtt_begin();
}
///////////////////////////////
////       Loop block     /////
///////////////////////////////
void loop() {
     
  if (wi_fi.connected() == true) {
    if (wi_fi.connectedJobs() == true) server.begin();
    if (wi_fi.APmode == false) {
      mqtt_loop();
      update_Arduino_handle(); // check OTA update
    }      
    web_server(); // web server handle
  }
  if (!motor.running) delay(10); //MUST delay to allow ESP8266 WIFI functions to run
  
  //timing switch1 and switch2 auto-off + timing movement sensor off + lightEffects
  if (relay.loop() == true) {mqtt_switch();}

  //AnalogPercent Light, Moisture sensor
  analog.loop();
  if (analog.changed) mqtt_analog(String(analog.percent));
  
  //connected analog
  if (sensorAnalog >= 1 and sensorAnalog <= 8) {sensor_analog();}
  if (sensorAnalog == 10) {
    int level = sound.measure();
    if (level != -1) {mqtt_analog(String(level));}
    if (potentiometer.changed) {mqtt_resistance(potentiometer.resistance());}    
  }
  //Current sensor
  ampere.loop();
  if (ampere.changed) {
    mqtt_analog(String(ampere.current)); 
    if (ampere.threshold != 0 and ampere.current > ampere.threshold / 100.0) relay.toggle(analogSW, "ON", sun);
  }
  //digital1 sensor
  if (sensorDigital1 >= 1 and sensorDigital1 <= 6 and device != 8) {
    sensor_digital(pinD1, sensorDigital1, digital1);
    if (wi_fi.switchAP == 1 and digitalRead(pinAP) == LOW) {
      if (mqtt_connected() == true) { //experimental: sleepMode from main loop instead of mqtt
        if ((digital1 == true and sleepMode == 1) or (digital1 == false and sleepMode == 2)) {
          mqtt_analog("");
          mqtt_digital1("");
          mqtt_digital2("");
          delay(1000);
          mqtt_disconnect();
          light_sleep();
        }    
      }
    } 
  }
  //termistors
  if (termistor.connected and device != 8) {
    if (termistor.getTemperature() == true) mqtt_digital1(String(termistor.temperature));
    if (termistor.getHumidity() == true) mqtt_digital2(String(termistor.humidity));
  }
  
  //digital2 sensor
  if (sensorDigital2 >= 1 and sensorDigital2 <= 6 and device != 8) {
    sensor_digital(pinD2, sensorDigital2, digital2);
  }

  //4-wire motor
  if (device == 9) {motor.run();}
}