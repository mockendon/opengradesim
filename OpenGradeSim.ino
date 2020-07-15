/*
  OpenGradeSimulator by Matt Ockendon 2019.11.14
  Modified by Brian Palmer 2020.4.6 - updated motor routine to use PID machine and added automatic trainer incline leveler
  ____                  _____               __      ____ ____ __  ___
  / __ \ ___  ___  ___  / ___/____ ___ _ ___/ /___  / __//  _//  |/  /
  / /_/ // _ \/ -_)/ _ \/ (_ // __// _ `// _  // -_)_\ \ _/ / / /|_/ /
  \____// .__/\__//_//_/\___//_/   \_,_/ \_,_/ \__//___//___//_/  /_/
   /_/
  This is the controller for a 3D printed elevation or 'grade' simulator to use with an indoor trainer
  The project in inspired by the Wahoo Kickr Climb but shares none of its underpinnings.
  Elevation is simulated on an indoor trainer by increasing resistance over that generated by frictional
  losses.
  I found the equation of a best fit line from points plotted using an online calculator of frictional losses vs speed
  and then took the residual power to calculate the incline being simulated
  Rather than using a servo linear actuator (expensive) I'm using the Arduino Nano 33 IoT BLE's built in
  accelerometers to find the position of the bicycle. This method is prone to noise and I have tried
  some filtering (moving average) to reduce this.
  The circuit:
  Arduino Nano 33 BLE
  3.3 to 5v level shifter
  L298N H bridge
  750Newton 200mm Linear Actuator
  1x2 pushbutton pad
  128x32 I2C OLED display
  3D printed parts and boxes
  At present a NPE CABLE ANT+ to BLE bridge is required
  (due to the lack of authentication in the
  AdruinoBLE library 1.1.2)
  This code is in the public domain.
  Uses the moving average filter of sebnil https://github.com/sebnil/Moving-Avarage-Filter--Arduino-Library-
  and the Flash Storage library https://github.com/sebnil/Moving-Avarage-Filter--Arduino-Library-
  // NANO 33 IoT
*/

#include <FlashStorage.h>
#include <MovingAverageFilter.h>
#include <ArduinoBLE.h>
#include <Arduino_LSM6DS3.h>
#include <Sabertooth.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PID_v1.h>
#include "Defines.h"
#include "PushButton.h"
#include "OLEDDisplay.h"'
#include "MyMenu.h"

Sabertooth ST(128); // The Sabertooth is on address 128.

// Declare our filters

MovingAverageFilter movingAverageFilter_x(9);        //
MovingAverageFilter movingAverageFilter_y(9);        // Moving average filters for the accelerometers
MovingAverageFilter movingAverageFilter_z(9);        //
MovingAverageFilter movingAverageFilter_power(8);    // 2 second power average at 4 samples per sec
MovingAverageFilter movingAverageFilter_speed(2);    // 0.5 second speed average at 4 samples per sec

// For incline declare some variables and set some default values

long previousMillis = 0;          // last time in ms
long weightPrevMillis = 0;        // last time for weight setting
long weightMillis = 5000;         // time for weight setting
long actuatorMillis = 0;          // time for moving the actuator
float smoothRadPitch = 0;         // variable for the pitch
int trainerIncline = 0;           // variable for the % trainerIncline (actual per accelerometers)
double trainerInclineZeroAdj = 0; // inline adjustment from auto zero trainer incline
double targetGrade = 0;           // variable for the calculated grade (aim)
double manualTargetGrade = 0;
bool controllerLeveled = false;

MovingAverageFilter movingAverageFilter_Kp(8);    // 2 second power average at 4 samples per sec
MovingAverageFilter movingAverageFilter_Ki(8);    // 2 second power average at 4 samples per sec
MovingAverageFilter movingAverageFilter_Kd(8);    // 2 second power average at 4 samples per sec

double Kp_avg = 0.0, Ki_avg = 0, Kd_avg = 0.0;

// motor pid params

