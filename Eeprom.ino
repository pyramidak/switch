//Memory:
//  0: memAlok
//  1: device
//  2: sleep mode
//  3: wifi ready
//  4: WiFi AP switch
//  5: analogSW
//  6: sensorAnalog
//  7: sensorDigital
//  8: sensorDigital2
//  9: startS1
// 10: startS2
// 11: stayoffS1
// 12: stayoffS2
// 13: milis pulseTime
// 14: seconds pulseTime
// 15: minutes pulseTime
// 16: hours pulseTime
// 17: milis pulseTime2
// 18: seconds pulseTime2
// 19: minutes pulseTime2
// 20: hours pulseTime2
// 21: brightness light
// 22: brightness leds
// 23: current calibration high
// 24: light effect
// 25: led mode
// 26: threshold1 light 
// 27: threshold2 sound
// 28: current calibration low
// 29: resistance
// 30: motor speed
// 31-35: wifi AP name
// 36-50: wifi name
// 51-65: wifi password
// 66-90: mqtt server
// 91-105: mqtt user
//106-120: mqtt password
//121-135: ota password
//136-150: device name
//151-165: server address
//166: temperature correction
//167: humidity correction
//168: remote update off
//169:
//170-175: web password
//176: min (moister dry, light night)
//177: max (moister wet, light day)
//178: current threshold low
//179: current threshold high
//180: 

void eeprom_begin() { 
  mem.begin();
  device = mem.readAndCheck(device, 1, "devicePurpose", 1, 9);
  sleepMode = mem.readAndCheck(sleepMode, 2, "sleepMode", 0, 2);
  report("wifi: " + String(mem.read(3)) );
  wi_fi.switchAP = mem.readAndCheck(wi_fi.switchAP, 4, "wifiSwitch", 0, 2);
  updateStart = mem.readAndCheck(1, 168, "updateRemote", 0, 1);
  analogSW = mem.readAndCheck(analogSW, 5, "analogSwitch", 0, 3);
  sensorAnalog = mem.readAndCheck(sensorAnalog, 6, "sensorAnalog", 0, 11);
  sensorDigital1 = mem.readAndCheck(sensorDigital1, 7, "sensorDigital", 0, 9);
  sensorDigital2 = mem.readAndCheck(sensorDigital2, 8, "sensorDigital2", 0, 6);
  relay.startS1 = mem.readAndCheck(relay.startS1, 9, "startState1", 0, 1);
  relay.startS2 = mem.readAndCheck(relay.startS2, 10, "startState2", 0, 1);
  relay.stayoffS1 = mem.readAndCheck(relay.stayoffS1, 11, "stayOff1", 0, 3);
  relay.stayoffS2 = mem.readAndCheck(relay.stayoffS2, 12, "stayOff2", 0, 3);
  relay.timePulse1 = createPulseTime(mem.read(13), mem.read(14), mem.read(15), mem.read(16));
  relay.timePulse2 = createPulseTime(mem.read(17), mem.read(18), mem.read(19), mem.read(20));
  relay.light.brightness = mem.readAndCheck(relay.light.brightness, 21, "brightnessLight", 0, 254);
  relay.diodBright = mem.readAndCheck(relay.diodBright, 22, "brightnessDiod", 0, 177);
  relay.light.effect = mem.readAndCheck(relay.light.effect, 24, "lightEffect", 0, 4);
  relay.diodLight = mem.readAndCheck(relay.diodLight, 25, "diodMode", 0, 3);
  threshold1 = mem.readAndCheck(25, 26, "threshold1", 1, 99);
  sound.threshold = mem.readAndCheck(sound.threshold, 27, "thresholdSound", 0, 1024);
  ampere.zeroPoint = mem.readAndCheck(28, 23, "ampereZeroPoint");
  ampere.threshold = mem.readAndCheck(178, 179, "ampereSwitchThreshold");
  potentiometer.resistance(mem.readAndCheck(potentiometer.resistance(), 29, "resistance", 0, 99));
  wi_fi.nameAP = mem.readAndCheck("", "wifiAPname", 31, 35, false);
  wi_fi.ssid = mem.readAndCheck(wi_fi.ssid, "wifiSSID", 36, 50, false);
  wi_fi.password = mem.readAndCheck(wi_fi.password, "wifiPass", 51, 65, true);
  if (wi_fi.password != "") newWIFIpass = "*";
  mqtt_server = mem.readAndCheck(mqtt_server, "MQTTserver", 66, 90, false);
  mqtt_user = mem.readAndCheck(mqtt_user, "MQTTuser", 91, 105, false);
  mqtt_password = mem.readAndCheck(mqtt_password, "MQTTpass", 106, 120, true);
  if (mqtt_password != "") newMQTTpass = "*";
  OTA_password = mem.readAndCheck(OTA_password, "OTApass", 121, 135, true);
  if (OTA_password != "") newOTApass = "*";
  deviceName = mem.readAndCheck(deviceName, "deviceName", 136, 150, false);
  update_server = mem.readAndCheck(update_server, "updateServer", 151, 165, false);
  termistor.fixTemperature = mem.readAndCheck(0, 166, "fixTemperature", 0, 99) / 10.0;
  termistor.fixHumidity = mem.readAndCheck(termistor.fixHumidity, 167, "fixHumidity", 0, 25);
  motor.speed(mem.readAndCheck(motor.speed(), 30, "motorSpeed", 4, 1024));
  analog.min = mem.readAndCheck(analog.min, 176, "minAnalog", 0, 1024);
  analog.max = mem.readAndCheck(analog.max, 177, "maxAnalog", 0, 1024);
}

/////////////////////////////
//// Create Pulse Time  /////
/////////////////////////////
long createPulseTime(int milis, int seconds, int minutes, int hours) {
  long pulse = 0;
  if (milis != 0) {
    pulse += milis;
    report("milis: " + String(milis));
  }
  if (seconds != 0) {
    pulse += seconds*1000UL;
    report("seconds: " + String(seconds));
  }
  if (minutes != 0) {
    pulse += minutes*60*1000UL;
    report("minutes: " + String(minutes));
  }
  if (hours != 0) {
    pulse += hours*60*60*1000UL;
    report("hours: " + String(hours));
  }
  report("pulseTime: " + String(pulse)); 
  return pulse;
}

////////////////////////////////////
//// Is Movement Sensor Active /////
////////////////////////////////////
bool IsMotionSensor() {
  if (sensorAnalog == 1 or sensorDigital1 == 1 or sensorDigital2 == 1) {
    return true;
  } else {
    return false;
  }
}

/////////////////////////
//// Byte to String /////
/////////////////////////
String byteToString(int value) {
  return String(char(value));
}

