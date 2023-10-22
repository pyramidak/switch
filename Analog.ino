void sensor_analog() {
  int reading = analogRead(pinA);
  int p = reading / 4.01;
  String state;
  if (sensorAnalog == 8) { 
    if (device <= 4 and relay.stateS1 == false) {
      int sunLast = sun;
      if (analog.percent < threshold1) {
        sun = 2;
      } else {
        sun = 1;
      }     
      if (sunLast != sun) report("Sun changed: " + String(sun));
    }
  } else if (sensorAnalog == 7) {
    if (device == 9) {
      if (abs(reading - potentio) > 4) {
        potentio = reading;
        motor.speed(potentio);
        mqtt_analog(String(potentio));
      }
    } else {
      if (p > potentio + 5 or p < potentio - 5) {
        if (p > 249) {p = 255;}
        if (p < 6) {p = 0;}
        potentio = p;
        report("Potentiometer changed: " + String(potentio), true);
        if (device == 3 or device == 4) {
          relay.manualBrightness = true;
          if (potentio == 0) {
            relay.toggle(analogSW, "OFF", sun);    
          } else {
            relay.toggle(analogSW, "ON", sun);
            mqtt_brightness(potentio);
          }
          mqtt_switch();
        }
        mqtt_analog(String(potentio));  
      }
    }
  } else if (sensorAnalog >= 1 and sensorAnalog <= 3) {
    if (p > potentio + 200) {
      potentio = p;    
      if (device <= 4) {
        if (sensorAnalog == 3) {
          report("Analog switch: TOGGLE", true);
          relay.toggle(analogSW, "TOGGLE", sun);  
        } else {
          if (sensorAnalog == 1 and relay.timeOffMotionSensor != 0) {
            report("Movement sensor ignored because switch turning off", true);
          } else {
            report("Analog switch: ON", true);
            relay.toggle(analogSW, "ON", sun);    
          }
        }  
        mqtt_switch();
      } else if (device == 9 and (sensorAnalog == 1 or sensorAnalog == 2)) {
        report("Analog switch: ON", true); 
        if (analogSW == 1) {
          motor.forward();
        } else if (analogSW == 2) {
          motor.backward();
        }
      }
      mqtt_analog("ON");
    } else if (p < potentio - 200) {
      potentio = p;
      if (sensorAnalog == 1) {relay.timeOffMotionSensor = 0;}
      if (device <= 4 and sensorAnalog == 2) {
        report("Analog switch: OFF", true);  
        relay.toggle(analogSW, "OFF", sun);  
        mqtt_switch();
      } else if (device == 9 and (sensorAnalog == 2 or sensorAnalog == 3)) {
        report("Analog switch: OFF", true); 
        motor.stop();
      }  
      mqtt_analog("OFF");
    }
   } else if (sensorAnalog >= 4 and sensorAnalog <= 6) {
    if (p < potentio - 200) {
      potentio = p;
      if (device <= 4) {
        if (sensorAnalog == 6) {
          report("Analog switch: TOGGLE", true);
          relay.toggle(analogSW, "TOGGLE", sun);  
        } else {
          report("Analog switch: ON", true);
          relay.toggle(analogSW, "ON", sun);  
        }  
        mqtt_switch();
      } else if (device == 9 and (sensorAnalog == 4 or sensorAnalog == 5)) {
        report("Analog switch: ON", true); 
        if (analogSW == 1) {
          motor.forward();
        } else if (analogSW == 2) {
          motor.backward();
        }
      }  
      mqtt_analog("ON");
    } else if (p > potentio + 200) {
      potentio = p;
      if (device <= 4 and sensorAnalog == 5) {
        report("Analog switch: OFF", true);  
        relay.toggle(analogSW, "OFF", sun);  
        mqtt_switch();
      } else if (device == 9 and (sensorAnalog == 5 or sensorAnalog == 6)) {
        report("Analog switch: OFF", true); 
        motor.stop();
      }  
      mqtt_analog("OFF");
    }
  }  
}