#include "LightEffect.h"

class Relays {

private:
  // PINs usage declaration //
  const byte pinS1 = D6;   //D6=12
  const byte pinS2 = D7;   //D7=13  
  const byte pinS3 = D5;   //D5=14  //D3=00 ! no pull-down
  const byte pinS4 = D8;   //D8=15  //D0=16 ! no pwm, used for wifi
  const byte pinS5 = D1;   //D1=05
  const byte pinS6 = D2;   //D2=04
  const byte pinL1 = D5;   //D5=11
  const byte pinL2 = D8;   //D8=14
  ////     Pulse Time    /////
  unsigned long lastPulse1;//last switch-on time switch1
  unsigned long lastPulse2;//last switch-on time switch2
  
  byte getPin(int pinS) {
    byte pin;
    switch (pinS) {
      case 1: return pinS1;
      case 2: return pinS2;
      case 3: return pinS3;
      case 4: return pinS4;
      case 5: return pinS5;
      default: return pinS6;
    }  
  }   

  int getSwitch(byte pin) {
    if (pin == pinS1) {
      return 1;
    } else if (pin == pinS2) {
      return 2;
    } else if (pin == pinS3) {      
      return 3;
    } else if (pin == pinS4) {
      return 4;
    } else if (pin == pinS5) {
      return 5;
    } else {
      return 6;
    }
  } 

  void report(String msg, bool offset = false) {
    if(offset) {Serial.println("");}
    Serial.println(msg);
  }

public:
  LightEffect light;
  int  device;
  long timePulse1;//time in milliseconds, after which the switch switches itself off (60*1000UL = 1 minute)
  long timePulse2;//time in milliseconds, after which the second switch switches itself off 
  unsigned long timeOffMotionSensor;//movement sensor off because sensor can see switch off
  bool motionSensor;     //movement sensor active
  bool manualBrightness; //mqtt brightness off because manual value chosen
  bool stateS1;          //pinS1 state
  bool stateS2;          //pinS2 state
  int  startS1;          //pinS1 turn on/off when device restarted
  int  startS2;          //pinS2 turn on/off when device restarted
  int  stayoffS1;        //0-none, 1-above horizon, 2-below horizon
  int  stayoffS2;        //0-none, 1-above horizon, 2-below horizon
  int  diodLight;        //how the diod lights should behave
  int  diodBright = 177; //default max.177
  
  Relays() {}

  void begin() {
    pinMode(pinS1, OUTPUT); // Relay Switch1/Light
    pinMode(pinS2, OUTPUT); // Relay Switch2
    pinMode(pinL1, OUTPUT); // Relay Diod1
    pinMode(pinL2, OUTPUT); // Relay Diod2
    if (device == 8) {
      pinMode(pinS3, OUTPUT); 
      pinMode(pinS4, OUTPUT); 
      pinMode(pinS5, OUTPUT); 
      pinMode(pinS6, OUTPUT); 
    }
    if (device >= 1 and device <= 4) {
      if (startS1 == 1) {    
        toggle1("ON", true);//here not yet known the state of the sun
        if (diodLight == 0) {digitalWrite(pinL1, HIGH);}
      } else {
        digitalWrite(pinS1, LOW);
        if (diodLight == 1) {digitalWrite(pinL1, HIGH);}
      }
      if (((device == 1 or device == 3) and startS1 == 0) or ((device >= 1 and device <= 4) and startS2 == 1)) {    
        toggle2("ON", true);
        if (diodLight == 0) {digitalWrite(pinL2, HIGH);}
      } else {
        digitalWrite(pinS2, LOW);
        if (diodLight == 1) {digitalWrite(pinL2, HIGH);}
      }
    } else if (device == 8) {
      if (startS1 == 1) {
        toggle1("ON", true);
      } else {
        digitalWrite(pinS1, LOW);
      }
      if (startS2 == 1) {
        toggle2("ON", true);
      } else {
        digitalWrite(pinS2, LOW);
      }
      digitalWrite(pinS3, LOW);
      digitalWrite(pinS4, LOW);
      digitalWrite(pinS5, LOW);
      digitalWrite(pinS6, LOW);
    }
  }

  bool loop() {
    bool offPulse = false;
    //auto-off switch1 and switch2
    if (timePulse1 > 0 and lastPulse1 > 0) {
      if (millis() - lastPulse1 >= timePulse1) {  
        lastPulse1 = 0; 
        offPulse = true;
        toggle1("OFF", false);
      }
    }
    if (timePulse2 > 0 and lastPulse2 > 0) {
      if (millis() - lastPulse2 >= timePulse2) {  
        lastPulse2 = 0;
        offPulse = true;
        toggle2("OFF", false);
      }
    }
    //timing movement sensor off because switch off can effect sensor and if not, then:
    if (timeOffMotionSensor != 0) {
      if (millis() - timeOffMotionSensor >= 5*1000UL) {timeOffMotionSensor = 0;}  
    }
    //gradually turn on/off light
    light.loop();
    return offPulse;
  }

  String state(int pin) {  
    return state(getPin(pin));
  }

  String state(byte pin) {  
    if (digitalRead(pin) == HIGH) {
      return "ON";
    } else {
      return "OFF";
    }  
  }

  void toggle(int sw, String toggle, int sun) {
    int deviceReturn;
    if (sw == 3 and device == 3) { //light both blinkers in one light device
      deviceReturn = device;
      device = 4;
    }  
    //sw: 1 = switch1, 2 = switch2, 3 = both switches, 4 = switch2 (GPIO), 5 = switch1 (GPIO)
    if (sw == 1 or sw ==  3 or sw == 5) {
      if ((sun == 1 and stayoffS1 == 1) or (sun == 2 and stayoffS1 == 2) or stayoffS1 == 3) {
        report("S1 stay OFF because of SUN");        
      } else {
        toggle1(toggle, false);
      }
    }
    if (sw == 2 or sw ==  3 or sw == 4) {
      if ((sun == 1 and stayoffS2 == 1) or (sun == 2 and stayoffS2 == 2) or stayoffS2 == 3) {
        report("S2 stay OFF because of SUN");        
      } else {
        toggle2(toggle, false);
      }
    }
    if (sw == 3 and device == deviceReturn) { //set back one light device
      device = deviceReturn;
    }  
  }

