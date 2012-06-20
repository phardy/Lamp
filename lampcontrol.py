"""lampcontrol.py

Android code to send control codes to an arduino lamp
via a Bluetooth serial connection.
"""

import android

try:
    LAMP_CMD = droid.getIntent().result[u"extras"][u"%LAMP_CMD"]
except:
    droid.makeToast("LAMP_CMD missing.")
try:
    LAMP_ARGS = droid.getIntent().result[u"extras"][u"%LAMP_ARGS"]
except:
    LAMP_ARGS = None

# cheat and hard code this for now...
BT_DEVICE_ADDR = "DB:B3:77:07:F9:2F" # The address of the device to send to.

droid = android.Android()

# First argument is the Bluetooth SPP UUID.
BTconnect = droid.bluetoothConnect('00001101-0000-1000-8000-00805F9B34FB',
                                   BT_DEVICE_ADDR)
if (BTconnect):
    if LAMP_CMD is "on":
        sendStr = "A"
    elif LAMP_CMD is "off":
        sendStr = "a"
    elif LAMP_CMD is "timer":
        sendStr = "B"
    else:
        sendStr = None
        droid.makeToast("LAMP_CMD is invalid.")

    if (sendStr):
        droid.bluetoothWrite(LAMP_CMD)

    # Not sure if want.
    # droid.bluetoothStop()
else:
    droid.makeToast("Unable to connect.")