double Kp = 0.0;                              // PID proportional control Gain
double Ki = 0.00;
double Kd = 0.0;
bool pidParmsIsSet = false;
double trainerInclineErr = 0;
double motorPWM = 0;
double prev_SaberSpeed = 0;
//PID       (&Input,             &Output,   &Setpoint,    Kp, Ki, Kd, Direction, Mode)
PID motorPID(&trainerInclineErr, &motorPWM, &targetGrade, Kp_avg, Ki_avg, Kd_avg, DIRECT);

// user settings

typedef struct {
  boolean valid;
  int riderWeight;
  int wheelCircCM;
} UserSettings;

UserSettings userSettings;

// Reserve a portion of flash memory to store a "UserSetting" and
// call it "userSettings_FlashStore".
FlashStorage(userSettings_FlashStore, UserSettings);

int riderWeight = 113;            // default val combined rider and bike weight
int powerTrainer = 0;             // variable for the power (W) read from bluetooth
int speedTrainer = 0;             // variable for the speed (kph) read from bluetooth
float speedMpersec = 0;           // for calculation
float resistanceWatts = 0;        // for calculation
float powerMinusResistance = 0;   // for calculation

// For power and speed declare some variables and set some default values

int wheelCircCM = 2070;           // Default val for wheel circumference in centimeters (700c 32 road wheel) 2300
long WheelRevs1;                  // For speed data set 1
long Time_1;                      // For speed data set 1
long WheelRevs2;                  // For speed data set 2
long Time_2;                      // For speed data set 2
bool firstData = true;
int speedKMH;                     // Calculated speed in KM per Hr

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

// Our BLE peripheral and characteristics

BLEDevice cablePeripheral;
BLECharacteristic speedCharacteristic;
BLECharacteristic powerCharacteristic;

///////////////////////////////// Setup ///////////////////////////////////////

void setup() {
  Serial.begin(9600);
  delay(2000);
  // Init LED
  pinMode(redLedPin, OUTPUT);             // sets the digital pin as output
  pinMode(commonHighLedPin, OUTPUT);      // sets the digital pin as output
  pinMode(greenLedPin, OUTPUT);           // sets the digital pin as output
  digitalWrite(commonHighLedPin, HIGH);   // LED common is high.
  biColorLED(true, false);

  // init motor controller
  saberToothSetup();

  // setup a pin connected to RST (A5, pin 19) to pull reset low if reset is required
  pinMode (resetPin, OUTPUT);
  digitalWrite (resetPin, HIGH);

  Serial.begin(9600);

  initOLED();

  // Check that the accelerometer is up and running else reset
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    resetSystem();
  }
  // else {
  //    // auto zero trainer incline
  //    autoLevelTrainerIncline();
  //  }

  //turn the motor PID controller on
  motorPID.SetMode(AUTOMATIC);
  //motorPID.SetOutputLimits(0, 255); // This is the default range. Try making this -127, 127 and get rid of the contraint and mapping
  motorPID.SetOutputLimits(0, 127); // This is the default range. Try making this -127, 127 and get rid of the contraint and mapping

  // begin BLE initialization reset if fails
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    //    displayLineLeft(0, 21, 0, F("BLE failed!"));
    //    doDisplay();
    resetSystem();
  }

  initUserSettings();

  biColorLED(true, true); // red to green
  myMenu.setCurrentMenu(&list1);
}

////////////////////////////////  loop  ///////////////////////////////////////

bool debugging = false;

void loop() {

  long currentMillis = millis();

  if (currentMillis - previousMillis >= 100)
  {
    previousMillis = currentMillis;
    checkButtons();
    myMenu.doMenu();
    doDisplay();
  }
}

////////////////////////   method declarations  ///////////////////////////////

bool inManualMode = false;

