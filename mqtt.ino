#include <PubSubClient.h> //mqtt

WiFiClient espClient;
PubSubClient mqtt(espClient);

// MQTT private //
unsigned long mqttConn;  //mqtt connection try
unsigned long mqttReport;//time of last mqtt connection status report
String sensor_analog_state;
String sensor_digital1_state;
String sensor_digital2_state;
 // MQTT TOPICs //
String mqtt_status          = "/status";
String device_restart       = "/restart/command";
String device_sleep         = "/sleep/command";
String device_update_command= "/update/command";
String device_update_state  = "/update/state";
String device_update_global = "pyramidak/update/command";
String device_ip_state      = "/ip/state";
String switch_state         = "/switch/state";
String switch_command       = "/switch/command";
String switch1_state        = "/switch1/state";
String switch1_command      = "/switch1/command";
String switch2_state        = "/switch2/state";
String switch2_command      = "/switch2/command";
String switch_global        = "pyramidak/switch/command";
String brightness_state     = "/brightness/state";
String brightness_command   = "/brightness/command";
String brightness_global    = "pyramidak/brightness/command";
String red_command          = "/red/command"; 
String yellow_command       = "/yellow/command"; 
String green_command        = "/green/command"; 
String analog_state         = "/analog/state";
String digital_state        = "/digital/state";
String digital2_state       = "/digital2/state";
String sensor_global        = "pyramidak/sensor/command";
String sun_state_global     = "pyramidak/sun/state";
String sun_command_global   = "pyramidak/sun/command";
String resistance_state     = "/resistance/state"; 
String resistance_command   = "/resistance/command"; 
String motor_command        = "/motor/command"; 
String motor_state          = "/motor/state"; 

bool mqtt_connected() {
  return mqtt.connected();
}

void mqtt_disconnect() {
  mqtt_reconnectNeeded = false;
  mqttReport = 0;
  mqttConn = 0;
  mqtt.publish((deviceName + mqtt_status).c_str(), "offline");
  report("MQTT status: offline");
  mqtt.disconnect();
  mqtt.setServer(mqtt_server.c_str(), 1883);
}

void mqtt_loop() {
  if (mqtt_reconnectNeeded == true) mqtt_disconnect();
  if (mqtt.connected() == true) {
    if (millis() - mqttReport >= 60*1000UL or mqttReport == 0) {  
      mqttReport = millis();  
      mqtt.publish((deviceName + mqtt_status).c_str(), "online");
      report("MQTT status: online");
    }
    mqtt.loop();    
  } else {
    if (millis() - mqttConn >= 10*1000UL or mqttConn == 0) {
      mqttConn = millis();
      mqtt_connect();
    }
  }
}

void mqtt_begin() { 
  mqtt.setServer(mqtt_server.c_str(), 1883);
  mqtt.setCallback(mqtt_callback);  
}

