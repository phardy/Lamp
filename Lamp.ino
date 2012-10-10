/*
  Control an LED lamp.
  Note that this is Leonardo-specific, but should work on other
  Arduino-compatible boards with a second serial (Serial1) connection
  with minor modifications.

  Wiring:
  * An RGB LED connected as:
    Red to pin 9.
    Green to pin 10.
    Blue to pin 11.
    I'm using a SparkFun high-power LED. Check references for driver circuit.
  * Momentary button attached to pin 4.
  * Bluetooth module: RX on pin 0, TX on pin 1. (This is mapped to
    Serial1 on Leonardo boards)

  Bluetooth protocol:
  A valid bluetooth command is no more than 31 characters and looks like:
  "command[, argument];"
  Valid commands are:
  "on": Turns the lamp on.
  "off": Turns the lamp off.
  "blink": The lamp blinks on and off, ending with turning on. Default
           blink time is 30 seconds, unless overridden with an optional arg.
  "mood": The lamp will cycle through random colours.
  "timer": Turn the lamp off after a given time. Default is five minutes,
           but can be overridden with an optional argument. If lamp is off,
	   it's turned on, otherwise the current mode is used.

  References:
  LED: http://www.sparkfun.com/products/8718
  LED driver: http://www.instructables.com/id/Circuits-for-using-High-Power-LED-s/
  Button wiring: http://www.arduino.cc/en/Tutorial/Button
  Compatible bluetooth module: http://www.sparkfun.com/products/10393

  LED characteristics:
  I've found that the minimum PWM values to make this LED light up are:
  Red: 24, Green: 25, Blue: 50.
  And naturally, they need to be considerably higher before the light is
  visible through my diffuser. I've tried to come up with colours that
  look reasonable with my LED, but they'll probably need to be tweaked
  for significantly different LEDs.

  Peter Hardy <peter@hardy.dropbear.id.au>
*/
#include <string.h>

// IO pins
const int switchPin=2; // The pin the switch is attached to.
const int rLedPin=9; // The pin the red LED is attached to.
const int gLedPin=10; // The pin the green LED is attached to.
const int bLedPin=11; // The pin the blue LED is attached to.
// The timing array holds counters used for timed events.
const int timingSize=5; // Overall size of the array.
const int debounceTiming=0; // Array element for debouncing switchPin input.
const int longPressTiming=1; // Array element for timing a long button press.
const int fadeTiming=2; // Array element for fading ledPin.
const int blinkTiming=3; // Array element to control how long to blink for.
const int timerTiming=4; // Array element to control the off timer.
long timingArray[timingSize];

// Current lamp state.
enum lampStates {
  lampOff, // The lamp is off.
  lampDim, // The lamp is on, but dimmed.
  lampOn, // The lamp is on.
  lampBlink, // The lamp will blink on and off.
  lampMood // The lamp will cycle through random colours.
};
lampStates lampState = lampOff;
boolean timerActive = false;

// Button state.
enum buttonStates {
  buttonOff, // The button is off.
  buttonOn, // The button is on (pressed).
  buttonOnLong // The button is on and has been held down.
};
buttonStates curButtonState = buttonOff;
buttonStates lastButtonState = buttonOff;

// Serial reading variables
const int serialLength = 32; // The maximum length of a serial command.
char serialByte; // An individual byte read from serial.
char serialString[serialLength]; // A full serial command.
byte serialIndex = 0; // The index of characters being inserted in inString.
char *serialCmd;
char *serialArgs;

// Debounce variables
int prebounceButtonState = LOW; // Tracks button state before debouncing.
long debounceDelay = 50; // The debounce time. Input is stable after this.

// Light fading variables
const int rLightLevel=0; // Array element for red LED light levels.
const int gLightLevel=1; // Array element for green LED light levels.
const int bLightLevel=2; // Array element for blue LED light levels.
int lightDesired[3]; // What we'd like to transition to.
int lightCurrent[3]; // What the light level currently is.
const int normalFade = 5; // Default amount to fade per step.
int fadeAmount = normalFade; // How much to fade per step.
const int normalStepTime = 15; // Default duration to wait between steps.
int fadeStepTime = normalStepTime; // How long to wait between steps.

