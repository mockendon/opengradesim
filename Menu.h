/*
 * Menu.h
 *
 *  Created on: Jun 30, 2015
 *      Author: David
 */

#ifndef MENU_H_
#define MENU_H_

#define USING_PROGMEM

#include "Arduino.h"

typedef boolean (*Item_Function)();

const typedef struct MenuItem_t {
  char text[16];
  Item_Function func;
} MenuItem;

class MenuList {
private:
  MenuItem *menuItems;
  uint8_t listSize;
public:
  MenuItem* getItem(int aIndex);
  MenuList(MenuItem* aList, uint8_t aSize) :
      menuItems(aList), listSize(aSize) {
  }
  uint8_t getSize();

};

class Menu {
private:

  boolean cancelFlag;
  boolean runningFunction;
public:
  MenuList *currentMenu;
  int currentItemIndex;

  Menu();
  Menu(MenuList* aList);

  void doMenu();
  void setCurrentMenu(MenuList*);
  boolean runFunction();
  void getText(char*, int);
  virtual void displayMenu() = 0;
//  static virtual void menuHandler();
  virtual boolean checkForCancel();
  virtual int updateSelection() = 0;
  virtual boolean selectionMade() = 0;

};

#endif /* MENU_H_ */
