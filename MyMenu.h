#include "Menu.h"
#include "defines.h"
#include "OLEDDisplay.h"
//

boolean setWeight();
boolean setWheelSize();
boolean settrainerErrSensitivity();
boolean setManualAdjPcnt();
boolean lowerTrainer();
boolean gradeSim();
boolean gradeSimWithLeveling();
boolean autoLevelTrainerIncline();
boolean setPIDParms();
boolean setP();
boolean setI();
boolean setD();
boolean undoPIDChanges();
boolean resetSystem();

boolean setOLEDDimOn();
boolean setOLEDDimOff();
boolean getOLEDDimMode();

boolean startDebuggingMode();
boolean stopDebuggingMode();
boolean getDebuggingMode();

boolean setUnitsImperial();
boolean setUnitsMetric();
int getDisplayUnits();


boolean setLevelingOff();
boolean setLevelingOn();
LevelingMode getLevelingMode();

boolean gotoMainMenu();
boolean gotoSettingsMenu();
boolean gotoDisplayMenu();
boolean gotoPIDMenu();
boolean gotoDebugMenu();
boolean gotoUnitsMenu();
boolean gotoLevelingMenu();
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
  return 0; // select btn
}

boolean SimpleSerialMenu::selectionMade() {
  //Serial.print("currentItemIndex:"); Serial.println(currentItemIndex);
  return getbtnPressed() == SELECT_BTN ? true : false;
}

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

MenuItem PROGMEM mainMenu[3] = {
  { "Ride", gradeSim }
  , { "Level/Ride", gradeSimWithLeveling }
  , { "Settings", gotoSettingsMenu }
};

MenuItem PROGMEM settingsMenu[12] = {
   // "....:....:...."  14 char max
  { "Units", gotoUnitsMenu }                        // 0
  , { "Bike+Rider wt", setWeight }                  // 1
  , { "Wheel Size", setWheelSize }                  // 2
  , { "Motor PID", gotoPIDMenu }                    // 3
  , { "Manual Step %", setManualAdjPcnt }           // 4
  , { "Grade Accuracy", settrainerErrSensitivity }  // 5
  , { "Leveling", gotoLevelingMenu }                // 6
  , { "Display", gotoDisplayMenu }                  // 7
  , { "Debugging", gotoDebugMenu }                  // 8
  , { "Reset", resetSystem }                        // 9
  , { "Lower Trainer", lowerTrainer }               // 10
  , { "<Back>", gotoMainMenu }                      // 11
};
const int menuUnitsOption=0;
const int menuLevelingOption=6;
const int menuDisplayOption=7;
const int menuDebugMenuOption=8;
const int menulowerTrainerOption=11;

MenuItem PROGMEM displayMenu[2] = {
  { "Dimmer", setOLEDDimOn }
  , { "Brighter", setOLEDDimOff }
};

MenuItem PROGMEM levelingMenu[2] = {
  { "First Time", setLevelingOff }
  , { "Every time", setLevelingOn }
};

MenuItem PROGMEM pidParmsMenu[5] = {
  { "Proportional", setP }
  , { "Integral", setI }
  , { "Derivative", setD }
  , { "Undo Changes", undoPIDChanges }                     
  , { "<Back>", gotoSettingsMenu }
};

MenuItem PROGMEM debugMenu[2] = {
  { "Start Debug", startDebuggingMode }
  , { "End Debug", stopDebuggingMode }
};

MenuItem PROGMEM unitsMenu[2] = { // display formatting
  { "Imperial", setUnitsImperial } // displayOption=0
  , { "Metric", setUnitsMetric } // displayOption=1
};

MenuList mainMenuList(mainMenu, 3);
MenuList settingsMenuList(settingsMenu, 12);
MenuList displayMenuList(displayMenu, 2);
MenuList pidParmsMenuList(pidParmsMenu, 5);
MenuList debugMenuList(debugMenu, 2);
MenuList unitsMenuList(unitsMenu, 2);
MenuList levelingMenuList(levelingMenu, 2);

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

boolean gotoDisplayMenu() {
  myMenu.setCurrentMenu(&displayMenuList);
  myMenu.currentItemIndex = getOLEDDimMode()?0:1; // 0 = dimmer off, 1 = dimmer on
  return true;
}

boolean gotoPIDMenu() {
    myMenu.setCurrentMenu(&pidParmsMenuList);
  return true;
}

boolean gotoDebugMenu() {
  myMenu.setCurrentMenu(&debugMenuList);
  myMenu.currentItemIndex = !getDebuggingMode()?0:1; // start option = 0, end option = 1
  return true;
}

boolean gotoUnitsMenu() {
  myMenu.setCurrentMenu(&unitsMenuList);
  myMenu.currentItemIndex = getDisplayUnits();
  return true;
}

boolean gotoLevelingMenu() {
  myMenu.setCurrentMenu(&levelingMenuList);
  myMenu.currentItemIndex = !getLevelingMode()?0:1;
  return true;
}