  void toggle1(String toggle, bool switch2) {
    if (toggle != "") {
      if (toggle == "TOGGLE") {
        if (stateS1 == false) {
          toggle = "ON";
        } else {
          toggle = "OFF";
        }
      }
      //start/stop auto turn off
      if (toggle == "ON") {
        lastPulse1 = millis();
        if(timePulse1 > 0) {report("PulseTime1 ON");}
      } else if(toggle == "OFF") {
        lastPulse1 = 0;
        if(timePulse1 > 0) {report("PulseTime1 OFF");}
      }
      
      if(toggle == "ON" and stateS1 == false) {
        light.stepMiliS1 = 0; //stop light effect  
        if ((device == 1 or device == 3) and switch2 == false) {
          toggle2("OFF", true);
          delay(100);
        }
        stateS1 = true;  
        if (device == 3 or device == 4) {
          light.setEffect(true, pinS1);
        } else {
          digitalWrite(pinS1, HIGH);  
        }
        report("pinS1 ON");
        digitalWrite(LED_BUILTIN, LOW);
        report("led ON");
      }
      else if(toggle == "OFF" and stateS1 == true) {
        if (motionSensor == true) {timeOffMotionSensor = millis();} //ignore movement sensor for a while
        manualBrightness = false; //turn on mqtt brigtness
        light.stepMiliS1 = 0; //stop light effect  
        stateS1 = false;
        light.setEffect(false, pinS1);
        report("pinS1 OFF");
        digitalWrite(LED_BUILTIN, HIGH);
        report("led OFF");
        if ((device == 1 or device == 3) and switch2 == false) {
          delay(100);
          toggle2("ON", true);
        }
      }
      diod(1, stateS1, 1);
    }
  }

  void toggle2(String toggle, bool switch1) {
    if (toggle != "") {
      if (toggle == "TOGGLE") {
        if (stateS2 == false) {
          toggle = "ON";
        } else {
          toggle = "OFF";
        }
      }
      //start/stop auto turn off
      if(toggle == "ON") {
        lastPulse2 = millis();
        if(timePulse2 > 0) {report("PulseTime2 ON");}
      } else if(toggle == "OFF") {
        lastPulse2 = 0;
        if(timePulse2 > 0) {report("PulseTime2 OFF");}
      }

      if(toggle == "ON" and stateS2 == false) {
        light.stepMiliS2 = 0; //stop light effect  
        if ((device == 1 or device == 3) and switch1 == false) {
          toggle1("OFF", true);
          delay(100);
        }
        stateS2 = true;
        if (device == 3 or device == 4) {
          light.setEffect(true, pinS2);
        } else {
          digitalWrite(pinS2, HIGH);  
        }
        report("pinS2 ON");
      } 
      else if(toggle == "OFF" and stateS2 == true) {   
        if (motionSensor == true) {timeOffMotionSensor = millis();} //ignore movement sensor for a while
        manualBrightness = false; //turn on mqtt brigtness
        light.stepMiliS2 = 0; //stop light effect  
        stateS2 = false;
        light.setEffect(false, pinS2);
        report("pinS2 OFF");
        if ((device == 1 or device == 3) and switch1 == false) {
          delay(100);
          toggle1("ON", true);
        }
      }  
      diod(2, stateS2, 1);
    } 
  }

  String toggleX(String toggle, int pin) {
    return toggleX(toggle, getPin(pin));
  }

  String toggleX(String toggle, byte pin) {
    if (toggle != "") {
      if (toggle == "TOGGLE") {
        if (digitalRead(pin) == LOW) {
          toggle = "ON";
        } else {
          toggle = "OFF";
        }
      }
      String pinS = "pinS" + String(getSwitch(pin));
      if(toggle == "ON" and digitalRead(pin) == LOW) {
        digitalWrite(pin, HIGH);  
        report(pinS + " ON");
      } 
      else if(toggle == "OFF" and digitalRead(pin) == HIGH) {   
        digitalWrite(pin, LOW);  
        report(pinS + " OFF");
      }  
    }
    return state(pin);  
  }

  void brightness(int bright) {
    if (device >= 3 and device <= 4) {
      light.brightness = bright;
      if (stateS1 == true) {
        analogWrite(pinS1, bright);
        report("pinS1: " + String(bright));  
      }  
      if (stateS2 == true) {
        analogWrite(pinS2, bright);
        report("pinS2: " + String(bright));  
      }  
    }
  }

  void diod(int light, bool state, int logic) { //logic: 1 = switch, 2 = sensor
    byte pinL;
    if (light == 1) {pinL = pinL1;}
    if (light == 2) {pinL = pinL2;}
    
    if (logic == 1) {
      if (diodLight < 2 and device != 8) {
        if ((state == true and diodLight == 0) or (state == false and diodLight == 1)) { 
          analogWrite(pinL, diodBright);
        } else { 
          digitalWrite(pinL, LOW);
        }   
      }
    } else if (logic == 2) {
      if (diodLight == 2 or diodLight == 3) {
        if ((state == true and diodLight == 2) or (state == false and diodLight == 3)) { 
          analogWrite(pinL, diodBright);
        } else { 
          digitalWrite(pinL, LOW);
        }      
      }
    }    
  }

};