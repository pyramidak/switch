class LightEffect {

private:
  const byte pinS1 = D6;   //D6=12
  const byte pinS2 = D7;   //D7=13  
  unsigned long stepLastS1, stepLastS2;  //last time to change step
  int stepOneS1, stepOneS2;  //step size
  int stepCurrentS1, stepCurrentS2; //state of step progress
  bool stepUpS1, stepUpS2;   //direction of step  

  void activate(bool up, int one, int mili, byte pin) {
    int stepCurrent;
    if (up) {
      stepCurrent = 0;
    } else {
      if (stepCurrent == brightness or stepCurrent == 0) {
        stepCurrent = brightness;  
      }
    }
    if (pin == pinS1) {
      stepUpS1 = up;
      stepOneS1 = one;
      stepCurrentS1 = stepCurrent;
      stepMiliS1 = mili;  
      stepLastS1 = 0;
    } else if (pin == pinS2) {
      stepUpS2 = up;
      stepOneS2 = one;
      stepCurrentS2 = stepCurrent;
      stepMiliS2 = mili;  
      stepLastS2 = 0;
    }  
  }

  void step(byte pin, bool &stepUp, int stepOne, int &stepCurrent, int &stepMili, unsigned long &stepLast) {
    if (millis() - stepLast >= stepMili or stepLast == 0) {
      stepLast = millis();
      if (stepUp) {
        stepCurrent += stepOne;
        if (stepCurrent > brightness) {
          if (effect != 4) {stepMili = 0;}
          stepCurrent = brightness;
        }
      } else {
        stepCurrent -= stepOne;
        if (stepCurrent < 0) {
          if (effect != 4) {stepMili = 0;}
          stepCurrent = 0;
        }
      
      }
      if (effect == 4) {stepUp = !stepUp;}
      analogWrite(pin, stepCurrent);
    }
  }

public:
  int  stepMiliS1, stepMiliS2;//time step length and 0 value turn off effect
  int  effect;                //0-none, 1-light on/off gradually, 2-more slowly, 3-like dawn/dusk
  int  brightness = 255;      //default max.255

  LightEffect() {}

  void setEffect(bool up, byte pin) {
    if (effect == 1) {
      activate(up, 32, 100, pin);
    } else if (effect == 2) {
      activate(up, 24, 1000, pin);
    } else if (effect == 3) {
      activate(up, 16, 60*1000, pin);
    } else if (effect == 4 and up == true) {
      activate(up, 256, 500, pin);
    } else {
      if (up == true) {
        analogWrite(pin, brightness); 
      } else {
        digitalWrite(pin, LOW); 
      }
    }
  }  

  void loop() {
    if (stepMiliS1 != 0) {step(pinS1, stepUpS1, stepOneS1, stepCurrentS1, stepMiliS1, stepLastS1);}
    if (stepMiliS2 != 0) {step(pinS2, stepUpS2, stepOneS2, stepCurrentS2, stepMiliS2, stepLastS2);}
  }

};