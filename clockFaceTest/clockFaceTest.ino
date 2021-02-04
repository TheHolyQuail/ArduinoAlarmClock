//Arduino libraries
#include <Arduino.h>

// screen libraries
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//// screen definitions
// dimentions
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/// screen display arrays
// 0: no menu, 1: main menu, 2: set time (minutes), 3: set time (hours), 4: set alarm (time select screen), 5: set pomodoro timer (work time select screen), 6: set pomodoro timer (rest time select screen)
short int menuDisplay = 0;
// 0: no menu, 1: first option, 2: second option 3: third option
short int menuOptionHighlight = 0;
// false: no menu settings (left = menu, right = ____), true: in a menu or menu item (left = entr, right = back)
bool buttonDisplay = false;
// (Timer/Alarm window) 0: none, 1: timer, 2: alarm, 3: sounding alarm/timer
short int TAwindow = 0;

//// time declarations
//unsigned long currentMillis; decalred in loop
unsigned long previousMillis = 0;
unsigned long previousMillisSound = 0;
const unsigned long period = 60000;  //the value is a number of milliseconds, ie 60 seconds

short int curTimeHour = 0; // hours portion of the current time
short int curTimeMin = 0; // minutes portion of the current time
char curTimeChar[5] = {'0', '0', ':', '0', '0'}; // displayable text of the time
bool AM = true; // true if the time is AM, false if it is PM
/// timer and alarm variables
short int alarmTime = 0; // minutes left in the timer
short int timerWorkTime = 0; // minutes set for work timer
short int timerWorkTimeRemaining = 0; // minutes left in the timer
short int timerBreakTime = 0; // minutes set for break timer
short int timerBreakTimeRemaining = 0; // minutes left in the timer
char timerChar[2] = {'0', '0'}; // the first and second characters representing the remaining time in minutes (timers and alarms can be a max of 1 hour in this clock screen design)
char alarmChar[2] = {'0', '0'}; // the first and second characters representing the remaining time in minutes (timers and alarms can be a max of 1 hour in this clock screen design)
char TAwindowSym = 'w'; // ' ': none, 'w': timer (work section), 'b': timer (break section), 'a': alarm 
short int TAwindowProgress = 25; // percent of the time left (out of the max progress bar length: 25)

//// encoder declarations
//#define encoderOutputA 6 // pin 6 is encoder output A
//#define encoderOutputB 5 // pin 5 is encoder output B
// #define buttonE 7 // pin 7 is the encoder button output
volatile boolean fired;
volatile boolean up;

const byte encoderPinA = 2;
const byte encoderPinB = 3;

// Interrupt Service Routine for a change to encoder pin A
void isr ()
{
  if (digitalRead (encoderPinA))
    up = digitalRead (encoderPinB);
  else
    up = !digitalRead (encoderPinB);
  
  if(digitalRead (encoderPinA)){
    fired = true;
  }
}  // end of isr

bool scroll = false; // this determines if the encoder needs to be read. // it will be true if there is a scrollable menu open

//// button declarations
#define buttonA 9 // pin 9 is the left button output
#define buttonB 8 // pin 8 is the right button output
bool pressedA = false; // for use in activating button activated code and to prevent multifiring on one press
bool pressedB = false; // for use in activating button activated code and to prevent multifiring on one press

//// sound declarations
#define clickA 10 // the sound for making a menu selection
#define clickB 11 // the sound for finishing the setting of the clock, alarm, or timer
#define alarmEnd 12 // the sound that plays when an alarm finishes
bool soundPlaying = false; // keeps track of whether a sound is being played
short int curSoundPlaying = 0; // 0: no sound, 1: clickA, 2: clickB, 3: alarmEnd

    /////////////////
   ///// Setup /////
  /////////////////

