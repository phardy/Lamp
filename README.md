# Lamp

An Arduino sketch to drive an RGB LED lamp.

Note that this is Leonardo-specific, but should work on other Arduino-compatible boards with a second serial (Serial1) connection with minor modifications.

## Hardware

* An RGB LED wired as:
  * Red to pin 9.
  * Green to pin 10.
  * Blue to pin 11.
  I'm using a SparkFun high-power LED. Check references for driver circuit.
* Momentary button attached to pin 2.
* Bluetooth module: RX on pin 0, TX on pin 1. (This is mapped to Serial1 on
  Leonardo boards)

## Bluetooth protocol

A valid bluetooth command is no more than 31 characters and looks like
`command[, argument];`

Valid commands are:
* `on`: Turns the lamp on. Default is to turn on full white (0xFFFFFF), but
  can be overridden by sending an optional colour in hex.
* `off`: Turns the lamp off.
* `blink`: The lamp blinks on and off, before turning on. Default blink time
  is 30 seconds, but can be overridden by sending an optional time in seconds.
* `timer`: Turn the lamp off after a given time. Default is five minutes, but
  can be set by sending an optional time in seconds. If lamp is off, it's
  turned on, otherwise the current mode is used.
  
## LED characteristics

I've found that the minimum PWM values to make this LED light up are `Red: 24, Green: 25, Blue: 50`. And naturally, they need to be considerably higher before the light is visible through my diffuser. I've tried to come up with colours that look reasonable with my LED, but they'll probably need to be tweaked for significantly different LEDs.
  
## References

* LED: http://www.sparkfun.com/products/8718
* LED driver: http://www.instructables.com/id/Circuits-for-using-High-Power-LED-s/
* Button wiring: http://www.arduino.cc/en/Tutorial/Button
* Compatible bluetooth module: http://www.sparkfun.com/products/10393
