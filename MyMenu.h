#include "Menu.h"
#include "defines.h"
#include "OLEDDisplay.h"
//

boolean setWeight();
boolean setWheelSize();
boolean gradeSim();
boolean levelTrainer();
boolean autoLevelTrainerIncline();
boolean setPIDParms();
boolean setP();
boolean setI();
boolean setD();
boolean resetSystem();
boolean dimDisplay(); // brightness controll
boolean toggleOLEDDimOn();
boolean toggleOLEDDimOff();
boolean startPhoneySpeedPower();
boolean stopPhoneySpeedPower();

boolean gotoMainMenu();
boolean gotoSettingsMenu();
boolean gotoOnOffMenu();
boolean gotoPIDMenu();
boolean gotoDebugMenu();

int getbtnPressed();

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
  if (getbtnPressed() == INCLINE_BTN) {
    return 1;
  }
  if (getbtnPressed() == DECLINE_BTN) {
    return -1;
  }
  return 0;
}

boolean SimpleSerialMenu::selectionMade() {
  return getbtnPressed() == SELECT_BTN ? true : false;
}
//
//void SimpleSerialMenu::displayMenu() {
//
//  //  Blink Without Delay to keep prints from blasting out too fast.
//  static unsigned long preMil = millis();
//  unsigned long curMil = millis();
//  if(curMil - preMil >= 500) {  // print every half second
//    //Serial.print("btnPressed: "); Serial.println(btnPressed);
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

MenuItem PROGMEM mainMenu[2] = {
  { "Grade Sim", gradeSim }
  , { "Settings", gotoSettingsMenu }
};

MenuItem PROGMEM settingsMenu[8] = {
  { "Set Weight", setWeight }
  , { "Set Wheel Size", setWheelSize }
  , { "Auto-level", autoLevelTrainerIncline }
  , { "Motor PID", gotoPIDMenu }
  , { "Display", gotoOnOffMenu }
  , { "Reset", resetSystem }
  , { "Debug Mode", gotoDebugMenu }
  , { "<Back>", gotoMainMenu }
};

MenuItem PROGMEM onOffMenu[3] = {
  { "Dimmer", toggleOLEDDimOn }
  , { "Brighter", toggleOLEDDimOff }
  , { "<Back>", gotoSettingsMenu }
};

MenuItem PROGMEM pidParmsMenu[4] = {
  { "Proportional", setP }
  , { "Integral", setI }
  , { "Derivative", setD }
  , { "<Back>", gotoSettingsMenu }
};

MenuItem PROGMEM debugMenu[3] = {

  { "sim 15kpm,210W", startPhoneySpeedPower }
  , { "End sim", stopPhoneySpeedPower }
  , { "<Back>", gotoSettingsMenu }
};

MenuList mainMenuList(mainMenu, 2);
MenuList settingsMenuList(settingsMenu, 8);
MenuList onOffMenuList(onOffMenu, 3);
MenuList pidParmsMenuList(pidParmsMenu, 4);
MenuList debugMenuList(debugMenu, 3);

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
boolean gotoMainMenu() {
  myMenu.setCurrentMenu(&mainMenuList);
  return true;
}

boolean gotoSettingsMenu() {
  myMenu.setCurrentMenu(&settingsMenuList);
  return true;
}

boolean gotoOnOffMenu() {
  myMenu.setCurrentMenu(&onOffMenuList);
  return true;
}
boolean gotoPIDMenu() {
    myMenu.setCurrentMenu(&pidParmsMenuList);
  return true;
}
boolean gotoDebugMenu() {
  myMenu.setCurrentMenu(&debugMenuList);
  return true;
}
