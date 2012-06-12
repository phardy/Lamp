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
const int switchPin=4;
const int ledPin=5;
// Pre-defined light levels
const int lightFull=255;
const int lightHalf=128;
const int lightOff=0;
// Variables in the timing array
const int timingSize=2;
const int debounceTiming=0;
const int fadeTiming=1;

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

long timingArray[timingSize]; // Holds counts for functions that want to do things periodically

void setup() {
  pinMode(switchPin, INPUT);
  pinMode(ledPin, OUTPUT);
  buttonState = digitalRead(switchPin);
  // Serial.begin(9600);
  for (int i=0; i<timingSize; i++)
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
  int reading = digitalRead(switchPin);
  long curtime = millis();
  
  // If the switch has changed state
  // at all, reset the debounce timer.
  if (reading != prebounceButtonState) {
    timingArray[debounceTiming] = millis();
  }
  
  // if the last debounce time was long enough
  // ago, then the current reading is stable.
  if ((millis() - timingArray[debounceTiming]) > debounceDelay) {
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
  long curTime = millis();
  if (curTime - timingArray[fadeTiming] > fadeStepTime) {
    timingArray[fadeTiming] = curTime;
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
