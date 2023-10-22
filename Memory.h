#include <EEPROM.h> //permanent memory

class Memory {

private:
  int memAlok;
  int memUsed = 180 + 50;

  void report(String msg, bool offset = false) {
    if(offset) {Serial.println("");}
    Serial.println(msg);
  }

  int findFreeMem() {
    //check the occupied memory
    int start, end;
    String text;
    start = 0;
    for (int i = 1; i < 50 + 1; i++) { 
      int one = EEPROM.read(i);
      if (one == 255) {
        break;
      } else {
        if (start == 0) {start = i;}
        end = i;      
        text += String(char(one));    
      }
    }
    report ("memOccupied: "+ text, true);
    report ("start: " + String(start) + "; end: " + String(end));
    if (end == 50) {end = 0;}
    return end + 1;
  } 

public:
  
  Memory() {}

  void begin() {
    EEPROM.begin(memUsed);  
    report("EEPROM reading", true);
    //alokace paměti
    memAlok = EEPROM.read(0);
    if (memAlok > 50) {
      memAlok = 1;    
    } else if (memAlok == 255) {
      memAlok = findFreeMem();
    }
    report("memAlokFrom: " + String(memAlok)); 
    if (EEPROM.read(0) != memAlok) {
      clear();
      EEPROM.write(0, memAlok);
      EEPROM.commit();
    }
  }

  void clear() {
    bool change;
    for (int i = memAlok; i <= memUsed; i++) { 
      if (EEPROM.read(i) != 255) {
        EEPROM.write(i, 255);
        change = true;
      }
    }  
    EEPROM.write(0, 255);
    if(change) {EEPROM.commit();}
    report("EEPROM cleared"); 
  }

  int read(int pos) {
    int value = EEPROM.read(pos + memAlok);
    if (value == 255) value = 0;
    return value;
  } 

  String read(int start, int end) {
    start = start + memAlok;
    end = end + memAlok;
    String text;
    for (int i = start; i < end + 1; i++) { 
      int one = EEPROM.read(i);
      if (one == 255) {
        break;
      } else {
        text += String(char(one));    
      }
    }
    return text;
  }

  int readAndCheck(int def, int pos, String text, int min, int max) {
    pos += memAlok;
    int value = EEPROM.read(pos);
    if (value != 255) report(text + ": " + String(value));
    if (max > 254) value = round(value * 4.03);
    if (value >= min and value <= max) {
      return value;
    } else {
      return def;
    }
  }

  int readAndCheck(int posLow, int posHigh, String text) {
    posLow += memAlok;
    posHigh += memAlok;
    int valLow = EEPROM.read(posLow);
    if (valLow == 255) valLow = 0;
    int valHigh = EEPROM.read(posHigh);
    if (valHigh == 255) valHigh = 0;
    int value = (valHigh * 254) + valLow;
    report(text + ": " + String(value));
    return value;
  }

  String readAndCheck(String def, String text, int start, int end, bool password) {
    String value = read(start, end);
    if (value != "") {
      if (password == true) {
        report(text + ": *"); 
      } else {
        report(text + ": " + value); 
      }   
      return value;
    } else {
      return def;
    }   
  }

  float readAndCheck(float def, int pos1, String text) {
    pos1 += memAlok;    
    union {
      float floats;
      unsigned char bytes[4];
    } convert;
    for (int i=0; i<4; i++) {
      convert.bytes[i] = EEPROM.read(pos1 + i);
    }
    if (isnan(convert.floats)) {
      return def;
    } else {
      report(text + ": " + String(convert.floats));
      return convert.floats;
    }
  }

  void write(float value, int pos) {
    pos += memAlok;
    union {
      float floats;
      unsigned char bytes[4];
    } convert;
    convert.floats = value;
    for (int i=0; i<4; i++) {
      if (EEPROM.read(pos + i) != convert.bytes[i]) {
        EEPROM.write(pos + i, convert.bytes[i]);
      }
    }
    EEPROM.commit(); 
  }

  void write(int value, int pos) {
    pos += memAlok;
    if (value > 254) value = round(value / 4.03);
    if (EEPROM.read(pos) != value) {
      EEPROM.write(pos, value);
      EEPROM.commit();
    }
  }

  void write(int value, int posLow, int posHigh) {
    posLow += memAlok;
    posHigh += memAlok;
    int valLow = value % 254;
    int valHigh = floor(value / 254.0);
    if (EEPROM.read(posLow) != valLow or EEPROM.read(posHigh) != valHigh) {
      EEPROM.write(posLow, valLow);
      EEPROM.write(posHigh, valHigh);
      EEPROM.commit();
    }
  }

  void write(String text, int start, int end) {
    start += memAlok;
    end += memAlok;
    for (int i = 0; i < end - start + 1; i++) { 
      byte one;
      if (i < text.length()) {
        one = byte(text.charAt(i));
      } else {
        one = 255;
      }
      if (EEPROM.read(i + start) != one) {
        EEPROM.write(i + start, one);  
      }
    }
    EEPROM.commit();
    report("EEPROM_" + String(start) + "-" + String(end) + "(" + String(start - memAlok) + "-" + String(end - memAlok) + "): " + text);   
  }

};
