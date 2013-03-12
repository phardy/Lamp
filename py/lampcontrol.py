"""lampcontrol.py

Android SL4A script to send control codes to an
arduino lamp via a Bluetooth serial connection.

Peter Hardy <peter@hardy.dropbear.id.au>
"""

import android
import sys, time

droid = android.Android()

# Anything not in this list is ignored.
validCommands = ["on", "off", "blink", "timer", "mood"]

try:
    lamp_cmd = str(droid.getIntent().result[u"extras"][u"%lamp_cmd"]).lower()

    if lamp_cmd in validCommands:
        sendStr = "%s" % lamp_cmd
    else:
        droid.makeToast("Received invalid command: %s." % lamp_cmd)
        sys.exit(1)
except:
    droid.makeToast("No lamp_cmd set.")
    sendStr = "off"

try:
    lamp_args = str(droid.getIntent().result[u"extras"][u"%lamp_args"]).lower()
    sendStr = "%s %s;" % (sendStr, lamp_args)
except:
    sendStr = "%s;" % sendStr

# cheat and hard code this for now...
BT_DEVICE_ADDR = "00:06:66:46:5B:B2" # The address of the device to send to.

droid = android.Android()

# First argument is the Bluetooth SPP UUID.
BTconnect = droid.bluetoothConnect('00001101-0000-1000-8000-00805F9B34FB',
                                   BT_DEVICE_ADDR)
if (BTconnect.error is None):
    # A little bit of testing showed that waiting
    # 0.3 seconds was just long enough to ensure
    # the device had settled after connecting.
    time.sleep(0.3)
    droid.bluetoothWrite(sendStr)
    droid.makeToast("Sent \"%s\" command." % sendStr)
    droid.bluetoothStop()
else:
    print (BTconnect)
    droid.makeToast("Unable to connect.")
