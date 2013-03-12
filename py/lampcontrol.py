"""lampcontrol.py

Android SL4A script to send control codes to an
arduino lamp via a Bluetooth serial connection.

Peter Hardy <peter@hardy.dropbear.id.au>
"""

import android
import sys, time

droid = android.Android()

try:
    lamp_cmd = int(droid.getIntent().result[u"extras"][u"%lamp_cmd"])
except:
    # droid.makeToast("LAMP_CMD missing.")
    # sys.exit(1)
    lamp_cmd = 2
try:
    lamp_args = droid.getIntent().result[u"extras"][u"%lamp_args"]
except:
    lamp_args = None

# cheat and hard code this for now...
BT_DEVICE_ADDR = "00:06:66:46:5B:B2" # The address of the device to send to.

droid = android.Android()

# First argument is the Bluetooth SPP UUID.
BTconnect = droid.bluetoothConnect('00001101-0000-1000-8000-00805F9B34FB',
                                   BT_DEVICE_ADDR)
if (BTconnect.error is None):
    connID = BTconnect.id
    if lamp_cmd == 1:
        sendStr = "on;"
    elif lamp_cmd == 2:
        sendStr = "off;"
    elif lamp_cmd == 3:
        sendStr = "blink, 15;"
    elif lamp_cmd == 4:
        sendStr = "mood;"
    elif lamp_cmd == 5:
        sendStr = "timer, 1800;"
    else:
        sendStr = None
        droid.makeToast("lamp_cmd \"%s\" is invalid." % LAMP_CMD)

    if (sendStr):
        # A little bit of testing showed that waiting
        # 0.3 seconds was just long enough to ensure
        # the device had settled after connecting.
        time.sleep(0.3)
        droid.bluetoothWrite(sendStr)
        droid.makeToast("Sent \"%s\" command." % lamp_cmd)
    droid.bluetoothStop()
else:
    print (BTconnect)
    droid.makeToast("Unable to connect.")
