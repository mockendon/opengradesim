
#ifndef OLEDDisplay_h
#define OLEDDisplay_h

#include "Arduino.h"
#include <Adafruit_SSD1306.h>
//#include "Defines.h"
#include "PushButton.h"

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 32     // OLED display height, in pixels

void initOLED();

void doDisplay();
void bargraph(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void displayLineLeft (int row, int rowPos, int textsize, const char* message);
void displayLineLeft (int row, int rowPos, int textsize, const __FlashStringHelper* message);

void displayLineRight(int row, int rowPos, int textsize, const char* message);
void displayLineRight(int row, int rowPos, int textsize, const __FlashStringHelper* message);

void displayTextLeft (int row, int rowPos, int startcol, int colwidth, int textsize, const char* message);
void displayTextLeft (int row, int rowPos, int startcol, int colwidth, int textsize, const __FlashStringHelper* message);

void displayTextRight(int row, int rowPos, int startcol, int colwidth, int textsize, const char* message);
void displayTextRight(int row, int rowPos, int startcol, int colwidth, int textsize, const __FlashStringHelper* message);

bool setNumber(int& val, int valMin, int valMax, int increment, const __FlashStringHelper* fmtStr);
bool setDouble(double& val, double valMin, double valMax, double increment, const __FlashStringHelper* fmtStr);
void checkButtons();
bool pressAnyButtonToExit();
void doCursor();
void setCursor(uint8_t, uint8_t);
void showCursor(boolean);
bool displayDelay(int);
#endif
