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
#define SCREEN_WIDTH 128; // OLED display width, in pixels
#define SCREEN_HEIGHT 64; // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4; // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

/// screen display arrays
// 0: no menu, 1: main menu, 2: set time, 3: set alarm (time select screen), 4: set pomodoro timer (work time select screen), 5: set pomodoro timer (rest time select screen)
int menuDisplay = 0;
// 0: first option, 1: second option, 2: third option
int menuOptionHighlight = 0; //if necessary for minimizing memory using two booleans would be 16 bits instead of the 32 for an int
// false: no menu settings (left = menu, right = ____), true: in a menu or menu item (left = entr, right = back)
bool buttonDisplay = false;
// (Timer/Alarm window) 0: none, 1: timer, 2: alarm, 3: sounding alarm/timer
int TAwindow = 0;

//// time declarations
char curTime[8] = "00:00 AM";
/// timer and alarm variables
//not sure what to do here yet...

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
    aState = digitalRead(outputA);
    if (aState != aLastState){     
      if (digitalRead(outputB) != aState) { 
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
  ///clear display
  //display.clearDisplay();
  
  /// draw the clock

  /// draw the alarm/timer
  // draw the rectangle
  switch (TAwindow) { //0: none, 1: timer, 2: alarm, 3: sounding alarm/timer
    case "0":
      // none
    break;
    
    case "1":
      // timer
    break;

    case "2":
      // alarm
    break;

    case "3":
      // sounding alarm/timer
    break;
  }
  
  /// draw the menu
  switch (menuDisplay) {
      case '0':
        // no menu
      break;
      
      case '1':
        /// main menu
        // draw the three boxes and lables for t-set, alarm, and timer
        // draw the selection box around the currently highlighed box with the switch below
        switch (menuOptionHighlight) {
          case "0":
            // option 1
          break;
          
          case "1":
            // option 2
          break;

          case "2":
            // option 3
          break;
        }
      break;

      case '2':
        /// set time
        //draw a static scroll symbol
      break;

      case '3':
        /// set alarm (time select screen)
        // draw a scroll symbol with a readout of the current alarm time
      break;

      case '4':
        /// set pomodoro timer (work time select screen)
        // draw a scroll symbol with a readout of the current alarm time and indication taht this is the "work" time
      break;

      case '5':
        /// set pomodoro timer (rest time select screen)
        // draw a scroll symbol with a readout of the current alarm time and indication taht this is the "rest" time
      break;
  }

  /// draw the buttons
  //draw the two rectangles
  switch (buttonDisplay) {
    case false:
      // no menu settings (left = menu, right = ____)
      // draw the text
    break;
    
    case true:
      // in a menu or menu item (left = entr, right = back)
      // draw the text
    break;
  }
}