boolean gradeSim() {
  static long previousSpeedandPowerMillis = 0;
  static bool firstTime = true;

  if (!controllerLeveled) {
    autoLevelTrainerIncline();
    return false;
  }

  Serial.print("inManualMode: "); Serial.print(inManualMode);
  Serial.print(", selectBtnPressed: "); Serial.print(selectBtnPressed());
  Serial.print(", upDownBtnPressed: "); Serial.print(upDownBtnPressed());
  Serial.print(", btnPressed:"); Serial.print(getbtnPressed());
  Serial.print(", firstTime:"); Serial.print(firstTime);
  Serial.println("");

  if (inManualMode)
  {
    setDouble(targetGrade, 1, 45, 1); // check for manual changes in grade

    if (selectBtnPressed())
    {
      inManualMode = false;
      return false;
    }
  } else {
    if (firstTime)
    {
      firstTime = false;
      return false; // skipping first cycle to ignore the selectBtnPressed that got us here.
    }
    // smart trainer mode
    if (selectBtnPressed())
    {
      Serial.println("exiting gradeSim");
      firstTime = true; // reset for next entry
      return true; // exit gradeSim loop
    } else {
      if (upDownBtnPressed()) // switching to manual mode
      {
        inManualMode = true;
        return false;
      }
    }

    if (debugging)
    {
      powerTrainer = movingAverageFilter_power.process(210);
      speedTrainer = movingAverageFilter_speed.process(15);
    } else {
      if (!cablePeripheral.connected())
      {
        getBLEServices(); // BLE setup
      }
      // fetch new speed and power data from trainer 5 times a second.
      long currentMillis = millis();
      if (currentMillis - previousSpeedandPowerMillis >= 200)
      {
        previousSpeedandPowerMillis = currentMillis;
        refreshSpeedandPower(); // Get any updated data
      }
    }
    calculateTargetGrade(); // Use power and speed to calculate the targetGrade
  }

  findTrainerIncline(true); // read the accelerometer to find auto-level adjusted trainerIncline
  trainerInclineErr = -abs(targetGrade - trainerIncline); // make err negative if it isnt already.

  //Serial.print(" inclineErr:");
  //Serial.println(trainerInclineErr);


  getPIDSettings();
  motorPID.Compute(); // Use target angle and trainer angle to calc the motor pwm value.

  // Display the current data
  gradeSimDisplay();

  // Update the actuator positon only if the trainer is in use
  if ((powerTrainer > 40) && (speedTrainer > 5))
  {
    moveActuator();
  }

}

void gradeSimDisplay()
{
  displayLineLeft(0, 20, 1, " "); // erase line 0
  displayLineLeft(1, 20, 1, " "); // erase line 1
  displayLineLeft(2, 20, 1, " "); // erase line 2

  // --   row 1 --
  char buf[7];
  sprintf_P(buf, PSTR("%d W"), powerTrainer); // Display power in watts top left

  //displayTextLeft( row,  rowPos,  startcol,  colwidth,  textsize, message )
  displayTextLeft (0, 0, 0, 5, 1, buf);

  // Display speed top right if more than 4kph
  if (speedTrainer > 4)
  {
    sprintf_P(buf, PSTR("%d kph"), speedTrainer);
    displayTextRight(0, 0, 20, 7, 1, buf);
  } else {
    displayTextRight(0, 0, 20, 7, 1, "-- kpm");
  }

  // --   row 2 --
  //sprintf_P(buf, PSTR("%.2d%%"), trainerIncline); //  Display current trainerIncline centred and 2X-scale text
  sprintf_P(buf, PSTR("%d%%"), trainerIncline); //  Display current trainerIncline centred and 2X-scale text
  displayTextRight (1, 9, 6, 7, 2, buf);

  // --   row 3 --
  sprintf_P(buf, PSTR("%d kg"), riderWeight); // Display weight bottom left
  //sprintf_P(buf, PSTR("%d pwm"), motorPWM); // Display motor PWM bottom left
  //displayTextLeft( row,  rowPos,  startcol,  colwidth,  textsize, message )
  displayTextLeft (2, 24, 0, 6, 1, buf);

  // Display manual or smart trainer incline bottom right
  if (inManualMode) {
    sprintf_P(buf, PSTR("Manual %.2g%%"), targetGrade); // Display targetGrade bottom right
  } else {
    sprintf_P(buf, PSTR("Trainer %.2g%%"), targetGrade); // Display targetGrade bottom right
  }
  //void displayTextRight( row, rowPos, startcol, colwidth, textsize,  message)
  displayTextRight(2, 24, 20, 13, 1, buf);

}
void getBLEServices() {

  displayLineLeft(1, 12, 0, F("Bluetooth scanning"));
  displayLineLeft(2, 24, 1, F("for ((CABLE)) Device"));
  //displayLineLeft(2, 24, 2, F(" ")); // erase the unused line
  doDisplay();

  // entering blocking code
  while (!cablePeripheral.connected()) {
    Serial.println("BLE Central");
    Serial.println("Turn on trainer and CABLE module and check batteries");

    // Scan or rescan for BLE services
    BLE.scan();

    // check if a peripheral has been discovered and allocate it
    cablePeripheral = BLE.available();

    if (cablePeripheral) {
      // discovered a peripheral, print out address, local name, and advertised service
      Serial.print("Found ");
      Serial.print(cablePeripheral.address());
      Serial.print(" '");
      Serial.print(cablePeripheral.localName());
      Serial.print("' ");
      Serial.print(cablePeripheral.advertisedServiceUuid());
      Serial.println();

      if (cablePeripheral.localName() == ">CABLE") {
        // stop scanning
        BLE.stopScan();
        Serial.println("got CABLE device. scan stopped");

        // subscribe to BLE speed and power
        getsubscribedtoSensor(cablePeripheral);

      }
    }
    delay(200);
  } // end while
}

