class AmpereMeter {

private:
  const byte pinA = A0; 
  float currentJump;
  float currentLast = -1;
  unsigned long lastRead; 

  float getCurrentAC() {
    uint32_t period = 1000000 / 50;
    uint32_t t_start = micros();

    uint32_t Isum = 0, measurements_count = 0;
    int32_t Inow;

    while (micros() - t_start < period) {
      Inow = analogRead(pinA) - zeroPoint;
      Isum += Inow*Inow;
      measurements_count++;
    }

    float Irms = sqrt(Isum / measurements_count) / 1023.0 * 5.0 / 0.185;
    return Irms;
  }

public:
  int zeroPoint = 512;
  float current;
  bool connected;
  bool changed;
  int threshold;
  
  AmpereMeter() {}

  void begin(int sensor) {    
    if (sensor == 11) {
      connected = true;
    } else {
      connected = false;
      currentLast = -1;
    }
  }

  int calibrate() {
    if (connected == false) {return 0;}
    uint16_t acc = 0;
    for (int i = 0; i < 10; i++) {
      acc += analogRead(pinA);
    }
    zeroPoint = acc / 10;
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
      float value = getCurrentAC();  
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
