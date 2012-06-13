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
const int switchPin=4; // The pin the switch is attached to.
const int ledPin=5; // The pin the LED is attached to.
// Pre-defined light levels
const int lightFull=255; // Maximum brightness.
const int lightHalf=128; // Mid-range brightness. Tweak to taste.
const int lightOff=0; // Minimum brightness (off).
// Variables in the timing array
const int timingSize=2; // Overall size of the array.
const int debounceTiming=0; // Array element for debouncing switchPin input.
const int fadeTiming=1; // Array element for fading ledPin.

// Overall lamp states
boolean offState = true; // Whether we're off, or turning off.
boolean onState = false; // Whether we're on, or turning on.

// IO state
int ledState = LOW; // Current LED state.
int buttonState; // Current (debounced) button state.
int lastButtonState = LOW; // Last (debounced) button state.

// Debounce variables
int prebounceButtonState = LOW; // Tracks button state before debouncing.
long debounceDelay = 50; // The debounce time. Input is stable after this.

// Light fading variables
int lightDesired; // What we'd like to transition to.
int lightCurrent; // What the light level currently is.
const int fadeAmount = 5; // How much to fade per step.
const int fadeStepTime = 10; // How long to wait between each fade.

long timingArray[timingSize]; // All timers store their last-run time here.

void setup() {
  pinMode(switchPin, INPUT);
  pinMode(ledPin, OUTPUT);
  buttonState = digitalRead(switchPin);
  // Serial.begin(9600);
  for (int i=0; i<timingSize; i++) {
    timingArray[i] = 0;
  }
}

void loop() {
  // Update button and light states.
  debounceButton();
  lightFade();

  if (buttonState != lastButtonState && buttonState == HIGH) {
    if (offState) {
      // Serial.println("Turning on");
      turnOn();
    } else {
      // Serial.println("Turning off");
      turnOff();
    }
  }
  
  // Reset lastButtonState if anything's changed.
  if (buttonState != lastButtonState) {
    lastButtonState = buttonState;
  }
}

void debounceButton() {
  long *timer = &timingArray[debounceTiming];
  int reading = digitalRead(switchPin);
  long curtime = millis();
  
  // If the switch has changed state
  // at all, reset the debounce timer.
  if (reading != prebounceButtonState) {
    *timer = millis();
  }
  
  // if the last debounce time was long enough
  // ago, then the current reading is stable.
  if ((millis() - *timer) > debounceDelay) {
    buttonState = reading;
  }
  
  prebounceButtonState = reading;
}

void turnOn() {
  lightDesired = lightFull;
  onState = true;
  offState = false;
}

void turnOff() {
  lightDesired = lightOff;
  offState = true;
  onState = false;
}

void lightFade() {
  long *timer = &timingArray[fadeTiming];
  long curTime = millis();
  if (curTime - *timer > fadeStepTime) {
    *timer = curTime;
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
    analogWrite(ledPin, lightCurrent);
  }
}
