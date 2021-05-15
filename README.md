# opengradesim
  Original Author: OpenGradeSimulator by Matt Ockendon 2019.11.14. See https://github.com/mockendon/opengradesim
  This branch by Brian Palmer 2020.4.6 at https://github.com/BrianFrankPalmer/opengradesim

  "This is the controller for a 3D printed elevation or 'grade' simulator to use with an indoor trainer.
  The project in inspired by the Wahoo Kickr Climb but shares none of its underpinnings.
  Elevation is simulated on an indoor trainer by increasing resistance over that generated by frictional
  losses." - Matt Odendon

  I was inspired to build Matt Odendon's cool opengradesim project and ended up coming up with this branch
  in the process. This version addresses a couple issues on his to-do list and adds a few other nicities.

  - A manual mode was added. Pressing the up or down button switches to manual mode. Pressing the stop button returns to smart trainer mode.
  - The blocking motor control routine was replaced with a PID controller. Now the loop() runs uninterrupted allowing the target grade
  to be constantly adjusted. This allows for accurate grade simulations (if your not peddling).
  - Peddle vibration injecting unwanted noise into the control loop was handled by locking the position once a target grade
  is achieved. It's only unlocked when the REQUESTED grade has changed by n% or more. The requested grade comes from either manual input,
  Matt's calculated grade routine,  or serial console input (+, -) during debugging/testing. But does not include the Nano's noisy sensor incline data.
  So you can now shift your weight, peddle, bang on your bike, etc. and the climber wont move unless the requested grade changes.
  - A bicolor LED was added. It turns red when the the actuator is moving and green when its locked on a target grade.
  - Now allows for negative grades.
  - Added a climber leveling mode. This mode both physically levels the bike to 0% incline and sets the controller grade display to 0%. 
  While in this mode, the up and down buttons are used adjust the bikes incline. Once you have leveled the trainer, pressing
  the stop button stores the current incline to flash memory. This value is used to automatically re-level
  the bike the next time gradeSim is started. So now you dont have to physically move the control head to level the climber and the control
  head can be mounted at an angle.

  - Added UI Setting menu that allows setting:
      Units: (imperial/metric)
      Rider/Bike Weight: combined bike/rider weight in kg or lbs
      Wheel Size: defaults to 2070 mm
      Manual Step %: Controls sensivity of manual incline/decline buttons.
      Grade Accuracy: Controls precision of grade adjustments. 
      Leveling Mode: Controls when the leveling mode is entered (each time GradeSim is started or first time only).
      PID parameters: All setting of motor control loop parameters.
      Debug Mode: When On, allows input through Serial console. 
  
  - Finished Matt's work on storage of user settings. This could easily be expanded store multiple rider/bike profiles.
  - Added a battery level display that displays the battery level of the <CABLE> device.
  - Added a IMU temperature sensing and display.  This can be used to ensure your arduino isnt overheatin in its case. This was achieved by 
  upgrading to the LSM6DS3 driver. See https://github.com/arduino-libraries/Arduino_LSM6DS3/issues/9. Uncomment the display lines to use.

  The circuit: This is basically like Matt's circuit except -
  - I use a bigger actuator that requires a bigger motor driver. After experementing with a few different boards I ended up using a
  Sabertooth 3x32 that I had from a previous project. They are pricey, but worth it. It can be controlled with 3V signal eliminating the
  need for a logic level shifter, can be controlled with just two wires, and COMPLETELY ELIMINATED THE MOTOR WHINE AT ALL SPEEDS.
  (Update - this was a bad idea. The sabertooth is really designed to be powered by a battery or a PSU. I couldnt get it to run for more than a
  few minutes off a wall wart.)
  - Uses a 3 button pad instead of 2.
  - Added a bi-color LED to indicate state.
  - Added 3 POTS for storing initial PID param values. I ended up not using these for anything because the PID values are stored in ROM and can
  be be adjusted in Settings -> PID values.
  - Uses a Serial cable connector between the control box and the motor driver. I had planned to swap this out for a coiled 6-pin RJ-12 cable.
  But after tripping over the serial cable a few times without causing any damage, I think I will stick with it.