// Predefined colours.
int White[3]={255, 255, 255};
int DimWhite[3]={127, 127, 127};
int Red[3]={255, 0, 0};
int Green[3]={0, 255, 0};
int Blue[3]={0, 0, 255};
int Teal[3]={27, 224, 214};
int Purple[3]={242, 85, 219};
int Yellow[3]={247, 255, 18};
int Orange[3]={255, 164, 18};
int Pink[3]={235, 129, 177};
int Black[3]={0, 0, 0}; // I hope nobody emails me about this.

// Blink variables
int blinkTime; // How long to blink for before switching to On.
int blinkColour[3] = {255, 0, 0}; // Colour to blink to.

void setup() {
  pinMode(switchPin, INPUT);
  pinMode(rLedPin, OUTPUT);
  pinMode(gLedPin, OUTPUT);
  pinMode(bLedPin, OUTPUT);
  // Populate the timing array.
  for (int i=0; i<timingSize; i++) {
    timingArray[i] = 0;
  }
  // Populate the light level arrays.
  for (int i=0; i<3; i++) {
    lightDesired[i]=0;
    lightCurrent[i]=0;
  }
  // Serial.begin(9600); // Debugging.
  Serial1.begin(115200); // Leonardo uses Serial1.
  randomSeed(analogRead(0));
}

void loop() {
  debounceButton(); // Debounce button input.
  lightFade();  // Update light state.
  if (lampState == lampBlink) {
    blinkLight(); // Update blinking.
  }
  if (lampState == lampMood) {
    moodLight(); // Update random colours.
  }
  if (timerActive) {
    checkTimer(); // Update off timer.
  }
  readButton(); // Check for debounced button state changes.
  readSerial(); // Check for commands from bluetooth.
}

void readButton() {
  if ((curButtonState != buttonOff) && (curButtonState != lastButtonState)) {
    fadeAmount = normalFade;
    fadeStepTime = normalStepTime;
    if (lampState == lampOff) {
      // go to mid
    } else if (lampState == lampDim) { 
      // go to full
    } else {
      // go to off
    }
  }
  // Reset lastButtonState if anything's changed.
  if (curButtonState != lastButtonState) {
    lastButtonState = curButtonState;
  }
}

void readSerial() {
  while ((Serial1.available() > 0) && (serialIndex < serialLength-1)) {
    serialByte = Serial1.read();
    if (serialByte != ';') {
      serialString[serialIndex] = serialByte;
      serialIndex++;
    }
    if (serialByte == ';' or serialIndex == (serialLength-1)) {
      parseSerial();
      serialIndex = 0;
      memset(&serialString, 0, serialLength);
    }
  }
}

void parseSerial() {
  char *s = serialString;
  serialCmd = strtok_r(s, ", ", &s);
  serialArgs = strtok_r(NULL, ", ", &s);

  if (strcmp(serialCmd, "on") == 0) {
    turnOn();
  } else if (strcmp(serialCmd, "off") == 0) {
    turnOff();
  } else if (strcmp(serialCmd, "blink") == 0) {
    int time = atoi(serialArgs);
    blinkOn(time);
  } else if (strcmp(serialCmd, "mood") == 0) {
    moodOn();
  } else if (strcmp(serialCmd, "timer") == 0) {
    int time = atoi(serialArgs);
    timerOn(time);
  } else {
    // Serial.println("Unable to parse BT command."); // Debugging.
  }
}

void debounceButton() {
  long *timer = &timingArray[debounceTiming];
  int reading = digitalRead(switchPin);
  int curTime = millis();
  
  // If the switch has changed state
  // at all, reset the debounce timer.
  if (reading != prebounceButtonState) {
    *timer = curTime;
  }
  
  // if the last debounce time was long enough
  // ago, then the current reading is stable.
  if ((curTime - *timer) > debounceDelay) {
    if (reading == HIGH) {
      curButtonState = buttonOn;
    } else {
      curButtonState = buttonOff;
    }
  }
  
  prebounceButtonState = reading;
}

