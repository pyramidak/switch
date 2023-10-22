class Soundmeter {

private:
  const byte pinA = A0; 
  const int sampleLength = 128;
  int thresholdHigh = 0; //1 = sound exceeded threshold
  int SCD = 0; //sound calibration direction: 0 = none, 1 = down, 2 = up, 3 = done
  unsigned long lastNoise; //čas posledního reportu hluku
  unsigned long lastResistance; //čas poslední změny odporu
  unsigned long lastChange; //čas posledního záznamu zvuku
  DigitalPotentiometer *potentiometer;

public:
  int level;
  int threshold = 75;

  Soundmeter(DigitalPotentiometer *digitalPotentio) { 
    potentiometer = digitalPotentio;
  }

  int measure() {  
    long sum = 0;   
    for ( int i = 0 ; i < sampleLength; i ++) {  
    sum = sum + analogRead(pinA);  // Performs 128 signal readings 
    }   
    int thisLevel = sum / sampleLength; // Calculate the average value  

    //re-calibration 5 minutes
    if (millis() - lastChange > 5*60*1000) { 
      lastChange = millis();
      SCD = 0;
    }

    //calibration
    int lastSCD = SCD;
    if (millis() - lastResistance > 1000 and potentiometer->constructed == true) { 
      if (thisLevel > 1000 and SCD != 3) { 
        if (SCD == 0) {SCD = 1;}
        if (SCD == 2 or potentiometer->resistance() == 1) {SCD = 3;}
        potentiometer->resistance(potentiometer->resistance()-1); //sensitivity too high;
      } else if (thisLevel < 500 and SCD != 3) {
        if (SCD == 0) {SCD = 2;}
        if (SCD == 1 or potentiometer->resistance() == 99) {SCD = 3;}
        potentiometer->resistance(potentiometer->resistance()+1); //sensitivity too low    
      }
    }

    //level report
    thisLevel = 1024 - thisLevel;
    if (millis() - lastNoise > 333 and thisLevel != level and SCD == 3) {
      if (thisLevel > threshold) {
        lastNoise = millis();
        thresholdHigh = 1;
        level = thisLevel;
        return level;
      } else {
        if(thresholdHigh == 1) {
          lastNoise = millis();
          thresholdHigh = 0;
          level = 0;
          return level;
        }
      }
    }
    return -1;
  }

};