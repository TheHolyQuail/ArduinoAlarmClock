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

// screen definitions
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET 4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//// encoder declarations
#define encoderOutputA 6 // pin 6 is encoder output A
#define encoderOutputB 7 // pin 7 is encoder output B

int encoderCounter = 0; // a counter for the number of encoder ticks since last reset 
//(encoderCounter resets to zero after a scrollable menu option is chosen resulting in the closure of the menu)
int aState; // the state of encoderOutputA
int aLastState; // the last state of encoderOutputA
bool scroll = false; // this determines if the encoder needs to be read.
// it will be true if there is a scrollable menu open

//// button declarations
//#define buttonA _ // pin _ is the left button output
//#define buttonB _ // pin _ is the right button output
//#define buttonE _ // pin _ is the encoder button output

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

  // encoder setup
  pinMode (encoderOutputA,INPUT);
  pinMode (encoderOutputB,INPUT);
  //pinMode (buttonE, INPUT); (commented until I decide on the pin assignment)

  // button setup
  //pinMode (buttonA, INPUT); (commented until I decide on the pin assignment)
  //pinMode (buttonB, INPUT); (commented until I decide on the pin assignment)

  // OLED screen setup

  // clock setup (for when using real time clock)

  
  
  Serial.begin (9600); // not usre if this is the best serial pace to use

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
  
  //  check button A for presses
  if (digitalRead(buttonA) == HIGH){
    //activate whatever menu option is active
  }

  //  check button B for presses
  if (digitalRead(buttonB) == HIGH){
    //activate whatever secondary option is active (most likely "back" or "cancel")
  }
  
}