void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  //convert topic to string to make it easier to work with
  String topicStr = topic; 
  String payloadStr = "";
  for (int i = 0; i < length; i++) { payloadStr = payloadStr + (char)payload[i]; }
    
  report("Message arrived:", true);  
  report("topic: " + topicStr);  
  report("payload: " + payloadStr);  

  if(topicStr == (deviceName + device_restart)) {
    report("restarting...");  
    mqtt_disconnect();
    ESP.restart();
  }

  if(topicStr == (deviceName + device_sleep)) light_sleep();

  if(topicStr == (deviceName + device_update_command) or topicStr == device_update_global) {
    if (updateStart == 1) update_Server_check();
  }
 
  if(topicStr == switch_global) {
    mqtt_switch();
    if (device == 8) { 
      for (int pin = 3 ; pin < 7; pin ++) { 
        mqtt_switchX(relay.toggleX("", pin), String(pin));
      }
    }     
  }
  if (device == 8) { 
    for (int pin = 3 ; pin < 7; pin ++) { 
      if (topicStr == deviceName + "/switch" + String(pin) + "/command") {  
        mqtt_switchX(relay.toggleX(payloadStr, pin), String(pin));
      }   
    }
  }
  if(topicStr == (deviceName + switch_command)) {
    if (payloadStr == "TOGGLE") {
      if (relay.stateS1 == true or relay.stateS2 == true) {
        payloadStr = "OFF";
      } else {
        payloadStr = "ON";
      }
    }
    relay.toggle1(payloadStr, true);
    relay.toggle2(payloadStr, true);
    mqtt_switch();
  }
  if(topicStr == (deviceName + switch1_command)) {
    relay.toggle1(payloadStr, false);
    mqtt_switch();
  }
  if(topicStr == (deviceName + switch2_command)) {
    relay.toggle2(payloadStr, false);
    mqtt_switch();
  }

  if(topicStr == (deviceName + brightness_command) or topicStr == brightness_global) {
    if(payloadStr == "") {
      mqtt_brightness(-1);
    } else {
      if (relay.manualBrightness == false) {
        mqtt_brightness(payloadStr.toInt());  
      } else {
        report("Brightness not changed - potentiometer priority.", true);
      }
    }
  }

  if(topicStr == (deviceName + resistance_command)) {
    if (payloadStr != "") {
      mqtt_resistance(potentiometer.resistance(payloadStr.toInt()));
    }
  }     

  if(topicStr == sun_state_global) { 
    if(payloadStr == "above_horizon") {
      sun = 1;
      report("sun is above_horizon");
    } else if(payloadStr == "below_horizon") {
      sun = 2;
      report("sun is below_horizon");
    }
  }

  if(topicStr == sensor_global) {
    mqtt_update(firmwareVersion);
    mqtt_analog("");
    mqtt_digital1("");
    mqtt_digital2("");
    if (device == 9) mqtt_motor(motor.state());
  }

  if(topicStr == (deviceName + red_command) or topicStr == (deviceName + yellow_command) or topicStr == (deviceName + green_command)) {
    if (payloadStr == "") {return;}
    int value = payloadStr.toInt();
    if (topicStr == (deviceName + red_command)) {
      semafor.light("red", value);
    } else if (topicStr == (deviceName + yellow_command)) {
      semafor.light("yellow", value);
    } else if (topicStr == (deviceName + green_command)) {
      semafor.light("green", value);
    }  
  }

  if(topicStr == (deviceName + motor_command)) {
    if (motor.command(payloadStr) == true) {
      mqtt_motor(motor.state());
    } else {  
      mqtt_motor("UNKNOWN");
    } 
  } 
}

void mqtt_update(String version) { 
  mqtt.publish((deviceName + device_ip_state).c_str(), WiFi.localIP().toString().c_str()); 
  mqtt.publish((deviceName + device_update_state).c_str(), version.c_str());
  report("update state and IP address published: " + version);            
}

void mqtt_motor(String state) { 
  if (mqtt.connected()) { 
    mqtt.publish((deviceName + motor_state).c_str(), state.c_str());                      
    report("motor state published: " + state);
  }
}

void mqtt_resistance(int value) {
  if (mqtt.connected()) { 
    mqtt.publish((deviceName + resistance_state).c_str(), String(value).c_str());
    report("resistance state published: " + String(value));
  }
}

void mqtt_switch() {
  if (mqtt.connected()) {
    String state;
    if (relay.stateS1 == true) {
      state = "ON";
    } else {
      state = "OFF";
    }   
    mqtt.publish((deviceName + switch1_state).c_str(), state.c_str()); 
    report("switch1 state published: " + state);

    if (relay.stateS2 == true) {
      state = "ON";
    } else {
      state = "OFF";
    } 
    mqtt.publish((deviceName + switch2_state).c_str(), state.c_str()); 
    report("switch2 state published: " + state);
    
    if (relay.stateS1 == true or relay.stateS2 == true) {
      state = "ON";
    } else {
      state = "OFF";
    } 
    mqtt.publish((deviceName + switch_state).c_str(), state.c_str());  
    report("switches state published: " + state);
  } 
}

void mqtt_switchX(String state, String pin) {
   if (mqtt.connected()) {
    mqtt.publish((deviceName + "/switch" + pin + "/state").c_str(), state.c_str());
    report("pinS" + pin + " state published: " + state);
  } 
}

void mqtt_brightness(int bright) {
  if (bright > 0 and bright < 256) relay.brightness(bright);
  bright = relay.light.brightness;
  if (mqtt.connected()) { 
    mqtt.publish((deviceName + brightness_state).c_str(), String(bright).c_str());
    report("brightness state published: " + String(bright));
  }
}

