class TrafficLights {

private:
   // PINs usage declaration //
  const byte pinBlue   = D4;   //D4=02 on-board diod
  const byte pinRed    = D6;   //D6=12
  const byte pinYellow = D7;   //D7=13  
  const byte pinGreen  = D8;   //D8=14
  int diodBright = 177;   //default brightness of semafor lights

public:
  int red, yellow, green; //brightness of semafor lights

  TrafficLights() {}

  void begin() {
    pinMode(pinBlue, OUTPUT); // onboard Led light
    pinMode(pinRed, OUTPUT); // onboard Led light
    pinMode(pinYellow, OUTPUT); // onboard Led light
    pinMode(pinGreen, OUTPUT); // onboard Led light
  }

  void light() {
    analogWrite(pinRed, red);
    analogWrite(pinYellow, yellow);  
    analogWrite(pinGreen, green);
  }

  int brightness(int bright = 0) {
    if (bright > 0 and bright <= 177) {
      diodBright = bright;
      if (red != 0) red = diodBright;
      if (yellow != 0) yellow = diodBright;
      if (green != 0) green = diodBright;
      light();
    }
    return diodBright;
  }

  void light(String color, int value) {
    if (value < 0 or value > 177) {value = diodBright;}
    red = 0;
    yellow = 0;
    green = 0;
    if (color == "red") {
      red = value;
    } else if (color == "yellow") {
      yellow = value;
    } else if (color == "green") {
      green = value;  
    }
    light();
  }

  void light(int color) {
    red = 0;
    yellow = 0;
    green = 0;
    if (color == 3) {
      red = diodBright;
    } else if (color == 2) {
      yellow = diodBright;
    } else if (color == 1) {
      green = diodBright;
    }
    light();
  }

  void blue(bool state) {
    if (state == true) {
      analogWrite(pinBlue, diodBright);
    } else {
      analogWrite(pinBlue, 0);
    }
  }
};
