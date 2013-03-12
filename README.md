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

## Talking to the lamp from Android

`py/lampcontrol.py` is a quick little python script intended as a helper to send commands to the lamp from [Tasker](http://tasker.dinglisch.net/). It requires [SL4A](http://code.google.com/p/android-scripting/) and the python interpreter.

To use it, edit the script and set the `BT_DEVICE_ADDR` variable to the address of your bluetooth device. Use the "Run SL4A Script" action in Tasker, and set the `lamp_cmd` variable. Valid commands are listed are identical to the bluetooth commands, minus the trailing semicolon.

For commands that accept an optional argument, the argument can be specified using the `lamp_args` variable.

## Talking to the lamp from a PC

Once you've paired with the bluetooth module, you should be able to open a serial connection to it with your favourite terminal emulator and issue text commands described in the Bluetooth Protocol section.

`py/ColorSender.py` is a small GTK app that will send arbitrary RGB colours to the lamp, using [pyGTK](http://pygtk.org/) and [pySerial](http://pyserial.sourceforge.net/).

## References

* LED: http://www.sparkfun.com/products/8718
* LED driver: http://www.instructables.com/id/Circuits-for-using-High-Power-LED-s/
* Button wiring: http://www.arduino.cc/en/Tutorial/Button
* Compatible bluetooth module: http://www.sparkfun.com/products/10393
