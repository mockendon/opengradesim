/*
 * Menu.cpp
 *
 *  Created on: Jun 30, 2015
 *      Author: David
 */

#include "Menu.h"

MenuItem* MenuList::getItem(int aIndex) {
  // To make modulo work with negatives the way we want
  while (aIndex < 0) {
    aIndex += listSize;
  }
  return &(menuItems[aIndex % listSize]);
}

uint8_t MenuList::getSize() {
  return this->listSize;
}

Menu::Menu() {
  currentMenu = 0;
  currentItemIndex = 0;
  cancelFlag = false;
  runningFunction = false;
}

Menu::Menu(MenuList* aList) {
  currentMenu = aList;
  currentItemIndex = 0;
  cancelFlag = false;
  runningFunction = false;
}

boolean Menu::runFunction() {
  Item_Function theFunc;

#ifdef USING_PROGMEM
  theFunc =
      (Item_Function) pgm_read_word(&(currentMenu->getItem(currentItemIndex)->func));
#else
  theFunc = currentMenu->getItem(currentItemIndex)->func;

#endif
  return theFunc();

}

void Menu::doMenu() {
  cancelFlag = checkForCancel();
  if (runningFunction) {
    runningFunction = !runFunction();
  } else {
    //menuHandler();
    cancelFlag = false;
    int selUp = updateSelection();
    if (selUp) {
      currentItemIndex += selUp;
      //  Avoid negative numbers with the modulo
      //  to roll the menu over properly
      while (currentItemIndex < 0) {
        currentItemIndex += currentMenu->getSize();
      }
      currentItemIndex %= currentMenu->getSize();

    } else {   // only check for selection if menu not also updating
      if (selectionMade()) { // selection made, run item function once now
        if (!runFunction()) { // If item function returns false then it isn't done and we need to keep running it.
          runningFunction = true;
          return; // skip displaying the menu if we're going to run item function again.
        }
      }
    }
    displayMenu();
  }
}

void Menu::getText(char* aBuf, int aIndex) {
#ifdef USING_PROGMEM
  strcpy_P(aBuf, (currentMenu->getItem(aIndex)->text));
#else
  strcpy(aBuf, (currentMenu->getItem(aIndex)->text));
#endif
}

boolean Menu::checkForCancel() {
  return false;
}

void Menu::setCurrentMenu(MenuList* aMenu) {
  currentMenu = aMenu;
  currentItemIndex = 0;
}
