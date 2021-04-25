#include "OLEDDisplay.h"
#include "Defines.h"

#ifdef USING_SERIAL
extern HardwareSerial Serial;
#endif
#define OLED_RESET     -1         // No reset pin on cheap OLED display
Adafruit_SSD1306 *OLED;
char lineBuffer[NUM_LCD_ROWS][NUM_LCD_COLS + 1]; // Leave an extra space for terminating null
int lineSize[NUM_LCD_ROWS];
int linePosition[NUM_LCD_ROWS];
boolean cursorActive = false;
uint8_t cursorRow;
uint8_t cursorCol;
uint8_t graph_X;
uint8_t graph_Y;
uint8_t graph_Width;
uint8_t graph_Height;
bool display_Graph = false;

static bool valIsSet = false;

// 4 - button pad
//PushButton btn1 = { 5 };
//PushButton btn2 = { 4 };
//PushButton btn3 = { 7 };
//PushButton btn4 = { 6 };

// 3-button pad
PushButton btn1 = { 4, 1 };
PushButton btn2 = { 5, 2 };
PushButton btn3 = { 6, 3 };
int btnPressed = 0;

// Custom Char Bluetooth Logo

byte customChar[] = {
  B00000,
  B00110,
  B00101,
  B10110,
  B01100,
  B10110,
  B00101,
  B00110
};

