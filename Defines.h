/*
 * Defines.h
 */

#ifndef DEFINES_H_
#define DEFINES_H_

/*  ARDUINO NANO 33 IOT Pin Usage */
#define NO_BTN_PRESS          0
#define DECLINE_BTN           1
#define INCLINE_BTN           2
#define SELECT_BTN            3
#define RED_LED_PIN           8  // bi-color LED connected to digital pin
#define COMMON_HIGH_LED_PIN   9  // bi-color LED connected to digital pin
#define GREEN_LED_PIN        10  // bi-color LED connected to digital pin
#define RESET_PIN            20 // (A6, pin 20) is used to pull the arduino reset pin low allowing software to do hard resets


// oled display has 64 rows x 128 cols
#define NUM_LCD_ROWS 3
#define NUM_LCD_COLS 21

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 32     // OLED display height, in pixels

#define DISPLAY_DELAY 500

#define WAIT_FOR_ACTUATOR_STOP_MIL 4000 //  seconds to wait for linear actuator to reach bottom

#define BUTTON_DEBOUNCE 60
#define BUTTON_LONG_PRESS_TIME 2000

#define VERSION_NUMBER 1.01

#endif /* DEFINES_H_ */