void setup() {
  // put your setup code here, to run once:

  /// OLED screen setup
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(1000); // Pause for 2 seconds
  // Clear the buffer
  display.clearDisplay();
  
  /// encoder setup
  pinMode (encoderPinA, INPUT_PULLUP);     // enable pull-ups
  pinMode (encoderPinB, INPUT_PULLUP); 
  attachInterrupt (digitalPinToInterrupt (encoderPinA), isr, CHANGE);   // interrupt 0 is pin 2
//  pinMode (encoderOutputA,INPUT);
//  pinMode (encoderOutputB,INPUT);
  //pinMode (buttonE, INPUT); 
  
  /// button setup
  pinMode (buttonA, INPUT);
  pinMode (buttonB, INPUT);

  /// sound pin setup (seems to be unnecessary due to the activation logic in the loop)
//  pinMode (clickA, OUTPUT);
//  pinMode (clickB, OUTPUT);
//  pinMode (alarmEnd, OUTPUT);
  
  Serial.begin (9600); // not sure if this is the best serial pace to use

}


    ////////////////
   ///// Loop /////
  ////////////////

void loop() {
  // put your main code here, to run repeatedly:
  
  static long rotaryCount = 0; // declare the counter for the encoder
  static long previousRotaryCount = 0; // declare the previous count counter for the encoder

    /////////////////////////
   // timing funtionality //
  /////////////////////////
  
  // time incrementing
  unsigned long currentMillis = millis();  //get the current "time" (actually the number of milliseconds since the program started)
  if (currentMillis - previousMillis >= period)  //test whether the period has elapsed
  {
    previousMillis = currentMillis;  //IMPORTANT to save the start time of the current LED state.
    
    //// update that a minute has passed
    /// clock
    curTimeMin++; // add a mimute to the curent time minute variable
    // if the current minute count is 60 (or higher for some reason) increase the hour by one and rollover the minute count
    if (curTimeMin >= 60){
        curTimeMin = curTimeMin - 60;
        curTimeHour++;
        // if the current hour count is 12 (or higher for some reason) rollover the hour count and flip the state of AM
        if (curTimeHour >= 12){
            curTimeHour = curTimeHour - 12;
            AM = !AM;
        }
    }
    
    /// timer/alarm
    // only adjusts if the alarm/timer variables are greater than zero
    // if it sets an alarm of timer to zero the alarm done screen is activated for a second and a sound is played
    // if a timer is what finished then swap the timer value over to the next phase of the pomodoro set
    // check if an alarm is active
    if (alarmTime > 0){
      alarmTime--;
      // if the alarm has just been set to zero show th alarm sounding icon, reset the timer window symbol, and play the alarm sound
      if (alarmTime == 0){
        TAwindow = 3;
        TAwindowSym = ' ';
        //ring the alarm <--
        //0: none, 1: clickA, 2: clickB, 3: alarmEnd
        curSoundPlaying = 3;
      }
    }
    
    if (timerWorkTimeRemaining > 0){
      timerWorkTimeRemaining--;
      // if the work Time has just been set to zero swap to the break countdown by setting the timer window symbol to 'b', reset the work timer, and play the timer sound
      if (timerWorkTimeRemaining == 0){
        TAwindowSym = 'b';
        timerWorkTimeRemaining = timerWorkTime;
        //ring the timer <--
        //0: none, 1: clickA, 2: clickB, 3: alarmEnd
        curSoundPlaying = 3;
      }
    }

    if (timerBreakTimeRemaining > 0){
      timerBreakTimeRemaining--;
      // if the work Time has just been set to zero swap to the work countdown by setting the timer window symbol to 'w', reset the break timer, and play the timer sound
      if (timerBreakTimeRemaining == 0){
        TAwindowSym = 'w';
        timerBreakTimeRemaining = timerBreakTime;
        //ring the timer <--
        //0: none, 1: clickA, 2: clickB, 3: alarmEnd
        curSoundPlaying = 3;
      }
    }
    
  }

    ////////////////////////
   // input funtionality //
  ////////////////////////
  
  // encoder reading
  // If there is a scrollable menu open the loop will check tbe encoder for updates
  // and will modify encoderCounter accordingly
  if (scroll){
    // if the interupt has been activated the encoder hs been "fired"
    if (fired){
      if (up){
//        rotaryCount++;
        rotaryCount--; // reversed to correct rotation direction
      } else {
//        rotaryCount--;
        rotaryCount++; // reversed to correct rotation direction
      }
      fired = false;
      
//      Serial.print ("Count = ");  
//      Serial.println (rotaryCount);
    }
  }

  // check button A for presses
  if (digitalRead(buttonA) == HIGH){
    // show future code that button A has been pressed
//    Serial.println ("button A pressed");
    pressedA = true;
    delay(500);
    //0: none, 1: clickA, 2: clickB, 3: alarmEnd
    //curSoundPlaying = 2; curretly disabled until better sounds are found
  } else {
    pressedA = false; 
  }

  // check button B for presses
  if (digitalRead(buttonB) == HIGH){
    // show future code that button B has been pressed
//    Serial.println ("button B pressed");
    pressedB = true;
    delay(500);
    //0: none, 1: clickA, 2: clickB, 3: alarmEnd
    //curSoundPlaying = 1; curretly disabled until better sounds are found
  } else {
    pressedB = false; 
  }
  
    /////////////////////////////////////
   ///// UI specific functionality /////
  /////////////////////////////////////
  
  /// menu actions
  switch (menuDisplay) {
      case 0:
        // no menu
        // if button A is pressed then open the menu screen
        if (pressedA){
          // activate menu option 1
          menuDisplay = 1;
          // reset and activate relevant variables
          menuOptionHighlight = 0;
          rotaryCount = 0;
          previousRotaryCount = 0;
          scroll = true;
          pressedA = false;
          buttonDisplay = true;
        }
      break;
      
      case 1:
        /// main menu
        //if the encoder has rotated to the right move the highlight to the right
        //if the encoder has rotated to the left move the highlight to the left
        // if button B is pressed exit the main menu (imediately break from this case since the menu is now closed)
        if (pressedB){
          menuDisplay = 0;
          pressedB = false;
          pressedA = false;
          scroll = false;
          rotaryCount = 0;
          previousRotaryCount = 0;
          buttonDisplay = false;
        } else {
          // if the count has gone down or up change the menu highlight accordingly
          if (previousRotaryCount < rotaryCount){
            switch (menuOptionHighlight) {
              case 0:
                menuOptionHighlight = 1;
              break;
              case 1:
                menuOptionHighlight = 2;
              break;
              case 2:
                menuOptionHighlight = 0;
              break; 
            }
          } else if (previousRotaryCount > rotaryCount){
            switch (menuOptionHighlight) {
              case 0:
                menuOptionHighlight = 2;
              break;
              case 1:
                menuOptionHighlight = 0;
              break;
              case 2:
                menuOptionHighlight = 1;
              break; 
            }
          }
          // set the new previous rotary encoder count
          previousRotaryCount = rotaryCount;
        }
        /// draw the selection box around the currently highlighed box with the switch below
        switch (menuOptionHighlight) {
          case 0:
            // option 1
            if (pressedA){
              // activate menu option 1
              menuDisplay = 2;
              // reset and activate relevant variables
              rotaryCount = 0;
              previousRotaryCount = 0;
              scroll = true;
              pressedA = false;
            }
          break;
          
          case 1:
            // option 2
            if (pressedA){
              // activate menu option 2
              menuDisplay = 4;
              // reset and activate relevant variables
              rotaryCount = 0;
              previousRotaryCount = 0;
              scroll = true;
              pressedA = false;
              //set the alarm to 0 and delete any running alarms or timers
            }
          break;

          case 2:
            // option 3
            if (pressedA){
              // activate menu option 3
              menuDisplay = 5;
              // reset and activate relevant variables
              rotaryCount = 0;
              previousRotaryCount = 0;
              scroll = true;
              pressedA = false;
              //set the timer to 0 and delete any running alarms or timers
            }
          break;
        }
      break;

      case 2:
        /// set time (minutes)
        if (pressedB){
          menuDisplay = 0;
          pressedB = false;
          pressedA = false;
          scroll = false;
          rotaryCount = 0;
          previousRotaryCount = 0;
          buttonDisplay = false;
        }
        
        // update the munites based on the encoder value
        curTimeMin = curTimeMin + rotaryCount;
        // reset encoder value
        rotaryCount = 0;
        // if the current set minutes is greater than 60 remove 60 from it and check again until it is less than 60
        while (curTimeMin > 59){
          // subtract the rollover
          curTimeMin = curTimeMin - 60;
        }
        // if the current set minutes is less than 0 add 60 to it and check again until it is greater than 0
        while (curTimeMin < 0){
          // remove the rollover
          curTimeMin = curTimeMin + 60;
        }
        
        // if button A is pressed move on to selecting the hour
        if (pressedA){
          // move on to adjusting the hour time
          menuDisplay = 3;
          // reset and activate relevant variables
          rotaryCount = 0;
          previousRotaryCount = 0;
          scroll = true;
          pressedA = false;
        }
      break;

      case 3:
        /// set time (hours)
        if (pressedB){
          menuDisplay = 0;
          pressedB = false;
          pressedA = false;
          scroll = false;
          rotaryCount = 0;
          previousRotaryCount = 0;
          buttonDisplay = false;
        }
        
        // update the munites based on the encoder value
        curTimeHour = curTimeHour + rotaryCount;
        // reset encoder value
        rotaryCount = 0;
        // if the current set minutes is greater than 60 remove 60 from it and check again until it is less than 60
        while (curTimeHour > 12){
          // subtract the rollover
          curTimeHour = curTimeHour - 12;
          // flip AM
          AM = !AM;
        }
        // if the current set minutes is less than 0 add 60 to it and check again until it is greater than 0
        while (curTimeHour <= 0){
          // remove the rollover
          curTimeHour = curTimeHour + 12;
          // flip AM
          AM = !AM;
        }
        
        // if button A is pressed move on to selecting the hour
        if (pressedA){
          // finish the clock setting and return to main screen
          menuDisplay = 0;
          // reset and activate relevant variables
          pressedB = false;
          pressedA = false;
          scroll = false;
          rotaryCount = 0;
          previousRotaryCount = 0;
          buttonDisplay = false;
        }
      break;

      case 4:
        /// set alarm (time select screen)
        // display the timer as it is set
        TAwindow = 2;
        TAwindowSym = 'a';
        
        if (pressedB){
          menuDisplay = 0;
          pressedB = false;
          pressedA = false;
          scroll = false;
          rotaryCount = 0;
          previousRotaryCount = 0;
          buttonDisplay = false;
          TAwindow = 0;
          TAwindowSym = ' ';
        }

        // update the minutes based on the encoder value
        alarmTime = alarmTime + rotaryCount;
        // reset encoder value
        rotaryCount = 0;
        // if the current set minutes is greater than 60 remove 60 from it and check again until it is less than 60
        while (alarmTime > 60){
          // subtract the rollover
          alarmTime = alarmTime - 60;
        }
        // if the current set minutes is less than 0 add 60 to it and check again until it is greater than 0
        while (alarmTime < 0){
          // remove the rollover
          alarmTime = alarmTime + 60;
        }
        
        // if button A is pressed move on to selecting the hour
        if (pressedA){
          // finish the alarm setting and return to main screen
          menuDisplay = 0;
          TAwindow = 2;
          TAwindowSym = 'a';
          // set the progress bar to its full length
          TAwindowProgress = 25;
          // reset and activate relevant variables
          pressedB = false;
          pressedA = false;
          scroll = false;
          rotaryCount = 0;
          previousRotaryCount = 0;
          buttonDisplay = false;
        }
      break;

      case 5:
        /// set pomodoro timer (work time select screen)
        // display the timer as it is set
        TAwindow = 1;
        TAwindowSym = 'w';
          
        if (pressedB){
          menuDisplay = 0;
          pressedB = false;
          pressedA = false;
          scroll = false;
          rotaryCount = 0;
          previousRotaryCount = 0;
          buttonDisplay = false;
          TAwindow = 0;
          TAwindowSym = ' ';
        }

        // update the minutes based on the encoder value
        timerWorkTime = timerWorkTime + rotaryCount;
        // reset encoder value
        rotaryCount = 0;
        // if the current set minutes is greater than 60 remove 60 from it and check again until it is less than 60
        while (timerWorkTime > 60){
          // subtract the rollover
          timerWorkTime = timerWorkTime - 60;
        }
        // if the current set minutes is less than 0 add 60 to it and check again until it is greater than 0
        while (timerWorkTime < 0){
          // remove the rollover
          timerWorkTime = timerWorkTime + 60;
        }
        
        // if button A is pressed move on to selecting the hour
        if (pressedA){
          // finish the work timer setting and move to the break timer select
          menuDisplay = 6;
          TAwindow = 1;
          timerWorkTimeRemaining = timerWorkTime;
          // set the progress bar to its full length
          TAwindowProgress = 12;
          // reset and activate relevant variables
          pressedB = false;
          pressedA = false;
          scroll = true;
          rotaryCount = 0;
          previousRotaryCount = 0;
          buttonDisplay = true;
        }
      break;

      case 6:
        /// set pomodoro timer (rest time select screen)
        // display the timer as it is set
        TAwindow = 1;
        TAwindowSym = 'b';
        
        if (pressedB){
          menuDisplay = 0;
          pressedB = false;
          pressedA = false;
          scroll = false;
          rotaryCount = 0;
          previousRotaryCount = 0;
          buttonDisplay = false;
          TAwindow = 0;
          TAwindowSym = ' ';
        }

        // update the minutes based on the encoder value
        timerBreakTime = timerBreakTime + rotaryCount;
        // reset encoder value
        rotaryCount = 0;
        // if the current set minutes is greater than 60 remove 60 from it and check again until it is less than 60
        while (timerBreakTime > 60){
          // subtract the rollover
          timerBreakTime = timerBreakTime - 60;
        }
        // if the current set minutes is less than 0 add 60 to it and check again until it is greater than 0
        while (timerBreakTime < 0){
          // remove the rollover
          timerBreakTime = timerBreakTime + 60;
        }
        
        // if button A is pressed move on to selecting the hour
        if (pressedA){
          // finish the break timer setting, activate the work timer, and return to main screen
          menuDisplay = 0;
          TAwindow = 1;
          timerBreakTimeRemaining = timerBreakTime;
          TAwindowSym = 'w';
          // set the progress bar to its full length
          TAwindowProgress = 25;
          // reset and activate relevant variables
          pressedB = false;
          pressedA = false;
          scroll = false;
          rotaryCount = 0;
          previousRotaryCount = 0;
          buttonDisplay = false;
        }
        //if the encoder has rotated to the right move the break timer one minute up
        //if the encoder has rotated to the left move the break timer one minute down
        //if button A is pressed start the timer (begining with a work cycle) and menuDisplay = 0 and buttonDisplay = false
        //if button B is pressed cancel the timer and menuDisplay = 1
      break;
  }

  //// clock maintinence
  // minutes char conversion
  if(curTimeMin < 10){
    curTimeChar[3] = '0';
    curTimeChar[4] = '0' + curTimeMin;
  } else {
    short int tensPlace = curTimeMin / 10;
    curTimeChar[3] = '0' + tensPlace;
    curTimeChar[4] = '0' + (curTimeMin - (tensPlace * 10));
  }
  
  // hours char conversion
  if(curTimeHour < 10){
    curTimeChar[0] = '0';
    curTimeChar[1] = '0' + curTimeHour;
    
  } else { 
    curTimeChar[0] = '1';
    curTimeChar[1] = '0' + (curTimeHour - 10);
  }
  
  switch (TAwindow) { //0: none, 1: timer, 2: alarm, 3: sounding alarm/timer
    case 0:
      // display nothing
      
    break;
    
    case 1:
      // timer
      if(TAwindowSym == 'w'){
        if(timerWorkTimeRemaining < 10){
          timerChar[0] = '0';
          timerChar[1]= '0' + timerWorkTimeRemaining;
        } else {
          short int tensPlace = timerWorkTimeRemaining / 10;
          timerChar[0] = '0' + tensPlace;
          timerChar[1] = '0' + (timerWorkTimeRemaining - (tensPlace * 10));
        }
      } else {
        if(timerBreakTimeRemaining < 10){
          timerChar[0] = '0';
          timerChar[1]= '0' + timerBreakTimeRemaining;
        } else {
          short int tensPlace = timerBreakTimeRemaining / 10;
          timerChar[0] = '0' + tensPlace;
          timerChar[1] = '0' + (timerBreakTimeRemaining - (tensPlace * 10));
        }
      }
    break;

    case 2:
      // alarm
      if(alarmTime < 10){
        alarmChar[0] = '0';
        alarmChar[1]= '0' + alarmTime;
      } else {
        short int tensPlace = alarmTime / 10;
        alarmChar[0] = '0' + tensPlace;
        alarmChar[1] = '0' + (alarmTime - (tensPlace * 10));
      }
    break;

    case 3:
      // ringing alarm (no progess bar or variable numbers)
    break;
  }

    //////////////////////
   /////sound timing/////
  //////////////////////
  
  if (currentMillis - previousMillisSound >= 130)  //test whether the time it takes to activate a sound (+10 miliseconds to ensure reliability) has elapsed
  {
    previousMillisSound = currentMillis;  //IMPORTANT to save the start time of the current LED state.

    if (soundPlaying){
      curSoundPlaying = 0;
    }
    //// update the sound
    switch (curSoundPlaying) { //0: none, 1: clickA, 2: clickB, 3: alarmEnd
      case 0:
        // turn off all sound channels
        // set all sound pins to input to emulate an open drain pin
        pinMode(10, INPUT);
        pinMode(11, INPUT);
        pinMode(12, INPUT);
        soundPlaying = false;
      break;
      
      case 1:
        // play clickA (10) and turn off all sound channels
        pinMode(10, OUTPUT);
        digitalWrite(10, LOW);
        pinMode(11, INPUT);
        pinMode(12, INPUT);
        soundPlaying = true;
      break;
      
      case 2:
        // play clickB (11) and turn off all sound channels
        pinMode(10, INPUT);
        pinMode(11, OUTPUT);
        digitalWrite(11, LOW);
        pinMode(12, INPUT);
        soundPlaying = true;
      break;
      
      case 3:
        // play alarmEnd (12) and turn off all sound channels
        pinMode(10, INPUT);
        pinMode(11, INPUT);
        pinMode(12, OUTPUT);
        digitalWrite(12, LOW);
        soundPlaying = true;
      break;
    }
    
  }

  /// draw the screen
  drawScreen();
}