//display.setTextColor(BLACK, WHITE); // 'inverted' text
//Characters are rendered in the ratio of 7:10. Meaning, passing font size 1 will render the text at 7×10 pixels per character, passing 2 will render the text at 14×20 pixels per character and so on.
void initOLED()
{

#ifdef USING_SERIAL
  Serial.begin(19200);
#else
  OLED = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

  if (!OLED->begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    //resetSys(F("SSD1306 allocation failed"));
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with a splash screen (edit the splash.h in the library).

  OLED->setRotation(0);
  //OLED.display();
  //delay(1000);                    // Pause for 1 seconds
  OLED->clearDisplay();

  // Show the firmware version

  OLED->setTextSize(1);
  OLED->setTextColor(SSD1306_WHITE);
  OLED->setCursor(5, 10);
  OLED->print(F("FW Version: "));
  OLED->print(VERSION_NUMBER);
  OLED->display();
  delay(500);                    // Pause for version # display
  OLED->clearDisplay();
#endif
}

void doCursor() {
  if (cursorActive) {
#ifdef USING_SERIAL
    Serial.print("Cursor at ");
    Serial.print(cursorRow);
    Serial.println(cursorCol);
#else
    //OLED->cursor();
    OLED->setCursor(cursorCol, cursorRow);
#endif
  }
  else {
#ifdef USING_SERIAL

#else
    // OLED->noCursor();
#endif
  }
}

void doDisplay() {
  //  static unsigned long preMil = millis();
  //  unsigned long curMil = millis();
  //  if(curMil - preMil >= DISPLAY_DELAY){
#ifdef USING_SERIAL
  Serial.print(F("0: "));
  Serial.print(lineBuffer[0]);
  Serial.println(F("\\"));
  Serial.print(F("1: "));
  Serial.print(lineBuffer[1]);
  Serial.print(F("\\ RAM = "));
  Serial.println(freeRam());
  Serial.println(F("**"));

  delay(100);
#else
  OLED->clearDisplay();
  for (int i = 0; i < NUM_LCD_ROWS; i++) {
    lineBuffer[i][NUM_LCD_COLS] = 0;   // Make sure the null is there.

    OLED->setCursor(0, linePosition[i]);
    OLED->setTextSize(lineSize[i]);
    OLED->print(lineBuffer[i]);
  }
  if (display_Graph) {
    OLED->fillRect(graph_X, graph_Y, graph_Width, graph_Height, WHITE);
    display_Graph = false;
  }
  //  Update the display
  OLED->display();
#endif
  //doCursor();
  //  }
}

bool setOLEDDimmer (bool dimVal) {
  OLED->dim(dimVal);
  return true;
}

void checkButtons(void) {

  if (btn1.isOn()) {
    btnPressed = btn1.getId();
    if (btn2.isPressed()) {
      btnPressed = 4;
    }
  } else if (btn2.isOn()) {
    btnPressed = btn2.getId();
    if (btn3.isPressed()) {
      btnPressed = 5;
    }
  } else if (btn3.isOn()) {
    btnPressed = btn3.getId();
  } else  {
    btnPressed = 0;
  }

  //Serial.print("btnPressed: "); Serial.println(btnPressed);
  // return btnPressed;
}

bool selectBtnPressed()
{
  return (btnPressed == SELECT_BTN) ? true: false;
}

bool upDownBtnPressed()
{
  return ((btnPressed == INCLINE_BTN) || (btnPressed == DECLINE_BTN)) ?  true: false;
}

bool pressAnyButtonToExit()
{
  // Loop control - Runs 3 times minimum. return false.
  // run #1: init static vars btnWasPushed and firstTime.
  // run #2 - #n: check for any button push. If btn was pushed, set reminder to exit on next run. return false.
  // last run: reset firstTime to True ready for next time and return true;

  static bool firstTime = true;
  static bool btnWasPushed = false;

  switch (firstTime)
  {
    case true:
      btnWasPushed = false;
      firstTime = false;
      break;
    case false:
      switch (btnWasPushed)
      {
        case false:
          btnWasPushed = btnPressed > 0 ? true : false;
          return false;
          break;
        case true:
          firstTime = true;
          break;
      }
      break;
  }
  return btnWasPushed;
}

int getbtnPressed() {
  return btnPressed;
}
void bargraph(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
  display_Graph = true;
  graph_X = x;
  graph_Y = y;
  graph_Width = w;
  graph_Height = h;

}

void displayTextLeft(int row, int rowPos, int startcol, int colwidth, int textsize, const char* message) {
  lineSize[row] = textsize;
  linePosition[row] = rowPos;
  int charIndx = 0;
  boolean over = false;
  for (int i = startcol; i < startcol + colwidth; i++) {
    if (!over) {
      char c = message[charIndx];
      if (c == 0) {
        over = true;
        lineBuffer[row][i] = ' ';
      } else {
        lineBuffer[row][i] = c;
      }
    } else {
      lineBuffer[row][i] = ' '; // fill with spaces to clear anything else on the line.
    }
    charIndx++;
  }
}

void displayTextLeft(int row, int rowPos, int startcol, int colwidth, int textsize, const __FlashStringHelper* message) {
  lineSize[row] = textsize;
  linePosition[row] = rowPos;
  boolean over = false;
  PGM_P p = reinterpret_cast<PGM_P>(message);
  for (int i = startcol; i < startcol + colwidth; i++) {
    if (!over) {
      char c = pgm_read_byte(&p[i]);
      if (c == 0) {
        over = true;
        lineBuffer[row][i] = ' ';
      }
      else {
        lineBuffer[row][i] = c;
      }
    }
    else {
      lineBuffer[row][i] = ' '; // fill with spaces to clear anything else on the line.
    }
  }
}

void displayTextRight(int row, int rowPos, int startcol, int colwidth, int textsize, const char* message) {
  lineSize[row] = textsize;
  linePosition[row] = rowPos;
  //Serial.println(message);
  int numChars = strlen(message);
  int numSpaces = 0;

  // limit colwidth if not enough room to the right.
  //  if (colwidth > NUM_LCD_COLS - startcol) {
  //    colwidth = NUM_LCD_COLS - startcol;
  //  }

  int charIndx = numChars - 1;
  for (int i = startcol; i > startcol - colwidth; i--) {
    if (charIndx < 0) {
      lineBuffer[row][i] = ' ';
    } else {
      lineBuffer[row][i] = message[charIndx];
    }
    charIndx--;
  }
}

void displayTextRight(int row, int rowPos, int startcol, int colwidth, int textsize, const __FlashStringHelper* message) {
  lineSize[row] = textsize;
  linePosition[row] = rowPos;
  int numChars = strlen_P((const char*)message);
  int numSpaces = 0;
  PGM_P p = reinterpret_cast<PGM_P>(message);

  if (colwidth > NUM_LCD_COLS - startcol) {
    colwidth = NUM_LCD_COLS - startcol;
  }
  numSpaces = colwidth - numChars;
  if (numSpaces < 0) {
    numSpaces = 0;
  }
  for (int i = startcol; i < startcol + colwidth; i++) {
    if (i < numSpaces) {
      lineBuffer[row][i] = ' ';
    }
    else {
      lineBuffer[row][i] = pgm_read_byte(&p[i - numSpaces]);
    }
  }
}

void displayLineLeft(int row, int rowPos, int textsize, const char* message) {
  //Serial.println(message);
  displayTextLeft(row, rowPos, 0, NUM_LCD_COLS, textsize, message);
}

void displayLineLeft(int row, int rowPos, int textsize, const __FlashStringHelper* message) {
  displayTextLeft(row, rowPos, 0, NUM_LCD_COLS, textsize, message);
}

void displayLineRight(int row, int rowPos, int textsize, const char* message) {
  displayTextRight(row, rowPos, 0, NUM_LCD_COLS, textsize, message);
}

void displayLineRight(int row, int rowPos, int textsize, const __FlashStringHelper* message) {
  displayTextRight(row, rowPos, 0, NUM_LCD_COLS, textsize, message);
}

void showCursor(boolean aBool) {
  cursorActive = aBool;
}

void setCursor(uint8_t arow, uint8_t acol) {
  cursorRow = arow;
  cursorCol = acol;
  cursorActive = true;
}

//  To be used as a timer by other functions on the menu
//  Can only have one active at a time.
boolean displayDelay(unsigned int delSeconds) {
  static uint8_t state = 0;
  static unsigned long startMil = millis();
  unsigned long curMil = millis();

  switch (state) {

    case 0: {
        startMil = millis();
        state++;
        return false;
      }
    case 1: {
        if (curMil - startMil >= delSeconds * 1000ul) {
          state = 0;
          return true;
        } else {
          return false;
        }
      }

  }  // end switch (state)
  return false;
}

void displayNumber(int val, const __FlashStringHelper* txt)
{
  char buf[20];
  sprintf_P(buf, PSTR("  %d%s"), val, F(txt));
  //sprintf_P(buf, fmtStr*, val, btnPressed);
  displayLineLeft(1, 12, 2, buf);
  displayLineLeft(2, 24, 1, " "); // erase the unused line
}

void displayDouble(double val, const __FlashStringHelper* txt)
{
  char buf[20];
  sprintf_P(buf, PSTR("  %g%s"), val, "%");
  displayLineLeft(1, 12, 2, buf);
  displayLineLeft(2, 24, 1, " "); // erase the unused lines
}

int adjustNumber(int val, int origVal, int valMin, int valMax, int increment)
{
  //todo: limit to min and max
  switch (btnPressed) {

    case 1:
      val = val - increment;
      break;
    case 2:
      val = val + increment;
      break;
    case 3:
      valIsSet = true;
      break;
    case 4: // pressing both btn 1 and 2 - restore to last saved weight
      val = origVal;
      break;
    default:
      break;
  }
  if (val < valMin)
      val = valMin;
  if (val > valMax)
      val = valMax;
  return val;
}

bool setNumber(int& val, int valMin, int valMax, int increment)
{
  // This will loop 3 times minumum.
  // run 1: set valIsSet to false.
  // run 2 - n:  run adjustNumber (one or more times).
  // last run: reset firstTime to True and return true to end setNumber.
  static int origVal = 0;
  static bool firstTime = true;

  switch (firstTime)
  {
    case true:
      origVal  = val;
      valIsSet = false;
      firstTime = false;
      break;
    case false:
      switch (valIsSet)
      {
        case false:
          val = adjustNumber( val, origVal, valMin, valMax, increment );
          break;
        case true:
          firstTime = true;
          break;
      }
      break;
  }
  return firstTime;
}

double adjustDouble(double val, double origVal, double valMin, double valMax, double increment)
{

  switch (btnPressed) {

    case 1:
      val = val - increment;
      break;
    case 2:
      val = val + increment;
      break;
    case 3:
      valIsSet = true;
      break;
    case 4: // pressing both btn 1 and 2 - restore to last saved weight
      val = origVal;
      break;
    default:
      break;
  }
  if (val < valMin)
      val = valMin;
  if (val > valMax)
      val = valMax;
  return val;
}

bool setDouble(double& val, double valMin, double valMax, double increment)
{
  // This will loop 3 times minumum.
  // run 1: set valIsSet to false.
  // run 2 - n:  run adjustNumber (one or more times).
  // last run: reset firstTime to True and return true to end setNumber.;
  static double origVal = 0;
  static bool firstTime = true;

  switch (firstTime)
  {
    case true:
      origVal = val;
      valIsSet = false;
      firstTime = false;
      break;
    case false:
      switch (valIsSet)
      {
        case false:
          val = adjustDouble( val, origVal, valMin, valMax, increment );
          break;
        case true:
          firstTime = true;
          break;
      }
      break;
  }
  return firstTime;
}
