#!/bin/bash
sudo avrdude -v -p atmega128rfa1 -C /home/pi/avrdude_gpio.conf -c pi_1 -U flash:w:$1:i
