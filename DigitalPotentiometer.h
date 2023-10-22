#include <X9C.h> //digital potentiometer

class DigitalPotentiometer {

private:
  X9C dpot;
  int thisResistance = 57;

public:
  bool constructed;
  bool changed;

  DigitalPotentiometer() {}

  void begin() {
    dpot.begin(D5, D6, D7);
    dpot.setPot(thisResistance);
    constructed = true;
  }

  int resistance(int value = -1) {
    changed = false;
    if(value > 0 and value < 100 and value != thisResistance) {
      thisResistance = value;
      if (constructed == true) {
        dpot.setPot(value);
        changed = true;
      }
    }
    return thisResistance;
  }

};