// function for drawing the screen
void drawScreen() {
  /// clear display
  display.clearDisplay();
  
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.cp437(true); // Use full 256 char 'Code Page 437' font
  
  /// draw the constant outlines
  display.drawRect(0, 0, display.width(), 25, SSD1306_WHITE); // top rectangle
  display.drawLine(93, 1, 93, 24, SSD1306_WHITE); // clock/timer separation line
  display.drawRect(0, 52, 39, 11, SSD1306_WHITE); // left button action box
  display.drawRect(88, 52, 39, 11, SSD1306_WHITE); // right button action box
  display.drawLine(39, 52, 87, 52, SSD1306_WHITE); // button box area close in line
  
  /// draw the clock specific content
  display.setTextSize(2); // clock text size
  display.setCursor(3, 5);     // upper right
  
  for (int i = 0; i < 5; i++) {
    display.write(curTimeChar[i]);
  } // if length can change use sizeof(xyz)/sizeof(xyz[0]) to get the length of array xyz
  display.setCursor(68, 5);     // after numbers on upper right
  if (AM) {
    display.write("AM");
  }
  else {
    display.write("PM");
  }
  
  /// draw the alarm/timer
  // draw the rectangle
  switch (TAwindow) { //0: none, 1: timer, 2: alarm, 3: sounding alarm/timer
    case 0:
      // display nothing
    break;
    
    case 1:
      // timer
      display.setCursor(97, 4); // upper right
      display.write(timerChar[0]); // digit one of current timer value
      display.write(timerChar[1]); // digit two of current timer value
      display.setTextSize(1); // symbol text size
      display.setCursor(121, 3);     // upper left
      display.write(TAwindowSym); // current timer symbol
      // draw the progress bar
    break;

    case 2:
      // alarm
      display.setCursor(97, 4); // upper right
      display.write(alarmChar[0]); // digit one of current alarm value
      display.write(alarmChar[1]); // digit two of current alarm value
      display.setTextSize(1); // symbol text size
      display.setCursor(121, 3); // upper left
      display.write(TAwindowSym); // alarm symbol
    break;

    case 3:
      // ringing alarm (no progess bar or symbols)
      display.setCursor(97, 4); // upper right
      display.write("00"); // digit one of current alarm value
    break;
  }
  
  /// draw the menu
  switch (menuDisplay) {
      case 0:
        // no menu
      break;
      
      case 1:
        /// main menu
        /// draw the three boxes and lables for t-set, alarm, and timer
        for(int i = 0; i < 3; i++){
          display.drawPixel(2 + (42*i), 31, SSD1306_WHITE);
          display.drawPixel(3 + (42*i), 31, SSD1306_WHITE);
          display.drawPixel(2 + (42*i), 32, SSD1306_WHITE);
          
          display.drawPixel(40 + (42*i), 31, SSD1306_WHITE);
          display.drawPixel(41 + (42*i), 31, SSD1306_WHITE);
          display.drawPixel(41 + (42*i), 32, SSD1306_WHITE);
          
          display.drawPixel(41 + (42*i), 42, SSD1306_WHITE);
          display.drawPixel(41 + (42*i), 43, SSD1306_WHITE);
          display.drawPixel(40 + (42*i), 43, SSD1306_WHITE);
  
          display.drawPixel(3 + (42*i), 43, SSD1306_WHITE);
          display.drawPixel(2 + (42*i), 43, SSD1306_WHITE);
          display.drawPixel(2 + (42*i), 42, SSD1306_WHITE);
        }

        /// draw the text for the menu options
        display.setTextSize(1);
        display.setCursor(8, 34); 
        display.write("t-set");
        display.setCursor(50, 34); 
        display.write("alarm");
        display.setCursor(92, 34); 
        display.write("timer");
        
        /// draw the selection box around the currently highlighed box with the switch below
        switch (menuOptionHighlight) {
          case 0:
            // option 1
            display.drawLine(5, 31, 38, 31, SSD1306_WHITE); // top line
            display.drawLine(5, 43, 38, 43, SSD1306_WHITE); // bottom line
            display.drawLine(2, 34, 2, 40, SSD1306_WHITE); // left line
            display.drawLine(41, 34, 41, 40, SSD1306_WHITE); // right line
          break;
          
          case 1:
            // option 2
            display.drawLine(47, 31, 80, 31, SSD1306_WHITE); // top line
            display.drawLine(47, 43, 80, 43, SSD1306_WHITE); // bottom line
            display.drawLine(44, 34, 44, 40, SSD1306_WHITE); // left line
            display.drawLine(83, 34, 83, 40, SSD1306_WHITE); // right line
          break;

          case 2:
            // option 3
            display.drawLine(89, 31, 122, 31, SSD1306_WHITE); // top line
            display.drawLine(89, 43, 122, 43, SSD1306_WHITE); // bottom line
            display.drawLine(86, 34, 86, 40, SSD1306_WHITE); // left line
            display.drawLine(125, 34, 125, 40, SSD1306_WHITE); // right line
          break;
        }
      break;

      case 2:
        /// set time (minutes)
        //draw static parts of the image
        display.drawLine(0, 38, 54, 38, SSD1306_WHITE);
        display.drawLine(72, 38, 127, 38, SSD1306_WHITE);
        display.drawCircle(63, 38, 8, SSD1306_WHITE);
//        display.drawLine(62, 38, 66, 38, SSD1306_WHITE);
//        display.drawLine(63, 33, 63, 39, SSD1306_WHITE);
        /// moving clock arms
        display.drawLine(63, 38, (int)(63 + (4 * cos( ((TWO_PI/12)* curTimeHour) - HALF_PI ))),(int)(38 + (4 * sin( ((TWO_PI/12)* curTimeHour) - HALF_PI ))), SSD1306_WHITE); // hour hand
        display.drawLine(63, 38, (int)(63 + (6 * cos( ((TWO_PI/60)* curTimeMin) - HALF_PI ))),(int)(38 + (6 * sin( ((TWO_PI/60)* curTimeMin) - HALF_PI ))), SSD1306_WHITE); // minute hand

        // white pixel for arm center
        display.drawPixel(63, 38, 0);
      break;

      case 3:
        /// set time (hours)
        //draw static parts of the image
        display.drawLine(0, 38, 54, 38, SSD1306_WHITE);
        display.drawLine(72, 38, 127, 38, SSD1306_WHITE);
        display.drawCircle(63, 38, 8, SSD1306_WHITE);
//        display.drawLine(62, 38, 66, 38, SSD1306_WHITE);
//        display.drawLine(63, 33, 63, 39, SSD1306_WHITE);
        /// moving clock arms
        display.drawLine(63, 38, (int)(63 + (4 * cos( ((TWO_PI/12)* curTimeHour) - HALF_PI ))),(int)(38 + (4 * sin( ((TWO_PI/12)* curTimeHour) - HALF_PI ))), SSD1306_WHITE); // hour hand
        display.drawLine(63, 38, (int)(63 + (6 * cos( ((TWO_PI/60)* curTimeMin) - HALF_PI ))),(int)(38 + (6 * sin( ((TWO_PI/60)* curTimeMin) - HALF_PI ))), SSD1306_WHITE); // minute hand

        // white pixel for arm center
        display.drawPixel(63, 38, 0);
      break;

      case 4:
        /// set alarm (time select screen)
        display.drawLine(0, 38, 54, 38, SSD1306_WHITE);
        display.drawLine(72, 38, 127, 38, SSD1306_WHITE);
        display.drawCircle(63, 38, 8, SSD1306_WHITE);
        display.drawLine(62, 38, 66, 38, SSD1306_WHITE);
        display.drawLine(63, 33, 63, 39, SSD1306_WHITE);
        display.drawPixel(63, 38, 0);
        // draw a readout of the current alarm time
      break;

      case 5:
        /// set pomodoro timer (work time select screen)
        display.drawLine(0, 38, 54, 38, SSD1306_WHITE);
        display.drawLine(72, 38, 127, 38, SSD1306_WHITE);
        display.drawCircle(63, 38, 8, SSD1306_WHITE);
        display.drawLine(62, 38, 66, 38, SSD1306_WHITE);
        display.drawLine(63, 33, 63, 39, SSD1306_WHITE);
        display.drawPixel(63, 38, 0);
        // draw a readout of the current timer time and indication that this is the "work" time
      break;

      case 6:
        /// set pomodoro timer (rest time select screen)
        display.drawLine(0, 38, 54, 38, SSD1306_WHITE);
        display.drawLine(72, 38, 127, 38, SSD1306_WHITE);
        display.drawCircle(63, 38, 8, SSD1306_WHITE);
        display.drawLine(62, 38, 66, 38, SSD1306_WHITE);
        display.drawLine(63, 33, 63, 39, SSD1306_WHITE);
        display.drawPixel(63, 38, 0);
        // draw a readout of the current timer time and indication that this is the "break" time
      break;
  }

  /// draw the buttons
  //draw the two rectangles
  switch (buttonDisplay) {
    case false:
      // no menu settings (left = menu, right = ____)
      // draw the text
      display.setTextSize(1);
      display.setCursor(8, 53);     // lower left
      display.write("menu");
      display.setCursor(96, 53);     // lower right
      display.write("____");
    break;
    
    case true:
      // in a menu or menu item (left = entr, right = back)
      // draw the text
      display.setTextSize(1);
      display.setCursor(8, 53);     // lower left
      display.write("entr");
      display.setCursor(96, 53);     // lower right
      display.write("back");
    break;
  }

  /// show the screen
  display.display();
}
