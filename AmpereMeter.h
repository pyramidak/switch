#include "ACS712.h" //sensor AC current

class AmpereMeter {

private:
  const byte pinA = A0; 
  ACS712 *ampere;
  float currentJump;
  float currentLast = -1;
  unsigned long lastRead; 

public:
  int zeroPoint;
  float current;
  bool connected;
  bool changed;
  int threshold;
  
  AmpereMeter() {}

  void begin(int sensor) {    
    if (sensor == 11) {
      ampere = new ACS712(ACS712_05B, pinA);
      connected = true;
      if (zeroPoint != 0) ampere->setZeroPoint(zeroPoint);
    } else {
      connected = false;
      currentLast = -1;
    }
  }

  int calibrate() {
    if (connected == false) {return 0;}
    zeroPoint = ampere->calibrate();
    measure();
    return zeroPoint;
  }

  void loop() {
    changed = false;
    if (connected == true and (millis() - lastRead >= 500UL or lastRead == 0)) {
      lastRead = millis();
      changed = measure();
    }
  }

  bool measure() {
    if (connected == true) {     
      float value = ampere->getCurrentAC();  
      //koeficient for 5V
      float koef; 
      if (value < 0.01) {
        koef = 1.9;
      } else if (value > 0.01 and value < 1) {
        koef = 1.7;
      } else if (value > 1 and value < 3) {
        koef = 1.5;
      } else if (value > 3) {
        koef = 1.3;
      }
      //value = value * koef;
      
      if (value > 1) {
        value = (round(value * 10) / 10);
        if (value > current) {
          value -= 0.05;
        } else {
          value += 0.05;
        }
        value = value * 1.15; //koeficient for 3V
      } else {
        if (value > current and value - current < 0.04) {
          value = current;
        } else if (value < current and current - value < 0.01) {
          value = current;
        } else if (value > current) {  
          if (abs(currentJump - value) < 0.02) {
            value = current;
          } else {
            currentJump = value;  
          }
        }
      }
      current = value;
      if ((current > 0.1 and abs(current - currentLast) > 0.1) or (current < 0.1 and abs(current - currentLast) > 0.01) or currentLast == -1) {
        currentLast = current;
        return true;
      }
    }
    return false;
  }
  
};
