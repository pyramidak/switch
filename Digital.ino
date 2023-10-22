void sensor_digital(int pinD, int sensorDigital, bool &digital) {
  int ON, OFF;
  if(sensorDigital >= 1 and sensorDigital <= 3) {
    ON = 1;
    OFF = 0;
  } else {
    ON = 0;
    OFF = 1;
  } 
  int light;
  if (pinD == pinD1) {light = 1;}
  if (pinD == pinD2) {light = 2;}
  if (digitalRead(pinD) == ON and digital == false) {
    delay(50); //ms short change ignore
    if (digitalRead(pinD) != ON) return;
    digital = true;
    relay.diod(light, digital, 2);
    if (device <= 4) {
      if (sensorDigital == 3 or sensorDigital == 6) {
        relay.toggle(pinD, "TOGGLE", sun);
      } else {
        if (sensorDigital == 1 and relay.timeOffMotionSensor != 0) {
          report("Movement sensor ignored because switch turning off", true);
        } else {
          relay.toggle(pinD, "ON", sun); 
        }
      } 
      mqtt_switch();
    } else if (device == 9) {
      if (sensorDigital == 1 or sensorDigital == 2 or sensorDigital == 4 or sensorDigital == 5) {
        if (pinD == 5) { //sensorD1
          motor.forward();
        } else {
          motor.backward();
        }
      }  
    }
    report("Digital switch: ON", true); 
    if (pinD == 5) { //sensorD1
      mqtt_digital1("ON");
    } else {
      mqtt_digital2("ON");
    }    
  } else if (digitalRead(pinD) == OFF and digital == true) {
    delay(50); //ms short change ignore
    if (digitalRead(pinD) != OFF) return;
    digital = false;
    if (sensorDigital == 1) {relay.timeOffMotionSensor = 0;}
    relay.diod(light, digital, 2);
    if (device <= 4) {
      if (sensorDigital == 2 or sensorDigital == 5) {
        relay.toggle(pinD, "OFF", sun);
        mqtt_switch();
      }  
    } else if (device == 9) {
      if (sensorDigital == 2 or sensorDigital == 3 or sensorDigital == 5 or sensorDigital == 6) {motor.stop();}  
    }
    report("Digital switch: OFF", true); 
    if (pinD == 5) { //sensorD1
      mqtt_digital1("OFF");
    } else {
      mqtt_digital2("OFF");
    }  
  }
}

String sensor_state(bool state) {
  if (state == true) { 
    return "ON";
  } else {
    return "OFF";
  }  
}