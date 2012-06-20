"""lampcontrol.py

Android SL4A script to send control codes to an
arduino lamp via a Bluetooth serial connection.

Peter Hardy <peter@hardy.dropbear.id.au>
"""

import android
import sys, time

droid = android.Android()

try:
    LAMP_CMD = int(droid.getIntent().result[u"extras"][u"%LAMP_CMD"])
except:
    # droid.makeToast("LAMP_CMD missing.")
    # sys.exit(1)
    LAMP_CMD = 2
try:
    LAMP_ARGS = droid.getIntent().result[u"extras"][u"%LAMP_ARGS"]
except:
    LAMP_ARGS = None

# cheat and hard code this for now...
BT_DEVICE_ADDR = "00:06:66:46:5B:B2" # The address of the device to send to.

droid = android.Android()

# First argument is the Bluetooth SPP UUID.
BTconnect = droid.bluetoothConnect('00001101-0000-1000-8000-00805F9B34FB',
                                   BT_DEVICE_ADDR)
if (BTconnect.error is None):
    connID = BTconnect.id
    if LAMP_CMD == 1:
        sendStr = "A"
    elif LAMP_CMD == 2:
        sendStr = "a"
    elif LAMP_CMD == 3:
        sendStr = "B"
    else:
        sendStr = None
        droid.makeToast("LAMP_CMD \"%s\" is invalid." % LAMP_CMD)

    if (sendStr):
        # A little bit of testing showed that waiting
        # 0.3 seconds was just long enough to ensure
        # the device had settled after connecting.
        time.sleep(0.3)
        droid.bluetoothWrite(sendStr)
        droid.makeToast("Sent \"%s\" command." % LAMP_CMD)
    droid.bluetoothStop()
else:
    print (BTconnect)
    droid.makeToast("Unable to connect.")