void getsubscribedtoSensor(BLEDevice cablePeripheral) {
  //   connect to the peripheral
  Serial.println("Connecting ...");
  if (cablePeripheral.connect()) {
    Serial.println("Connected");

  } else {
    Serial.println("Failed to connect to CABLE device");
    return;
  }

  // discover Cycle Speed and Cadence attributes
  Serial.println("Discovering Cycle Speed and Cadence service ...");
  if (cablePeripheral.discoverService("1816")) {
    Serial.println("Cycle Speed and Cadence Service discovered");
  } else {
    Serial.println("Cycle Speed and Cadence Attribute discovery failed.");
    cablePeripheral.disconnect();

    resetSystem();
    return;
  }

  // discover Cycle Power attributes
  Serial.println("Discovering Cycle Power service ...");
  if (cablePeripheral.discoverService("1818")) {
    Serial.println("Cycle Power Service discovered");
  } else {
    Serial.println("Cycle Power Attribute discovery failed.");
    cablePeripheral.disconnect();

    resetSystem();
    return;
  }

  // retrieve the characteristics

  speedCharacteristic = cablePeripheral.characteristic("2a5B");
  powerCharacteristic = cablePeripheral.characteristic("2a63");

  // subscribe to the characteristics (note authentication not supported on ArduinoBLE library v1.1.2)

  if (!speedCharacteristic.subscribe()) {
    Serial.println("can not subscribe to speed");
  } else {
    Serial.println("subscribed to speed");
  };

  if (!powerCharacteristic.subscribe()) {
    Serial.println("can not subscribe to speed and power");

    //    // outcome display on OLED
    //    OLED.setTextSize(1);
    //    OLED.setTextColor(SSD1306_WHITE);
    //    OLED.setCursor(5, 10);
    //    OLED.print(F("Subscribe FAILED"));
    //    OLED.setCursor(5, 20);
    //    OLED.print(F("Speed and Power"));
    //    OLED.display();
    //    OLED.clearDisplay();

    delay(5000);
    resetSystem();

  } else {
    Serial.println("subscribed to speed and power");

    //    // outcome display on OLED
    //    OLED.setTextSize(1);
    //    OLED.setTextColor(SSD1306_WHITE);
    //    OLED.setCursor(5, 10);
    //    OLED.print(F("Subscribed to"));
    //    OLED.setCursor(5, 20);
    //    OLED.print(F("Speed and Power"));
    //    OLED.display();
    //    OLED.clearDisplay();

  };

  //  The time consuming BLE setup is done.

}