void turnOn() {
  // Serial.println("Turning lamp on"); // Debugging.
  fadeAmount = normalFade;
  fadeStepTime = normalStepTime;
  setColour(White);
  lampState = lampOn;
}

void turnOff() {
  // Serial.println("Turning lamp off"); // Debugging.
  setColour(Black);
  lampState = lampOff;
}

void blinkOn(int time) {
  // Serial.println("Turning lamp to blink"); // Debugging.
  fadeAmount = normalFade;
  fadeStepTime = normalStepTime / 2;
  if (time == 0) {
    blinkTime = 30000;
  } else {
    // We receive a time in seconds, but need to store it in millis.
    blinkTime = time * 1000;
  }
  timingArray[blinkTiming] = millis();
  setColour(blinkColour);
  lampState = lampBlink;
}

void moodOn() {
  // Serial.println("Turning lamp to mood."); // Debugging.
  fadeAmount = 1;
  fadeStepTime = 200;
  lampState = lampMood;
}

void timerOn(int time) {
  long *timer = &timingArray[timerTiming];
  if (time == 0) {
    *timer = millis() + 300000;
  } else {
    *timer = millis() + time*1000;
  }
  timerActive = true;
}

void lightFade() {
  long *timer = &timingArray[fadeTiming];
  long curTime = millis();
  if (curTime - *timer > fadeStepTime) {
    *timer = curTime;
    for (int i=0; i<3; i++) {
      int *lc = &lightCurrent[i];
      int *ld = &lightDesired[i];
      if (*lc != *ld) {
	// Explicitly deal with the case where difference between desired and
	// current is less than the interval, rather than bouncing above and
	// below desired.
	// Required because blinking depends on exact values. Also I have OCD.
	int diff = *ld-*lc;
	if (abs(diff) < fadeAmount) {
	  *ld=*lc;
	} else {
	  if (*lc < *ld) {
	    *lc = *lc + fadeAmount;
	  } else {
	    *lc = *lc - fadeAmount;
	  }
	}
      }
    }
    // Write the updated values to the pins.
    analogWrite(rLedPin, lightCurrent[rLightLevel]);
    analogWrite(gLedPin, lightCurrent[gLightLevel]);
    analogWrite(bLedPin, lightCurrent[bLightLevel]);
  }
}

void blinkLight() {
  long *blinkTimer = &timingArray[blinkTiming];
  long curTime = millis();

  // If the light has reached one end of the blink
  // range, start changing to the other direction.
  if (cmpColour(lightCurrent, blinkColour)) {
    // Serial.println("Blinking off"); // Debugging
    setColour(Black);
  } else if (cmpColour(lightCurrent, Black)) {
    // Serial.println("Blinking on"); // Debugging
    setColour(blinkColour);
  }

  // If total blinking time has exceeded blinkTotal, then stay on.
  if (curTime - *blinkTimer > blinkTime) {
    turnOn();
  }
}

void moodLight() {
  if (cmpColour(lightCurrent, lightDesired)) {
    for (int i=0; i<3; i++) {
      lightDesired[i] = random(256);
    }
  }
}

void checkTimer() {
  long *timer = &timingArray[timerTiming];
  long curTime = millis();
  if (curTime > *timer) {
    timerActive = false;
    turnOff();
  }
}

// Set the desired light levels to the colour represented by the
// three-element array passed in.
void setColour(int colour[]) {
  for (int i=0; i<3; i++) {
    lightDesired[i] = colour[i];
  }
}

// Return true if the two colours passed in match.
boolean cmpColour(int colourA[3], int colourB[3]) {
  for (int i=0; i<3; i++) {
    if (colourA[i] != colourB[i]) {
      return false;
    }
  }
  return true;
}
