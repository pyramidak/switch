class StepperMotor {

private:
  const int ENABLE = D8;
  const int DIR = D7;
  const int STEP = D6;
  const int DIRR = D5;
  const int LED = D3;
  bool constructed;
  int multiplier = 85;//min 82*(10+0), 82*(10+1023) = 84706
  int thisSpeed = 500; //0-1023 (1-1024)

public:
  bool running;
  bool clockwise;
  
  StepperMotor() {}

  void begin() {
    pinMode(D3, OUTPUT);  
    pinMode(D5, OUTPUT);  
    pinMode(D6, OUTPUT);  
    pinMode(D7, OUTPUT);  
    pinMode(D8, OUTPUT);  
    constructed = true;
    Serial.print("Motor PINS - Enable: " + String(D8) + "; DIR: " + String(D7) + "; DIRR: " + String(D5)  + "; STEP: " + String(D6));
    off();
  }

  int speed(int value = -1) {
    if (value != -1) thisSpeed = value;
    return thisSpeed;
  }

  void run() {
    if (constructed == false or running == false) {return;}
    digitalWrite(STEP, HIGH);
    int motorDelay = (1024 - thisSpeed) * multiplier; //constant acceleration
    motorDelay = motorDelay / 1024.0 * (1024.0 * 1.1 - thisSpeed); //exponential acceleration
    motorDelay += multiplier * 10; //min speed
    delayMicroseconds(motorDelay);
    digitalWrite(STEP, LOW);
    delayMicroseconds(motorDelay); 
  }  

  void forward() {
    if (constructed == false) {return;}
    running = true;
    clockwise = true;
    on();
    digitalWrite(DIR, HIGH);
    digitalWrite(DIRR, LOW);
  } 

  void backward() {
    if (constructed == false) {return;}
    running = true;
    clockwise = false;
    on();
    digitalWrite(DIR, LOW);   
    digitalWrite(DIRR, HIGH);
  } 

  void stop() {
    if (constructed == false) {return;}
    running = false;
  } 

  void on() {
    if (constructed == false) {return;}
    analogWrite(LED, 177);
    digitalWrite(ENABLE, LOW);
  } 

  void off() {
    if (constructed == false) {return;}
    digitalWrite(LED, LOW);
    digitalWrite(ENABLE, HIGH);
  } 

  bool command(String payload) {
    payload.toUpperCase();
    if (payload == "OFF") {
      off();
    } else if (payload == "ON") {
      on();    
    } else if (payload == "STOP") {
      stop();
    } else if (payload == "FORWARD") {
      forward();
    } else if (payload == "BACKWARD") {
      backward();
    } else {
      return false;
    }
    return true;
  }

  String state() {
    if (digitalRead(ENABLE) == HIGH) {
      return "OFF";
    } else if (running == false) {
      return "STOP";
    } else if (clockwise == true) {
      return "FORWARD";
    } else {
      return "BACKWARD";
    }
  }

};