void refreshSpeedandPower(void) {

  // Get updated power value
  if (powerCharacteristic.valueUpdated()) {

    // Define an array for the value
    uint8_t holdpowervalues[6] = {0, 0, 0, 0, 0, 0} ;

    // Read value into array

    powerCharacteristic.readValue(holdpowervalues, 6);

    // Power is returned as watts in location 2 and 3 (loc 0 and 1 is 8 bit flags)
    byte rawpowerValue2 = holdpowervalues[2];       // power least sig byte in HEX
    byte rawpowerValue3 = holdpowervalues[3];       // power most sig byte in HEX

    long rawpowerTotal = (rawpowerValue2 + (rawpowerValue3 * 256));

    Serial.print("Power: ");
    Serial.println(rawpowerTotal);

    // Use moving average filter to give '3s power'
    powerTrainer = movingAverageFilter_power.process(rawpowerTotal);

    //    Serial.print("rawpowerValue2: ");
    //    Serial.println(rawpowerValue2);
    //    Serial.print("rawpowerValue3: ");
    //    Serial.println(rawpowerValue3);

  }

  // Get speed - a bit more complication as the GATT specification calls for Cumulative Wheel Rotations and Time since wheel event
  // So we'll need to do some maths

  if (speedCharacteristic.valueUpdated()) {

    //  This value needs a 16 byte array
    uint8_t holdvalues[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ;

    //  But I'm only going to read the first 7
    speedCharacteristic.readValue(holdvalues, 7);

    byte rawValue0 = holdvalues[0];       // binary flags 8 bit int
    byte rawValue1 = holdvalues[1];       // revolutions least significant byte in HEX
    byte rawValue2 = holdvalues[2];       // revolutions next most significant byte in HEX
    byte rawValue3 = holdvalues[3];       // revolutions next most significant byte in HEX
    byte rawValue4 = holdvalues[4];       // revolutions most significant byte in HEX
    byte rawValue5 = holdvalues[5];       // time since last wheel event least sig byte in HEX
    byte rawValue6 = holdvalues[6];       // time since last wheel event most sig byte in HEX

    if (firstData) {
      // Get cumulative wheel revolutions as little endian hex in loc 2,3 and 4 (least significant octet first)
      WheelRevs1 = (rawValue1 + (rawValue2 * 256) + (rawValue3 * 65536) + (rawValue4 * 16777216));
      // Get time since last wheel event in 1024ths of a second
      Time_1 = (rawValue5 + (rawValue6 * 256));

      firstData = false;

    } else {

      // Get second set of data
      long WheelRevsTemp = (rawValue1 + (rawValue2 * 256) + (rawValue3 * 65536) + (rawValue4 * 16777216));
      long TimeTemp = (rawValue5 + (rawValue6 * 256));

      if (WheelRevsTemp > WheelRevs1) {           // make sure the bicycle is moving
        WheelRevs2 = WheelRevsTemp;
        Time_2 = TimeTemp;
        firstData = true;

        // Find distance difference in cm and convert to km
        float distanceTravelled = ((WheelRevs2 - WheelRevs1) * wheelCircCM);
        float kmTravelled = distanceTravelled / 1000000;

        // Find time in 1024ths of a second and convert to hours
        float timeDifference = (Time_2 - Time_1);
        float timeSecs = timeDifference / 1024;
        float timeHrs = timeSecs / 3600;

        // Find speed kmh
        speedKMH = (kmTravelled / timeHrs);

        //Serial.print("  speed: ");
        //Serial.println(speedKMH, DEC);

        // Reject zero values
        if (speedKMH < 0) {} else {
          speedTrainer = movingAverageFilter_speed.process(speedKMH);  // use moving average filter to find 3s average speed
          // speedTrainer =  speedKMH;               // redundant step to allow experiments with filters
        }
      }
    }

  }

}

bool autoLevelTrainerIncline() {
  /* Samples the controler head grade n times and saves it as a correction offset to be applied when calculating bike grade.
     Press any key to exit without updating. Could be modified to give up auto-level and accept current offset. */


  Serial.print("auto leveling trainer.");
  static int sampleTimes = 0;
  static int previousSample = 0;
  const int n = 30;
  findTrainerIncline(false);

  switch (sampleTimes)
  {
    case 0:
      previousSample = trainerIncline;
    default:

      if (trainerIncline == previousSample)
      {
        sampleTimes++;
      } else {
        sampleTimes = 0; // start over
      }
      previousSample = trainerIncline;

      char buf[20];
      //sprintf_P(buf, PSTR("offset:%d%%"), trainerIncline);
      sprintf_P(buf, PSTR("hold still..."), trainerIncline);
      displayLineLeft(1, 12, 1, buf);
      displayLineLeft(2, 24, 1, " "); // erase the unused line

      bargraph(0, 27, sampleTimes * (128 / n), 6);
      //return false;
      return pressAnyButtonToExit();
      break;
    case n: // must be same angle n times in a row to auto-stop
      trainerInclineZeroAdj = trainerIncline; // record the adjustment
      sampleTimes = 0; // reset for next time.
      controllerLeveled = true;
      return true;
      break;
  }

}

void findTrainerIncline(bool adjusted) {
  float rawx, rawy, rawz;
  float x, y, z;

  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(rawx, rawy, rawz);

    x = movingAverageFilter_x.process(rawx);      //
    y = movingAverageFilter_y.process(rawy);      //   Apply moving average filters to reduce noise
    z = movingAverageFilter_z.process(rawz);      //

    //    char buf[80];
    //    sprintf_P(buf, PSTR("IMU x:%d y:%d z:%d"), x, y, z);

    // find pitch in radians
    float radpitch = atan2(( y) , sqrt(x * x + z * z));

    smoothRadPitch = radpitch;

    // find the % grade from the pitch
    trainerIncline = tan(smoothRadPitch) * 100;

    trainerIncline = trainerIncline * -1; // flip the sign since its mounted with the USB port on the left.

    if (adjusted == true)
    {
      trainerIncline = trainerIncline - trainerInclineZeroAdj;
    }
    //sprintf_P(buf, PSTR("IMU trainer incline:%d"), trainerIncline);
    //Serial.println(buf);
  }
}

