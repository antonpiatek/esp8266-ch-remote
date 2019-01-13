# esp8266-ch-remote
Control panel to go with https://github.com/antonpiatek/esp8266-ch-controller

Basic idea was a 16x2 LCD to show temperatures around the house, a 4 digit 7-segment display to show how long the boiler/heating timers have left, both of these as i2c devices. Also two LEDs to show the boiler/heating are on or not.
ESP8266 subscribes to a topic and waits for json to update devices. Also has two buttons which when pressed trigger a mqtt PUB to turn on hot water/heating.

Copy sample_config.h to new file config.h and edit

