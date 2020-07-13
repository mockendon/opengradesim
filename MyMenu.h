
#include "Menu.h"
#include "defines.h"
#include "OLEDDisplay.h"
//
//int btnPushed = 0;
//
boolean setWeight();
boolean setWheelSize();
boolean setGrade();
boolean autoGrade();
boolean autoLevelTrainerIncline();
boolean setPIDParms();
boolean resetSystem();
boolean dimDisplay(); // brightness controll

boolean gotoMenuOne();
boolean gotoMenuTwo();
int getBtnPushed();

/*****************************
 ******************************
   Define the new derived menu class
   implement the functions to update
   the menu, make a selection, and
   display the menu
 ******************************
 *****************************/

class SimpleSerialMenu:
  public Menu {

  public:
    int updateSelection();
    boolean selectionMade();
    void displayMenu();

};

int SimpleSerialMenu::updateSelection() {
  if (getBtnPushed() == 2) {
    return 1;
  }
  if (getBtnPushed() == 1) {
    return -1;
  }
  return 0;
}

boolean SimpleSerialMenu::selectionMade() {
  // This is a simplified example
  // Edge detection and debouncing are exercises left to the user
  //return btnPushed == 3 ? true : false;
    return getBtnPushed() == 3 ? true : false;
}
//
//void SimpleSerialMenu::displayMenu() {
//
//  //  Blink Without Delay to keep prints from blasting out too fast.
//  static unsigned long preMil = millis();
//  unsigned long curMil = millis();
//  if(curMil - preMil >= 500) {  // print every half second
//    //Serial.print("btnPushed: "); Serial.println(btnPushed);
//    preMil = curMil;
//
//    char outBuf[17];
//
//    for (int i = 0; i < currentMenu->getSize(); i++) {
//
//      if (i == currentItemIndex) {
//        Serial.print(F("---> "));
//      }
//      else {
//        Serial.print(F("     "));
//      }
//      // use getText method to pull text out into a buffer you can print
//      getText(outBuf, i);
//      Serial.println(outBuf);
//    }
//    Serial.print(F("currentItemIndex =  "));
//    Serial.println(currentItemIndex);
//    for (int i = 0; i < 3; i++) {  // put some spaces before next print
//      Serial.println();
//    }
//  }
//}
void SimpleSerialMenu::displayMenu() {

  char outBuf[NUM_LCD_COLS + 1];

  int currentLine = 0;

  outBuf[0] = '-';
  outBuf[1] = '>';
  getText(outBuf + 2, currentItemIndex);
  displayLineLeft(currentLine++, 0, 1, outBuf);

  //Serial.println(outBuf);
  outBuf[0] = ' ';
  outBuf[1] = ' ';
  getText(outBuf + 2, currentItemIndex + 1);
  displayLineLeft(currentLine++, 12, 1, outBuf);

  getText(outBuf + 2, currentItemIndex + 2);
  displayLineLeft(currentLine++, 24, 1, outBuf);
  //Serial.println(outBuf);
}


/*****************************
 ******************************
   Setup the menu as an array of MenuItem
   Create a MenuList and an instance of your
   menu class
 ******************************
 *****************************/

MenuItem PROGMEM firstList[3] = {
    { "Manual", setGrade }
  , { "Smart Trainer", autoGrade }
  , { "Settings", gotoMenuTwo }
};
MenuItem PROGMEM secondList[7] = {
    { "Back", gotoMenuOne }
  , { "Set Weight", setWeight }
  , { "Set Wheel Size", setWheelSize }
  , { "Auto-level", autoLevelTrainerIncline }
  , { "Motor PID", setPIDParms }
  , { "Dim Display", dimDisplay }
  , { "Reset", resetSystem }
};

MenuList list1(firstList, 3);
MenuList list2(secondList, 7);

SimpleSerialMenu myMenu;

/*****************************
 ******************************
   Define the functions you want your menu to call
   They can be blocking or non-blocking
   They should take no arguments and return a boolean
   true if the function is finished and doesn't want to run again
   false if the function is not done and wants to be called again
 ******************************
 *****************************/
boolean gotoMenuOne() {
  myMenu.setCurrentMenu(&list1);
  return true;
}

boolean gotoMenuTwo() {
  Serial.println(F("loading menu two"));
  myMenu.setCurrentMenu(&list2);
  return true;
}
