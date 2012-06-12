/*
  Control an LED lamp.
  
  Works with a 1W LED driver switched through pin 5,
  and a momentary button attached to pin 4.
  
  References:
  LED driver: http://www.instructables.com/id/Circuits-for-using-High-Power-LED-s/
  Button wiring: http://www.arduino.cc/en/Tutorial/Button
  
  Peter Hardy <peter@hardy.dropbear.id.au>
*/

// IO pins
#define SWITCHPIN 4
#define LEDPIN 5
// Pre-defined light levels
#define LIGHTFULL 255
#define LIGHTHALF 128
#define LIGHTOFF 0
// Variables in the timing array
#define TIMINGSIZE 2
#define DEBOUNCETIMING 0
#define FADETIMING 1

int ledState = LOW; // current LED state
int buttonState; // current (debounced) button state
int lastButtonState = LOW; // last (debounced) button state

// Debounce variables
int prebounceButtonState = LOW; // tracks button state before debouncing
long debounceDelay = 50;

// Light fading variables
int lightDesired; // what we'd like to transition to
int lightCurrent; // what the light level currently is
const int fadeAmount = 5; // how much to fade per step
const int fadeStepTime = 10; // how long to wait between each fade

// lamp states
boolean offState = true;
boolean onState = false;

long timingArray[TIMINGSIZE]; // Holds counts for functions that want to do things periodically

void setup() {
  pinMode(SWITCHPIN, INPUT);
  pinMode(LEDPIN, OUTPUT);
  buttonState = digitalRead(SWITCHPIN);
  // Serial.begin(9600);
  for (int i=0; i<TIMINGSIZE; i++)
    timingArray[i] = 0;
}

void loop() {
  // update button and light states.
  debounceButton();
  lightFade();

  if (buttonState != lastButtonState && buttonState == HIGH) {
    if (offState) {
      Serial.println("Turning on");
      turnOn();
    } else {
      Serial.println("Turning off");
      turnOff();
    }
  }
  
  // Reset lastButtonState if anything's changed.
  if (buttonState != lastButtonState) {
    lastButtonState = buttonState;
  }
}

void debounceButton() {
  int reading = digitalRead(SWITCHPIN);
  long curtime = millis();
  
  // If the switch has changed state
  // at all, reset the debounce timer.
  if (reading != prebounceButtonState) {
    timingArray[DEBOUNCETIMING] = millis();
  }
  
  // if the last debounce time was long enough
  // ago, then the current reading is stable.
  if ((millis() - timingArray[DEBOUNCETIMING]) > debounceDelay) {
    buttonState = reading;
  }
  
  prebounceButtonState = reading;
}

void turnOn() {
  lightDesired = LIGHTFULL;
  onState = true;
  offState = false;
}

void turnOff() {
  lightDesired = LIGHTOFF;
  offState = true;
  onState = false;
}

void lightFade() {
  long curTime = millis();
  if (curTime - timingArray[FADETIMING] > fadeStepTime) {
    timingArray[FADETIMING] = curTime;
    if (lightDesired == lightCurrent) return;
    // Explicitly deal with the case where difference between desired and
    // current is less than the interval, rather than bouncing above and
    // below desired. It's a little thing, but still.
    int diff = lightDesired-lightCurrent;
    if (abs(diff) < fadeAmount) {
      lightCurrent = lightDesired;
    } else {
      if (lightCurrent < lightDesired) {
        lightCurrent = lightCurrent + fadeAmount;
      } else {
        lightCurrent = lightCurrent - fadeAmount;
      }
    }
    // Serial.print("Setting brightness to ");
    // Serial.println(lightCurrent);
    analogWrite(LEDPIN, lightCurrent);
  }
}
