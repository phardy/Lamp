#!/usr/bin/python
#
# A GUI for sending random RGB values to a lamp.
# Depends on python-glade and python-serial.
#
# Peter Hardy <peter@hardy.dropbear.id.au>

from gi.repository import Gdk, Gtk
import serial

class ColorSender:
    def __init__(self):
        self.serialport = "/dev/rfcomm0"
        self.conn = None
        self.builder = Gtk.Builder()
        self.builder.add_from_file("ColorSender.glade")
        self.builder.connect_signals(self)
        window = self.builder.get_object("window1")
        window.show_all()

        self.updateSerialPort()

    def exitApp(self, *args):
        Gtk.main_quit(*args)

    def updateSerialPort(self, *args):
        textbox = self.builder.get_object("serialportentry")
        newport = textbox.get_text()
        statusbar = self.builder.get_object("statusbar1")
        contextid = statusbar.get_context_id("Serial Port status")
        statusbar.pop(contextid)
        try:
            newconn = serial.Serial(newport, 115200)
        except serial.serialutil.SerialException:
            statusbar.push(contextid, "Unable to open serial port (%s)." % newport)
            return
        if self.conn is not None:
            self.conn.close()
        self.conn = newconn

    def updateColor(self, *args):
        cs = self.builder.get_object("colorselection1")
        color = cs.get_current_rgba()
        red = int(color.red * 255)
        green = int(color.green * 255)
        blue = int(color.blue * 255)
        colorval = "%02x%02x%02x" % (red, green, blue)
        # print("updateColor: %s" % colorval);
        if self.conn is not None and self.conn.isOpen():
                self.conn.write("on %s;" % colorval)

colorsender = ColorSender()
Gtk.main()
