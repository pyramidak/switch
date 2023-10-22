String menu = "main";
String newFirmwareVersion, lockPass;
bool web_restartNeeded; // restart needed
bool web_restartMust; // restart needed
////////////////////////////////////////////////////////////////////////////////////////////////
////       WEB REQUEST      ////////       WEB REQUEST      ////////       WEB REQUEST      ////
////////////////////////////////////////////////////////////////////////////////////////////////
void web_request(String &header) {
  //MAIN/////////////////////////////////////////////////////////////////////////////////
  if (header.indexOf("GET /main") >= 0) {
    menu = "main";
  } else if (header.indexOf("GET /restart") >= 0) {
    mqtt_disconnect();
    ESP.restart();
  } else if (header.indexOf("GET /reconnect") >= 0) {
    web_restartNeeded = false;
    wi_fi.reconnectNeeded = true;
    mqtt_reconnectNeeded = true;
  } else if (header.indexOf("GET /lock") >= 0) {
    menu = "lock";    
  } else if (header.indexOf("GET /device") >= 0) {
    menu = "device";
  } else if (header.indexOf("GET /sensor") >= 0) {
    menu = "sensor";
  } else if (header.indexOf("GET /switch1") >= 0) {
    menu = "switch1"; 
  } else if (header.indexOf("GET /switch2") >= 0) {
    menu = "switch2";
  } else if (header.indexOf("GET /led") >= 0) {
    menu = "led";
  } else if (header.indexOf("GET /mqtt") >= 0) {
    menu = "mqtt";
  } else if (header.indexOf("GET /wifi") >= 0) {
    menu = "wifi";
  } else if (header.indexOf("GET /clear") >= 0) {
    menu = "clear";
  } else if (header.indexOf("GET /firmware") >= 0) {
    menu = "firmware";
  //LOCK/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("GET /setlock") >= 0) {
    lockPass = "";
    menu = "main";
  } else if (header.indexOf("?unlock=") >= 0) {     
    lockPass = findSubmit("unlock");
  } else if (header.indexOf("?lock_pass=") >= 0) {
    lockPass = setOption("", "lock_pass", "password lock", 6, 170, 175, false);
  //SWITCH/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("GET /toggle/switch1") >= 0) {
    relay.toggle1("TOGGLE", false);
    mqtt_switch();
  } else if (header.indexOf("GET /toggle/switch2") >= 0) {
    relay.toggle2("TOGGLE", false);
    mqtt_switch();
  } else if (header.indexOf("GET /toggle/switch3") >= 0) {
    mqtt_switchX(relay.toggleX("TOGGLE", 3), "3");
  } else if (header.indexOf("GET /toggle/switch4") >= 0) {
    mqtt_switchX(relay.toggleX("TOGGLE", 4), "4");
  } else if (header.indexOf("GET /toggle/switch5") >= 0) {
    mqtt_switchX(relay.toggleX("TOGGLE", 5), "5");
  } else if (header.indexOf("GET /toggle/switch6") >= 0) {
    mqtt_switchX(relay.toggleX("TOGGLE", 6), "6");
  //LIGHT/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("?bright=") >= 0) {
    mqtt_brightness(setOption(relay.light.brightness, "bright", "brightness", 1, 255, 21, false));            
  //SENSOR/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("?1threshold=") >= 0) {     
    threshold1 = setOption(25, "1threshold", "light threshold", 1, 99, 26, false);  
  } else if (header.indexOf("?2threshold=") >= 0) {  
    sound.threshold = setOption(0, "2threshold", "sound threshold", 0, 1024, 27, false);      
  } else if (header.indexOf("?3threshold=") >= 0) {     
    ampere.threshold = setOption(float(0.0), "3threshold", "current threshold", 178, 179);      
  } else if (header.indexOf("?resistance=") >= 0) {
    int resistance = setOption(potentiometer.resistance(), "resistance", "resistance", 1, 99, 29, false);
    mqtt_resistance(potentiometer.resistance(resistance));
  } else if (header.indexOf("?fix_temp=") >= 0) {
    float value = findSubmit("fix_temp").toFloat();
    if (value < 0) value = abs(value);
    if (value > 9.9) value = 9.9;
    int val = value * 10.0;
    mem.write(val, 166);
    termistor.fixTemperature = value;         
    termistor.getTemperature();
  } else if (header.indexOf("?fix_humi=") >= 0) {
    termistor.fixHumidity = setOption(termistor.fixHumidity, "fix_humi", "humidity correction", 0, 25, 167, false);                            
    termistor.getHumidity();
  } else if (header.indexOf("GET /current/zero") >= 0) {
    mem.write(ampere.calibrate(), 28, 23);
  } else if (header.indexOf("GET /analog/min") >= 0) {
    analog.min = analog.reading;
    mem.write(analog.min, 176);
    report("min set: " + String(analog.min));    
    analog.measure();
  } else if (header.indexOf("GET /analog/max") >= 0) {
    analog.max = analog.reading;
    mem.write(analog.max, 177);    
    report("max set: " + String(analog.max));    
    analog.measure();
  //SEMAFOR/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("?red=") >= 0) {
    semafor.light("red", findSubmit("red").toInt());
  } else if (header.indexOf("?yellow=") >= 0) {
    semafor.light("yellow", findSubmit("yellow").toInt());
  } else if (header.indexOf("?green=") >= 0) {
    semafor.light("green", findSubmit("green").toInt());
  //MOTOR/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("GET /motor/forward") >= 0) {
    motor.forward();
    mqtt_motor(motor.state());
    report("motor command: forward");
  } else if (header.indexOf("GET /motor/backward") >= 0) {
    motor.backward();
    mqtt_motor(motor.state());
    report("motor command: backward");  
  } else if (header.indexOf("GET /motor/stop") >= 0) {
    motor.stop();
    motor.off();
    mqtt_motor(motor.state());
    report("motor command: stop");
  } else if (header.indexOf("?speed=") >= 0) {
    motor.speed(setOption(500, "speed", "motor speed", 4, 1024, 30, false));
  //DEVICE/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("?name=") >= 0) {
    deviceName = setOption(deviceName, "name", "device name", 15, 136, 150, true);
    wi_fi.deviceName = deviceName;
  } else if (header.indexOf("?purpose=") >= 0) {
    device = setOption(device, "purpose", "device purpose", 1, 9, 1, true);
    relay.device = device;
    web_restartMust = true;
  } else if (header.indexOf("?sleep=") >= 0) {
    sleepMode = setOption(sleepMode, "sleep", "sleep mode", 0, 2, 2, false);            
  //SENSOR/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("?1analog=") >= 0) {
    sensorAnalog = setOption(sensorAnalog, "1analog", "analog sensor", 0, 11, 6, false);  
    ampere.begin(sensorAnalog);
    analog.begin(sensorAnalog);
    relay.motionSensor = IsMotionSensor();
  } else if (header.indexOf("?SWanalog=") >= 0) {
    analogSW = setOption(analogSW, "SWanalog", "switch for analog sensor", 0, 3, 5, false);  
  } else if (header.indexOf("?1digital=") >= 0) {
    sensorDigital1 = setOption(sensorDigital1, "1digital", "digital sensor", 0, 9, 7, false); 
    termistor.begin(sensorDigital1);
    relay.motionSensor = IsMotionSensor();
  } else if (header.indexOf("?2digital=") >= 0) {
    sensorDigital2 = setOption(sensorDigital2, "2digital", "digital2 sensor", 0, 6, 8, false); 
    relay.motionSensor = IsMotionSensor();
  //SWITCH1+2/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("?start=") >= 0) {
    if (menu == "switch1") {
      relay.startS1 = setOption(relay.startS1, "start", "switch1 start state", 0, 1, 9, false); 
    } else if (menu == "switch2") {
      relay.startS2 = setOption(relay.startS2, "start", "switch2 start state", 0, 1, 10, false); 
    }
  } else if (header.indexOf("?milis=") >= 0) {
    if (menu == "switch1") {
      relay.startS2 = setOption(mem.read(13), "milis", "PulseTime in milis", 0, 250, 13, false); 
      relay.timePulse1 = createPulseTime(mem.read(13), mem.read(14), mem.read(15), mem.read(16)); 
    } else if (menu == "switch2") {
      relay.startS2 = setOption(mem.read(17), "milis", "PulseTime in milis", 0, 250, 17, false); 
      relay.timePulse2 = createPulseTime(mem.read(17), mem.read(18), mem.read(19), mem.read(20));
    }
  } else if (header.indexOf("?seconds=") >= 0) {
    if (menu == "switch1") {
      setOption(mem.read(14), "seconds", "PulseTime in seconds", 0, 250, 14, false); 
      relay.timePulse1 = createPulseTime(mem.read(13), mem.read(14), mem.read(15), mem.read(16)); 
    } else if (menu == "switch2") {
      setOption(mem.read(18), "seconds", "PulseTime in seconds", 0, 250, 18, false); 
      relay.timePulse2 = createPulseTime(mem.read(17), mem.read(18), mem.read(19), mem.read(20));
    }
  } else if (header.indexOf("?minutes=") >= 0) {
    if (menu == "switch1") {
      setOption(mem.read(15), "minutes", "PulseTime in minutes", 0, 250, 15, false); 
      relay.timePulse1 = createPulseTime(mem.read(13), mem.read(14), mem.read(15), mem.read(16)); 
    } else if (menu == "switch2") {
      setOption(mem.read(19), "minutes", "PulseTime in minutes", 0, 250, 19, false); 
      relay.timePulse2 = createPulseTime(mem.read(17), mem.read(18), mem.read(19), mem.read(20));
    }
  } else if (header.indexOf("?hours=") >= 0) {
    if (menu == "switch1") {
      setOption(mem.read(16), "hours", "PulseTime in hours", 0, 250, 16, false); 
      relay.timePulse1 = createPulseTime(mem.read(13), mem.read(14), mem.read(15), mem.read(16)); 
    } else if (menu == "switch2") {
      setOption(mem.read(20), "hours", "PulseTime in hours", 0, 250, 20, false); 
      relay.timePulse2 = createPulseTime(mem.read(17), mem.read(18), mem.read(19), mem.read(20));
    }
  } else if (header.indexOf("?stayoff=") >= 0) {
    if (menu == "switch1") {
      relay.stayoffS1 = setOption(relay.stayoffS1, "stayoff", "stay off when sun", 0, 3, 11, false); 
    } else if (menu == "switch2") {
      relay.stayoffS2 = setOption(relay.stayoffS2, "stayoff", "stay off when sun", 0, 3, 12, false); 
    }
  } else if (header.indexOf("?effect=") >= 0) {
    relay.light.effect = setOption(relay.light.effect, "effect", "light effect", 0, 4, 24, false); 
  //LED/////////////////////////////////////////////////////////////////////////////////////       
  } else if (header.indexOf("?diodmode=") >= 0) {
    relay.diodLight = setOption(relay.diodLight, "diodmode", "LED behaver", 0, 3, 25, false); 
  } else if (header.indexOf("?diodbright=") >= 0) {
    relay.diodBright = setOption(relay.diodBright, "diodbright", "LED brightness", 0, 177, 22, false);           
  //MQTT/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("?server_mqtt=") >= 0) {
    mqtt_server = setOption(mqtt_server, "server_mqtt", "MQTT server", 25, 66, 90, true);
  } else if (header.indexOf("?user_mqtt=") >= 0) {
    mqtt_user = setOption(mqtt_user, "user_mqtt", "MQTT user", 15, 91, 105, true);
  } else if (header.indexOf("?pass_mqtt=") >= 0) {
    newMQTTpass = setOption(newMQTTpass, "pass_mqtt", "MQTT password", 15, 106, 120, true);
  //WIFI/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("?ssid_wifi=") >= 0) {
    wi_fi.ssid = setOption(wi_fi.ssid, "ssid_wifi", "WIFI SSID", 15, 36, 50, true);
    mem.write(1, 3);
  } else if (header.indexOf("?pass_wifi=") >= 0) {
    newWIFIpass = setOption(newWIFIpass, "pass_wifi", "WIFI password", 15, 51, 65, true);
    wi_fi.password = newWIFIpass;
    mem.write(1, 3);
  } else if (header.indexOf("?pass_ota=") >= 0) {
    newOTApass = setOption(newOTApass, "pass_ota", "OTA password", 15, 121, 135, true);
  } else if (header.indexOf("?mode_wifi=") >= 0) {
    setOption(mem.read(3), "mode_wifi", "WIFI mode", 0, 1, 3, true);          
  } else if (header.indexOf("?ap_wifi=") >= 0) {
    wi_fi.switchAP = setOption(wi_fi.switchAP, "ap_wifi", "wifi switch exists", 0, 2, 4, false); 
  } else if (header.indexOf("?ssid_ap=") >= 0) {
    wi_fi.nameAP = setOption("", "ssid_ap", "WIFI AP name", 5, 31, 35, false);
  //FIRMWARE/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("?address_update=") >= 0) {
    update_server = setOption(update_server, "address_update", "server address", 15, 151, 165, false);      
  } else if (header.indexOf("?remote_update=") >= 0) {
    updateStart = setOption(updateStart, "remote_update", "remote update", 0, 1, 168, false);          
  //CLEAR/////////////////////////////////////////////////////////////////////////////////////
  } else if (header.indexOf("GET /save_clear") >= 0) {
    mem.clear();
    menu = "main";
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////
////        WEB PAGE        ////////        WEB PAGE        ////////        WEB PAGE        ////
////////////////////////////////////////////////////////////////////////////////////////////////
void web_page(WiFiClient &client, String &header) {
  web_createPage(client);
  // Web Page Heading
  client.println("<body><h3>pyramidak switch firmware " + firmwareVersion + "</h3>");
  client.println("<h1>" + deviceName + "</h1>");

  //LOCK/////////////////////////////////////////////////////////////////////////////////////
  if (mem.readAndCheck("", "weblock password", 170, 175, true) != lockPass) {
    createSubmit(client, "PASSWORD", lockPass, "", "unlock", 6, "access lock to firmware setting");
    return;
  } 
  //MAIN/////////////////////////////////////////////////////////////////////////////////////
  if (menu == "main") {
    String state;
    if (device >= 1 and device <= 4) {
      if (device >=  3) {
        client.println("<p>LIGHT 1</p>");  
      } else {
        client.println("<p>SWITCH 1</p>");  
      }
      if (relay.stateS1 == true) {
        state = "ON";
      } else {
        state = "OFF";
      }              
      createButton(client, state, "toggle/switch1");     

      if (device >= 3) {
        client.println("<p>LIGHT 2</p>");                    
      } else {
        client.println("<p>SWITCH 2</p>");     
      }
      if (relay.stateS2 == true) {
        state = "ON";
      } else {
        state = "OFF";
      }        
      createButton(client, state, "toggle/switch2");
      if (device >=  3) {
        createSubmit(client, "BRIGHTNESS", relay.light.brightness, "1-255", "bright", 3, "");    
      }

      if (sun != 0) {
        if (sun == 1) {
          state = "above_horizon";
        } else {
          state = "below_horizon";
        }    
        client.println("<p>SUN STATE: " + state + "</p>");
      }        
    } else if (device == 5) {
      createSubmit(client, "RESISTANCE", potentiometer.resistance(), "1-99", "resistance", 2, "");
    } else if (device == 6) {
      createSubmit(client, "RED", semafor.red, "0-177", "red", 3, "");
      createSubmit(client, "YELLOW", semafor.yellow, "0-177", "yellow", 3, "");
      createSubmit(client, "GREEN", semafor.green, "0-177", "green", 3, "");
    } else if (device == 8) {            
      createButton(client, "S1.D6: " + relay.state(1), "toggle/switch1"); 
      createButton(client, "S2.D7: " + relay.state(2), "toggle/switch2"); 
      createButton(client, "S3.D5: " + relay.state(3), "toggle/switch3"); 
      createButton(client, "S4.D8: " + relay.state(4), "toggle/switch4"); 
      createButton(client, "S5.D1: " + relay.state(5), "toggle/switch5"); 
      createButton(client, "S6.D2: " + relay.state(6), "toggle/switch6");        
    } else if (device == 9) {      
      if (motor.clockwise == false or motor.running == false) createButton(client, "Forward", "motor/forward");   
      if (motor.clockwise == true or motor.running == false) createButton(client, "Backward", "motor/backward");   
      if (motor.running == true) createButton(client, "Stop", "motor/stop");   
      createSubmit(client, "DEFAULT SPEED", String(int(round(mem.read(30) * 4.03))), "4-1024", "speed", 4, "");
    }

    if (sensorAnalog >= 1 and sensorAnalog <= 6) {
      if (potentio > 200) {
        state = "ON";
      } else {
        state = "OFF";
      }  
      client.println("<p>ANALOG SENSOR: " + state + "</p>");
    } else if (sensorAnalog == 7) {
      String str = "";
      if (device == 9) str = " SPEED";
      client.println("<p>POTENTIOMETER" + str + ": " + String(potentio) + "</p>");
    } else if (analog.sensor == 8) {
      client.println("<p>LIGHT LEVEL: " + String(analog.percent) + " %</br>");
      createMenu(client, "SET DARK", "analog/min", false);
      createMenu(client, "SET MAX", "analog/max", false);
      client.println("</p>");
      createSubmit(client, "SUN THRESHOLD", String(threshold1) + " %", "1-99", "1threshold", 2, "");
    } else if (analog.sensor == 9) {
      client.println("<p>MOISTURE: " + String(analog.percent) + " %</br>");
      createMenu(client, "SET DRY", "analog/min", false);
      createMenu(client, "SET WET", "analog/max", false);
      client.println("</p>");
    } else if (sensorAnalog == 10) {
      client.println("<p>SOUND LEVEL: " + String(sound.level) + "</p>");
      createSubmit(client, "LEVEL THRESHOLD", String(sound.threshold), "1-1024", "2threshold", 4, "");
    } else if (ampere.connected) {
      client.println("<p>CURRENT: " + String(ampere.current) + " A</br>");
      createMenu(client, "SET ZERO", "current/zero", false);
      client.println("</p>");
      if (analogSW != 0) createSubmit(client, "SWITCH" + String(analogSW) + " ON THRESHOLD", String(ampere.threshold / 100.0) + " A", "float", "3threshold", 4, "");
    }
    
    if (sensorDigital1 >= 1 and sensorDigital1 <= 6) {
      client.println("<p>DIGITAL SENSOR: " + sensor_state(digital1) + "</p>");
    } else if (sensorDigital1 >= 7 and sensorDigital1 <= 9) {
      client.println("<p>TEMPERATURE: " + String(termistor.temperature) + "*C</p>");
      createSubmit(client, "temperature correction", String(termistor.fixTemperature*-1), "0.0-9.9", "fix_temp", 3, "");
      if (sensorDigital1 != 9) {
        client.println("<p>HUMIDITY: " + String(termistor.humidity) + "%</p>");
        createSubmit(client, "humidity correction", String(termistor.fixHumidity*-1), "0-25", "fix_humi", 2, "");
      }
    }

    if (sensorDigital2 >= 1 and sensorDigital2 <= 6) {
      client.println("<p>DIGITAL2 SENSOR: " + sensor_state(digital2) + "</p>");
    }
    //MAIN MENU/////////////////////////////////////////////////////////////////////////////////////
    client.println("<p>MAIN MENU</p>");
    if (web_restartNeeded == true) {
      if (web_restartMust == true) {
        createMenu(client, "Restart", "restart");
      } else {  
        createMenu(client, "Reconnect", "reconnect");
      }
      client.println("<p>RESTART NEEDED</p>");
    }
    createMenu(client, "Reload", "main");
    createMenu(client, "Lock", "lock");
    createMenu(client, "Device", "device");
    createMenu(client, "Sensor", "sensor");
    if (device == 3 or device == 4) {
      state = "Light";
    } else {
      state = "Switch";
    }  
    if ((device >= 1 and device <= 4) or device == 8) {createMenu(client, state + "1", "switch1");}
    if (device == 2 or device == 4 or device == 8) {createMenu(client, state + "2", "switch2");}
    if (device != 7 and device != 8) {createMenu(client, "LED", "led");}
    createMenu(client, "MQTT", "mqtt");
    createMenu(client, "WIFI", "wifi");
    createMenu(client, "Firmware", "firmware");
    createMenu(client, "Clear", "clear");
    createMenu(client, "Restart", "restart");
  //LOCK/////////////////////////////////////////////////////////////////////////////////////
  } else if (menu == "lock") {
    createSubmit(client, "PASSWORD", lockPass, "", "lock_pass", 6, "access lock to firmware setting");
    
    client.println("<p>LOCK MENU</p>");
    createMenu(client, "Lock", "setlock");
    createMenu(client, "Return", "main");    
  //DEVICE/////////////////////////////////////////////////////////////////////////////////////
  } else if (menu == "device") {
    createSubmit(client, "DEVICE PURPOSE", device, "1-9", "purpose", 1, "1-switch, 2-two switches, 3-light, 4-two lights, 5-digital potentiometer, 6-traffic lights, 7-sensors only, 8-six switches, 9-motor");
    createSubmit(client, "DEVICE NAME", deviceName, "New name", "name", 15, "");
    client.println("<p>for Light-Sleep Mode set sensor DIGITAL first</p>");
    if (sensorDigital1 >= 1 and sensorDigital1 <= 6) {
      createSubmit(client, "LIGHT SLEEP", sleepMode, "0-2", "sleep", 1, "0-OFF, 1-ON when sensor ON, 2-ON when sensor OFF");               
    }
    
    client.println("<p>DEVICE MENU</p>");
    createMenu(client, "Return", "main");
    client.println("DEVICE PINs:");
    client.println("<p>Switches: Switch1 D6, Switch2 D7</p>");
    client.println("<p>Digital Potentio.: CS D5, INC D6, UD D7</p>");
    client.println("<p>Traffic Lights: Red D6, Yellow D7, Green D8</p>");
    client.println("<p>Steper Motor: STEP D6, DIR D7, ENABLE D8, inverse DIR D5</p>");
  //SENSOR/////////////////////////////////////////////////////////////////////////////////////
  } else if (menu == "sensor") {
    client.println("<p>sensors options: 1-PIR sensor, 2/3/4/5-DOOR sensor</p>");
    if (device == 9) {
      createSubmit(client, "ANALOG SENSOR", sensorAnalog, "0-12", "1analog", 2, "0-none, switch 1-NO ON, 2-NO ON/OFF, 3-NO OFF, 4-NC ON, 5-NC ON/OFF, 6-NC OFF, 7-potentiometer 1K, 8-lightmeter, 9-moisture, 10-soundmeter KY-037, 11-current ACS712-5A");
    } else {
      createSubmit(client, "ANALOG SENSOR", sensorAnalog, "0-12", "1analog", 2, "0-none, switch 1-NO ON, 2-NO ON/OFF, 3-NO TOGGLE, 4-NC ON, 5-NC ON/OFF, 6-NC TOGGLE, 7-potentiometer 1K, 8-lightmeter, 9-moisture, 10-soundmeter KY-037, 11-current ACS712-5A");                
    }
    createSubmit(client, "ANALOG CONTROL", analogSW, "0-2", "SWanalog", 1, "0-none, 1-switch1, 2-switch2, 3-both");  
    if (device != 8) {
      if (device == 9) {
        createSubmit(client, "DIGITAL SENSOR", sensorDigital1, "0-9", "1digital", 1, "0-none, switch 1-NO ON, 2-NO ON/OFF, 3-NO OFF, 4-NC ON, 5-NC ON/OFF, 6-NC OFF, thermometer 7-DHT11, 8-DHT22, 9-DS18B20");
        createSubmit(client, "DIGITAL2 SENSOR", sensorDigital2, "0-6", "2digital", 1, "0-none, switch 1-NO ON, 2-NO ON/OFF, 3-NO OFF, 4-NC ON, 5-NC ON/OFF, 6-NC OFF");
      } else {
        createSubmit(client, "DIGITAL SENSOR", sensorDigital1, "0-9", "1digital", 1, "0-none, switch 1-NO ON, 2-NO ON/OFF, 3-NO TOGGLE, 4-NC ON, 5-NC ON/OFF, 6-NC TOGGLE, thermometer 7-DHT11, 8-DHT22, 9-DS18B20");
        createSubmit(client, "DIGITAL2 SENSOR", sensorDigital2, "0-6", "2digital", 1, "0-none, switch 1-NO ON, 2-NO ON/OFF, 3-NO TOGGLE, 4-NC ON, 5-NC ON/OFF, 6-NC TOGGLE");
      }
    }
    client.println("<p>SENSOR MENU</p>");
    createMenu(client, "Return", "main");
    client.println("SENSOR PIN:");
    client.println("<p>Analog A0, Digital D1, Digital2 D2</p>");
  //SWITCH1/////////////////////////////////////////////////////////////////////////////////////                            
  } else if (menu == "switch1") {
    createSubmit(client, "START STATE", relay.startS1, "0-1", "start", 1, "0-off, 1-on");
    createSubmit(client, "AUTO OFF MILIS", mem.read(13), "0-250", "milis", 3, "");
    createSubmit(client, "AUTO OFF SECONDS", mem.read(14), "0-250", "seconds", 3, "");
    createSubmit(client, "AUTO OFF MINUTES", mem.read(15), "0-250", "minutes", 3, "");
    createSubmit(client, "AUTO OFF HOURS",  mem.read(16), "0-250", "hours", 3, "");
    createSubmit(client, "IGNORE SENSORS", relay.stayoffS1, "0-3", "stayoff", 1, "0-never, 1-sun above horizon, 2-sun below horizon, 3-always");
    if (device >= 3 and device <= 4) {createSubmit(client, "LIGH EFFECT", relay.light.effect, "0-4", "effect", 1, "0-none, 1-light on/off gradually, 2-more slowly, 3-like dawn/dusk, 4-blinker");}

    client.println("<p>SWITCH1 MENU</p>");
    createMenu(client, "Return", "main");
  //SWITCH2/////////////////////////////////////////////////////////////////////////////////////                            
  } else if (menu == "switch2") {
    createSubmit(client, "START STATE", relay.startS2, "0-1", "start", 1, "0-off, 1-on");
    createSubmit(client, "AUTO OFF MILIS", mem.read(17), "0-250", "milis", 3, "");
    createSubmit(client, "AUTO OFF SECONDS", mem.read(18), "0-250", "seconds", 3, "");
    createSubmit(client, "AUTO OFF MINUTES", mem.read(19), "0-250", "minutes", 3, "");
    createSubmit(client, "AUTO OFF HOURS", mem.read(20), "0-250", "hours", 3, "");
    createSubmit(client, "IGNORE SENSORS", relay.stayoffS2, "0-3", "stayoff", 1, "0-never, 1-sun above horizon, 2-sun below horizon, 3-always");
    if (device >= 3 and device <= 4) {createSubmit(client, "LIGH EFFECT", relay.light.effect, "0-4", "effect", 1, "0-none, 1-light on/off gradually, 2-more slowly, 3-like dawn/dusk, 4-blinker");}

    client.println("<p>SWITCH2 MENU</p>");
    createMenu(client, "Return", "main");
  //LED/////////////////////////////////////////////////////////////////////////////////////                            
  } else if (menu == "led") {
    createSubmit(client, "LED STATE", relay.diodLight, "0-3", "diodmode", 1, "0-on when switch on,  1-on when switch off,  2-on when sensor on,  3-on when sensor off");
    createSubmit(client, "LED BRIGHTNESS", relay.diodBright, "1-177", "diodbright", 3, ""); 

    client.println("<p>EXTRA DIOD MENU</p>");
    createMenu(client, "Return", "main");
    client.println("<p>EXTRA DIOD PIN:</p>");
    client.println("<p>LED1 D5, LED2 D8</p>");
  //MQTT/////////////////////////////////////////////////////////////////////////////////////                            
  } else if (menu == "mqtt") {
    createSubmit(client, "BROKER", mqtt_server, "New broker", "server_mqtt", 15, "");
    createSubmit(client, "USER", mqtt_user, "New user", "user_mqtt", 15, "");
    createSubmit(client, "PASSWORD", newMQTTpass, "New pass", "pass_mqtt", 15, "");

    client.println("<p>MQTT MENU</p>");
    createMenu(client, "Return", "main"); 

    web_createMQTTmanual(client);             
  //WIFI///////////////////////////////////////////////////////////////////////////////////// 
  } else if (menu == "wifi") {
    createSubmit(client, "SSID", wi_fi.ssid, "New SSID", "ssid_wifi", 15, "");
    createSubmit(client, "PASSWORD", newWIFIpass, "New pass", "pass_wifi", 15, "");
    createSubmit(client, "NEXT START", mem.read(3), "New mode", "mode_wifi", 1, "0-Access Point, 1(2)-Client");
    createSubmit(client, "WIFI SWITCH", wi_fi.switchAP, "Exists", "ap_wifi", 1, "0-none, 1-WiFi AP, 2-Modem Sleep");
    createSubmit(client, "AP SSID", "pyramidak" + wi_fi.nameAP, "New name", "ssid_ap", 5, "");

    client.println("<p>WIFI MENU</p>");
    createMenu(client, "Return", "main");  
  //FIRMWARE///////////////////////////////////////////////////////////////////////////////////// 
  } else if (menu == "firmware") {
    createSubmit(client, "UPDATE SERVER", update_server, "http://", "address_update", 15, "");
    createSubmit(client, "UPDATE START", updateStart, "0-1", "remote_update", 15, "0 - manual only, 1 - mqtt command");
    createSubmit(client, "ARDUINO UPDATE", newOTApass, "New pass", "pass_ota", 15, "");

    client.println("<p>FIRMWARE MENU</p>");                                                     
    createMenu(client, "Return", "main");                             

    if (update_server != "") {
      if (header.indexOf("GET /update") >= 0) {
        client.println("<h3>update in progress...</br>");  
        client.println("...click on Return in a minute</h3>");  
        client.println("<p>update " + update_Server_begin() + "</p>");    
      } else {
        String newVersion = update_Server_version();
        if (newVersion.length() > 5) {
          client.println("<h3>" + newVersion + "</h3>");                         
          newFirmwareVersion = "";
        } else { 
          newFirmwareVersion = newVersion;         
        }
        if (newFirmwareVersion != "") {
          if(newFirmwareVersion != firmwareVersion) {createMenu(client, "Update", "update");}
          client.println("<h3>firmware version on server " + newFirmwareVersion + "</h3>");
        }
      } 
    }          
  //CLEAR///////////////////////////////////////////////////////////////////////////////////// 
  } else if (menu == "clear") {
    client.println("<p>CLEAR MEMORY SETTINGS</p>");
    createMenu(client, "Confirm", "save_clear");
    createMenu(client, "Return", "main");                            
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////
////       MQTT MANUAL      ////////       MQTT MANUAL      ////////       MQTT MANUAL      ////
////////////////////////////////////////////////////////////////////////////////////////////////
void web_createMQTTmanual(WiFiClient client) {
  client.println("<p>Topics for central system of</br>");
  client.println("smart home as Home Assistant</p>");
  
  client.println("<p>DEVICE TOPICS</p>");
  
  client.println("<p>HA configuration:</br>");
  client.println("platform: mqtt</br>");
  client.println("name: \"Kitchen\"</br>");
  client.println("unique_id: \"pyramidak_kitchen\"</br>");
  client.println("state_topic: \"devicename/switch1/state\"</br>");
  client.println("command_topic: \"devicename/switch1/command\"</br>");
  client.println("#switch2 create as second device if needed</br>");
  client.println("state_topic: \"devicename/switch2/state\"</br>");
  client.println("command_topic: \"devicename/switch2/command\"</br>");
  client.println("#use switch to control both switches as one device:</br>");
  client.println("state_topic: \"devicename/switch/state\"</br>");
  client.println("command_topic: \"devicename/switch/command\"</br>");
  client.println("#create a brightness control if supported:</br>");
  client.println("brightness_state_topic: \"devicename/brightness/state\"</br>");
  client.println("brightness_command_topic: \"devicename/brightness/command\"</p>");

  client.println("<p>Other commands:</br>");
  client.println("\"" + deviceName + "/restart/command\"</br>");
  client.println("\"" + deviceName + "/sleep/command\"</br>");
  client.println("\"" + deviceName + "/update/command\"</br>");
  client.println("\"" + deviceName + "/resistance/command\"</br>");
  client.println("\"" + deviceName + "/red/command\"</br>");
  client.println("\"" + deviceName + "/yellow/command\"</br>");
  client.println("\"" + deviceName + "/green/command\"</br>");
  client.println("\"" + deviceName + "/motor/command\" (FORWARD, BACKWARD, STOP, OFF)</br>");

  client.println("Other states:</br>");
  client.println("\"" + deviceName + "/status\"</br>");
  client.println("\"" + deviceName + "/ip/state\"</br>");
  client.println("\"" + deviceName + "/update/state\"</br>");
  client.println("\"" + deviceName + "/analog/state\"</br>");
  client.println("\"" + deviceName + "/digital/state\"</br>");
  client.println("\"" + deviceName + "/digital2/state\" (humidity)</br>");
  client.println("\"" + deviceName + "/resistance/state\"</p>");
  client.println("\"" + deviceName + "/motor/state\" (FORWARD, BACKWARD, STOP, OFF)</p>");
 
  client.println("<p>GLOBAL TOPICS</p>");

  client.println("<p>HA start - update devices states:</br>");
  client.println("\"pyramidak/update/command\"</br>");
  client.println("\"pyramidak/brightness/command\"</br>");
  client.println("\"pyramidak/switch/command\"</br>");
  client.println("\"pyramidak/sensor/command\"</br>");
  client.println("Sun MQTT automation:</br>");
  client.println("events sunrise, sunset, \"pyramidak/sun/command\"</br>");
  client.println("action \"pyramidak/sun/state\"</br>");
  client.println("payload_template: '{{states(''sun.sun'')}}'</p>");
}