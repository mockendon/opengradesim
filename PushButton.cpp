#include "Arduino.h"
#include "PushButton.h"

PushButton::PushButton(uint8_t pin, uint8_t id)
  : pin(pin), id(id)
{ // remember the push button pin
  pinMode(pin, INPUT_PULLUP); // enable the internal pull-up resistor
  //digitalWrite(pin, LOW);
}

int PushButton::getId() {
  return id;
}

bool PushButton::isPressed() // read the button state check if the button has been pressed, debounce the button as well
{
  bool pressed = false;
  bool state = digitalRead(pin);               // read the button's state
  int8_t stateChange = state - previousState;  // calculate the state change since last time

  if (state == LOW) { // If the button is currently pressed
    if (millis() - previousBounceTime > debounceTime) { // check if the time since the last bounce is higher than the threshold
      pressed = true; // the button is pressed
    }
  }

  if (state == HIGH) { // if the button is released or bounces
    previousBounceTime = millis(); // remember when this happened
  }

  previousState = state; // remember the current state
  return pressed; // return true if the button was pressed and didn't bounce
};

bool PushButton::isOn()
{
  bool pressed = false;
  bool state = digitalRead(pin);               // read the button's state
  int8_t stateChange = state - previousState;  // calculate the state change since last time

  if (stateChange == falling) { // If the button is pressed (went from high to low)
    if (millis() - previousBounceTime > debounceTime) { // check if the time since the last bounce is higher than the threshold
      pressed = true; // the button is pressed
    }
  }

  if (stateChange == rising) { // if the button is released or bounces
    previousBounceTime = millis(); // remember when this happened
  }

  previousState = state; // remember the current state
  return pressed; // return true if the button was pressed and didn't bounce
}