bool resetSystem(void) {
  Serial.println("Resetting System");
  digitalWrite (19, LOW);
  return true;
}


bool setWeight(void)
{
  if (setNumber(riderWeight, 1, 1000, 1))
  {
    updateUserSettings();
    return true;
  } else {
    displayNumber(riderWeight, F(" kg"));
    return false;
  }
}

bool setWheelSize(void)
{
  if (setNumber(wheelCircCM, 1, 9999, 1))
  {
    updateUserSettings();
    return true;
  } else {
    displayNumber(wheelCircCM, F(" cm"));
    return false;
  }
}

bool setGrade(void)
{
  setDouble(targetGrade, 1, 45, 1);
  displayDouble(targetGrade, F("%"));
}

boolean startPhoneySpeedPower()
{
  debugging = true;
  return true;
}

boolean stopPhoneySpeedPower()
{
  debugging = false;
  return true;
}

void saberToothSetup()
{
  Sabertooth ST(128); // default address 128
  SabertoothTXPinSerial.begin(9600); // 9600 is the default baud rate for Sabertooth packet serial.
  ST.autobaud(); // Send the autobaud command to the Sabertooth controller(s).
}

void moveActuator(void)
{
  int SaberSpeed = motorPWM;
  //int pwm = constrain(motorPWM, 0, 255);
  //int SaberSpeed = map(pwm, 0, 255, 0, 127); // mapping default pid pwm speeds to SaberTooth SimpleSerial cmds (1 - 127)
  if (SaberSpeed != prev_SaberSpeed) {
    prev_SaberSpeed = SaberSpeed;

    //    if (abs(SaberSpeed)  < 50) {
    //      analogWrite(PWM_M1, 0); // turn off motors
    //    }

    if (trainerIncline < targetGrade) {
      SaberSpeed = -abs(SaberSpeed); // flip direction if trainer is below target
    }
    ST.motor(1, SaberSpeed);
  }

  //Serial.print("SaberSpeed: ");
  //Serial.print(SaberSpeed);
}

//void moveActuator(void) {
//
//  if (motorPWM != prev_motorPWM) {
//    prev_motorPWM = motorPWM;
//    if (abs(motorPWM)  < 50) {
//      // turn off motors
//      digitalWrite(actuatorOutPin1, LOW);
//      digitalWrite(actuatorOutPin2, LOW);
//    }
//    else if (trainerIncline < targetGrade) { // trainer below target. move up.
//      digitalWrite(actuatorOutPin1, LOW);
//      digitalWrite(actuatorOutPin2, HIGH);
//    } else {                                // trainer above target. move down.
//      digitalWrite(actuatorOutPin1, HIGH);
//      digitalWrite(actuatorOutPin2, LOW);
//    }
//
//    analogWrite(enA, motorPWM);
//
//  }
//}