void mqtt_analog(String state) {
  if (state == "" and sensor_analog_state == "") {
    return;
  } else if (state == "") {
    state = sensor_analog_state;  
  } else {
    sensor_analog_state = state;  
  }
  if (mqtt.connected()) { 
    mqtt.publish((deviceName + analog_state).c_str(), state.c_str()); 
    report("analog sensor state published: " + state);
  }
}

void mqtt_digital1(String state) {
  if (state == "" and sensor_digital1_state == "") {
    return;
  } else if (state == "") {
    state = sensor_digital1_state;  
  } else {
    sensor_digital1_state = state;  
  }
  if (mqtt.connected()) { 
    mqtt.publish((deviceName + digital_state).c_str(), state.c_str()); 
    report("digital sensor state published: " + state);
  }
}

void mqtt_digital2(String state) {
  if (state == "" and sensor_digital2_state == "") {
    return;
  } else if (state == "") {
    state = sensor_digital2_state;  
  } else {
    sensor_digital2_state = state;  
  }
  if (mqtt.connected()) { 
    mqtt.publish((deviceName + digital2_state).c_str(), state.c_str()); 
    report("digital2 sensor state published: " + state);
  }
}

void mqtt_connect() {
  // Attempt to connect
  if (mqtt.connect(deviceName.c_str(), mqtt_user.c_str(), mqtt_password.c_str())) {
    mqtt.publish((deviceName + mqtt_status).c_str(), "offline");
    report("MQTT was offline."); 
  
    mqtt.subscribe((deviceName + device_restart).c_str());
    report("restart subscribed");
    mqtt.subscribe((deviceName + device_sleep).c_str());
    report("sleep subscribed");    
    
    mqtt.subscribe((deviceName + device_update_command).c_str());
    mqtt.subscribe(device_update_global.c_str());
    mqtt_update(firmwareVersion);
    report("update subscribed");
    
    //devices: 1 = switch //2 = two switches //3 = light //4 = sensor //5 = matrix displaz
    if (device == 3 or device == 4) {
      mqtt_brightness(-1);
      mqtt.subscribe((deviceName + brightness_command).c_str());
      mqtt.subscribe(brightness_global.c_str());
      report("brightness subscribed"); 
    }
    if ((device >= 1 and device <= 4) or (device == 8)) {
      mqtt.subscribe((deviceName + switch_command).c_str());
      mqtt.subscribe(switch_global.c_str());
      report("switches subscribed");
      mqtt_switch();
      mqtt.subscribe((deviceName + switch1_command).c_str());
      report("switch1 subscribed");
      mqtt.subscribe((deviceName + switch2_command).c_str());
      report("switch2 subscribed");
      mqtt.subscribe(sun_state_global.c_str());
      report("sun subscribed");
      mqtt.publish(sun_command_global.c_str(), "");
      report("sun query published");
    }
    if (device == 5) {
      mqtt_resistance(-1);
      mqtt.subscribe((deviceName + resistance_command).c_str());
      report("resistance subscribed");
    }
    if (device == 6) {
      mqtt.subscribe((deviceName + red_command).c_str());
      mqtt.subscribe((deviceName + yellow_command).c_str());
      mqtt.subscribe((deviceName + green_command).c_str());
      report("red yellow green subscribed");
    }
    if (device == 8) {
      for (int pin = 3 ; pin < 7; pin ++) { 
        mqtt_switchX(relay.toggleX("", pin), String(pin));
        mqtt.subscribe((deviceName + "/switch" + String(pin) + "/command").c_str());
      }
      report("6 switches subscribed");
    }
    if (device == 9) {
      mqtt_motor(motor.state());
      mqtt.subscribe((deviceName + motor_command).c_str());
      report("motor subscribed");
    }
    mqtt_analog("");
    mqtt_digital1("");
    mqtt_digital2("");
    mqtt.subscribe(sensor_global.c_str());
    report("sensors subscribed");

    mqtt.publish((deviceName + mqtt_status).c_str(), "online");
    report("MQTT connected to " + mqtt_server, true); 
    
  } else {
    report("MQTT connn.failed: " + String(mqtt.state()));
  }
}
