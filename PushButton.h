#ifndef PushButton_h
#define PushButton_h

#include "Arduino.h"
class PushButton {
  public:
    PushButton(uint8_t pin, uint8_t id); // Constructor (executes when a PushButton object is created)
    bool isPressed(); // read the button state check if the button has been pressed, debounce the button as well
    bool isOn(); // is being pressed
    int getId();
  private:
    uint8_t pin;
    uint8_t id = 0;
    bool previousState = HIGH;
    unsigned long previousBounceTime = 0;

    const static unsigned long debounceTime = 25;
    const static int8_t rising = HIGH - LOW;
    const static int8_t falling = LOW - HIGH;
};


#endif