void calculateTargetGrade(void) {
  float speed28 = pow(speedTrainer, 2.8);                                             // pow() needed to raise y^x where x is decimal
  resistanceWatts = (0.0102 * speed28) + 9.428;                                       // calculate power from rolling / wind resistance
  powerMinusResistance = powerTrainer - resistanceWatts;                              // find power from climbing
  //Serial.print("powerMinusResistance:");
  //Serial.print(powerMinusResistance);

  speedMpersec = speedTrainer / 3.6;                                                  // find speed in SI units. 1 meter / second (m/s) is equal 3.6 kilometers / hour (km/h)
  if (speedMpersec == 0)
  {
    targetGrade = 0;
  }
  else
  {
    targetGrade = ((powerMinusResistance / (riderWeight * 9.8)) / speedMpersec) * 100; // calculate grade of climb in %
  }

  // Limit upper and lower grades
  if (targetGrade < -10) {
    targetGrade = -10;
  }
  if (targetGrade > 20) {
    targetGrade = 20;
  }
}

boolean setPIDParms()
{
  getPIDSettings();
  displayPIDParmVals();
  return pressAnyButtonToExit();
}

void displayPIDParmVals(void) {
  // Display PID values
  // TODO: Add ability to modify values and save to user settings.
  char buf[20];
  sprintf_P(buf, PSTR("%.2g, %.2g, %.2g"), Kp_avg, Ki_avg, Kd_avg);
  displayLineLeft(1, 12, 1, buf);
  displayLineLeft(2, 24, 1, " "); // erase the unused lines
}

void getPIDSettings() {
  // get PID setting from POTS and average them.
  Kp = analogRead(A3) * 0.004;     // Serial.print("  Kp = "); Serial.print(kp);
  Kp_avg = movingAverageFilter_Kp.process(Kp); // Serial.print("  Kp_avg = "); Serial.print(Kp_avg);

  Ki = analogRead(A2) * 0.0005;  // Serial.print("  Ki = "); Serial.print(ki);
  Ki_avg = movingAverageFilter_Ki.process(Ki); //Serial.print("  Ki_avg = "); Serial.print(Ki_avg);

  Kd = analogRead(A1) * .001;     // Serial.print("  Kd = "); Serial.print(kd);
  Kd_avg = movingAverageFilter_Kd.process(Kd);  //Serial.print("  Kd_avg = "); Serial.println(Kd_avg);
  motorPID.SetTunings(Kp_avg, Ki_avg, Kd_avg);
  //  Serial.println(Kp_avg);
  //  Serial.println(Ki_avg);
  //  Serial.println(Kd_avg);
}

void initUserSettings()
{
  // Read or initialize the content of "userSettings_FlashStore"
  userSettings = userSettings_FlashStore.read();

  // Intitialize flash store with default values the first time.
  if (userSettings.valid == false) {
    Serial.println("Initializing User Settings Flash Storage");
    updateUserSettings();
    return;
  }

  riderWeight = userSettings.riderWeight;
  wheelCircCM = userSettings.wheelCircCM;
}

void updateUserSettings()
{
  userSettings.riderWeight = riderWeight;
  userSettings.wheelCircCM = wheelCircCM;
  userSettings.valid = true;
  userSettings_FlashStore.write(userSettings);
}

void biColorLED(bool on, bool color1) {

  if (on) {
    if (color1)
    {
      // flip led to green
      digitalWrite(greenLedPin, LOW);    //LED green ON
      digitalWrite(redLedPin, HIGH);     //LED RED off
    } else {
      // flip led to red
      digitalWrite(greenLedPin, HIGH);   //LED green off
      digitalWrite(redLedPin, LOW);      //LED RED on
    }
  } else {
    digitalWrite(redLedPin, HIGH);        //LED RED off
  }
}
