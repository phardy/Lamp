/*
  Control an LED lamp.
  Note that this is Leonardo-specific, but should work on other
  Arduino-compatible boards with a second serial (Serial1) connection
  with minor modifications.

  Wiring:
  * LED connected to pin 5. (I'm using a 1W LED driver; check references.)
  * Momentary button attached to pin 4.
  * Bluetooth module: RX on pin 0, TX on pin 1. (This is mapped to
    Serial1 on Leonardo boards)
  
  References:
  LED driver: http://www.instructables.com/id/Circuits-for-using-High-Power-LED-s/
  Button wiring: http://www.arduino.cc/en/Tutorial/Button
  Compatible bluetooth module: http://www.sparkfun.com/products/10393
  Compatible Android control app: https://play.google.com/store/apps/details?id=com.app.control
  
  Peter Hardy <peter@hardy.dropbear.id.au>
*/

// IO pins
const int switchPin=4; // The pin the switch is attached to.
const int ledPin=5; // The pin the LED is attached to.
// Pre-defined light levels
const int lightFull=255; // Maximum brightness.
const int lightHalf=160; // Mid-range brightness. Tweak to taste.
const int lightOff=0; // Minimum brightness (off).
// The timing array holds counters used for timed events.
const int timingSize=3; // Overall size of the array.
const int debounceTiming=0; // Array element for debouncing switchPin input.
const int fadeTiming=1; // Array element for fading ledPin.
const int blinkTiming=2; // Array element to control how long to blink for.
long timingArray[timingSize];

// Current lamp state.
enum states {
  on, // The lamp is on.
  off, // The lamp is off.
  blink, // The lamp will blink on and off.
  timer // The lamp is on, but will turn off after a timed delay.
};
states lampState = off;

// IO state
int buttonState; // Current (debounced) button state.
int lastButtonState = LOW; // Last (debounced) button state.
int serialByte; // Bytes read from the bluetooth serial device.

// Debounce variables
int prebounceButtonState = LOW; // Tracks button state before debouncing.
long debounceDelay = 50; // The debounce time. Input is stable after this.

// Light fading variables
int lightDesired; // What we'd like to transition to.
int lightCurrent; // What the light level currently is.
const int normalFade = 5; // Default amount to fade per step.
int fadeAmount = normalFade; // How much to fade per step.
const int fadeStepTime = 10; // How long to wait between each fade.

// Blink variables
const int blinkTime = 30000; // How long to blink for before switching to On.

void setup() {
  pinMode(switchPin, INPUT);
  pinMode(ledPin, OUTPUT);
  buttonState = digitalRead(switchPin);
  // Populate the timing array.
  for (int i=0; i<timingSize; i++) {
    timingArray[i] = 0;
  }

  // Serial.begin(9600); // Debugging.
  Serial1.begin(115200); // Leonardo uses Serial1.
}

void loop() {
  debounceButton(); // Debounce button input.
  lightFade();  // Update light state.
  if (lampState == blink) {
    blinkLight(); // Update blinking.
  }
  readButton(); // Check for debounced button state changes.
  readSerial(); // Check for commands from bluetooth.
}

void readButton() {
  if (buttonState != lastButtonState && buttonState == HIGH) {
    if (lampState == off) {
      turnOn();
    } else {
      turnOff();
    }
  }
  // Reset lastButtonState if anything's changed.
  if (buttonState != lastButtonState) {
    lastButtonState = buttonState;
  }
}

void readSerial() {
  if (Serial1.available() > 0) {
    serialByte = Serial1.read();
    switch (serialByte) {
    case 'A':
      // Serial.println("BT turn on"); // Debugging.
      turnOn();
      break;
    case 'a':
      // Serial.println("BT turn off"); // Debugging.
      turnOff();
      break;
    case 'B':
      // Serial.println("BT blink mode"); // Debugging.
      blinkOn();
      break;
    default:
      Serial.print("BT unknown received: "); // Debugging.
      Serial.println(serialByte);
      break;
    }
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
  // Serial.println("Turning lamp on"); // Debugging.
  lightDesired = lightFull;
  lampState = on;
}

void turnOff() {
  // Serial.println("Turning lamp off"); // Debugging.
  lightDesired = lightOff;
  lampState = off;
}

void blinkOn() {
  timingArray[blinkTiming] = millis();
  lightDesired = lightFull;
  lampState = blink;
}

void lightFade() {
  long *timer = &timingArray[fadeTiming];
  long curTime = millis();
  if (curTime - *timer > fadeStepTime) {
    *timer = curTime;
    if (lightDesired == lightCurrent) return;
    // Explicitly deal with the case where difference between desired and
    // current is less than the interval, rather than bouncing above and
    // below desired.
    // Required because blinking depends on exact values. Also I have OCD.
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
    analogWrite(ledPin, lightCurrent);
  }
}

void blinkLight() {
  long *blinkTimer = &timingArray[blinkTiming];
  long curTime = millis();

  // If the light has reached one end of the blink
  // range, start changing to the other direction.
  if (lightCurrent == lightFull) {
    // Serial.println("Blinking off"); // Debugging
    lightDesired = lightHalf;
  } else if (lightCurrent == lightHalf) {
    // Serial.println("Blinking on"); // Debugging
    lightDesired = lightFull;
  }

  // If total blinking time has exceeded blinkTotal, then stay on.
  if (curTime - *blinkTimer > blinkTime) {
    turnOn();
  }
}
