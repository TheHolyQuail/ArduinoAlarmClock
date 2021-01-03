//Arduino libraries
#include <Arduino.h>

// button and encoder libraries
// #include <Button.h> maybe no need for this
// #include <Encoder.h> maybe no need for this

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
// 0: no menu, 1: main menu, 2: set time, 3: set alarm (time select screen), 4: set pomodoro timer (work time select screen), 5: set pomodoro timer (rest time select screen)
short int menuDisplay = 0;
// 0: no menu, 1: first option, 2: second option 3: third option
short int menuOptionHighlight = 0;
// false: no menu settings (left = menu, right = ____), true: in a menu or menu item (left = entr, right = back)
bool buttonDisplay = false;
// (Timer/Alarm window) 0: none, 1: timer, 2: alarm, 3: sounding alarm/timer
short int TAwindow = 0;

//// time declarations
short int curTimeHour = 0; // hours portion of the current time
short int curTimeMin = 0; // minutes portion of the current time
char curTimeChar[5] = {'0', '0', ':', '0', '0'}; // displayable text of the time
bool AM = true; // true if the time is AM, false if it is PM
/// timer and alarm variables
short int alarmTime = 0; // minutes left in the timer
char alarmChar[2] = {'0', '0'}; // the first and second characters representing the remaining time in minutes (timers and alarms can be a max of 1 hour in this design)
char TAwindowSym = 'w'; // " ": none, "w": timer (work section), "b": timer (break section), "a": alarm 
short int TAwindowProgress = 25; // percent of the time left (out of the max progress bar length: 25)

//// encoder declarations
#define encoderOutputA 6 // pin 6 is encoder output A
#define encoderOutputB 5 // pin 5 is encoder output B
#define buttonE 7 // pin 7 is the encoder button output

int encoderCounter = 0; // a counter for the number of encoder ticks since last reset 
//(encoderCounter resets to zero after a scrollable menu option is chosen resulting in the closure of the menu)
int aState; // the state of encoderOutputA
int aLastState; // the last state of encoderOutputA
bool scroll = false; // this determines if the encoder needs to be read.
// it will be true if there is a scrollable menu open

//// button declarations
#define buttonA 9 // pin 9 is the left button output
#define buttonB 8 // pin 8 is the right button output

//// menu declarations
/// menu
bool menuOpen = false;
int menuPos = 0;
/// timer
bool setTimerOpen = false;
/// clock
// variable for time (not sure how to store)
/// set clock
bool setClockOpen = false;


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
  delay(2000); // Pause for 2 seconds
  // Clear the buffer
  display.clearDisplay();
  
  /// encoder setup
  pinMode (encoderOutputA,INPUT);
  pinMode (encoderOutputB,INPUT);
  pinMode (buttonE, INPUT); 
  
  /// button setup
  pinMode (buttonA, INPUT);
  pinMode (buttonB, INPUT);

  /// clock setup (for when using real time clock)

  
  Serial.begin (9600); // not sure if this is the best serial pace to use

  // encoder setup after serial.being
  // Reads the initial state of the outputA
  aLastState = digitalRead(encoderOutputA);

}

void loop() {
  // put your main code here, to run repeatedly:

  // encoder reading
  // If there is a scrollable menu open the loop will check tbe encoder for updates
  // and will modify encoderCounter accordingly
  if (scroll){
    aState = digitalRead(encoderOutputA);
    if (aState != aLastState){     
      if (digitalRead(encoderOutputB) != aState) { 
        encoderCounter ++;
      }
      else {
        encoderCounter--;
      }
    }
    aLastState = aState;
  }
  
  // check button A for presses
  if (digitalRead(buttonA) == HIGH){
    // activate whatever menu option is active
  }

  // check button B for presses
  if (digitalRead(buttonB) == HIGH){
    // activate whatever secondary option is active (most likely "back" or "cancel")
  }
  
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
  //display.write(curTimeChar);
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
      display.write(alarmChar[0]); // digit one of current timer value
      display.write(alarmChar[1]); // digit two of current timer value
      display.setTextSize(1); // symbol text size
      display.setCursor(121, 3);     // upper left
      display.write(TAwindowSym); // current timer symbol
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
        /// set time
        //draw a static image
        display.drawLine(0, 38, 54, 38, SSD1306_WHITE);
        display.drawLine(72, 38, 127, 38, SSD1306_WHITE);
        display.drawCircle(63, 38, 8, SSD1306_WHITE);
        display.drawLine(62, 38, 66, 38, SSD1306_WHITE);
        display.drawLine(63, 33, 63, 39, SSD1306_WHITE);
        display.drawPixel(63, 38, 0);
      break;

      case 3:
        /// set alarm (time select screen)
        display.drawLine(0, 38, 54, 38, SSD1306_WHITE);
        display.drawLine(72, 38, 127, 38, SSD1306_WHITE);
        display.drawCircle(63, 38, 8, SSD1306_WHITE);
        display.drawLine(62, 38, 66, 38, SSD1306_WHITE);
        display.drawLine(63, 33, 63, 39, SSD1306_WHITE);
        display.drawPixel(63, 38, 0);
        // draw a readout of the current alarm time
      break;

      case 4:
        /// set pomodoro timer (work time select screen)
        display.drawLine(0, 38, 54, 38, SSD1306_WHITE);
        display.drawLine(72, 38, 127, 38, SSD1306_WHITE);
        display.drawCircle(63, 38, 8, SSD1306_WHITE);
        display.drawLine(62, 38, 66, 38, SSD1306_WHITE);
        display.drawLine(63, 33, 63, 39, SSD1306_WHITE);
        display.drawPixel(63, 38, 0);
        // draw a readout of the current timer time and indication that this is the "work" time
      break;

      case 5:
        /// set pomodoro timer (rest time select screen)
        display.drawLine(0, 38, 54, 38, SSD1306_WHITE);
        display.drawLine(72, 38, 127, 38, SSD1306_WHITE);
        display.drawCircle(63, 38, 8, SSD1306_WHITE);
        display.drawLine(62, 38, 66, 38, SSD1306_WHITE);
        display.drawLine(63, 33, 63, 39, SSD1306_WHITE);
        display.drawPixel(63, 38, 0);
        // draw a readout of the current timer time and indication that this is the "rest" time
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
      display.write("menu");
      display.setCursor(96, 53);     // lower right
      display.write("back");
    break;
  }

  /// show the screen
  display.display();
}
