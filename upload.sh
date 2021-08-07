#!/bin/bash
sudo avrdude -v -p atmega128rfa1 -C /home/pi/avrdude_gpio.conf -c pi_1 -U flash:w:build/SMART-Response-XE-Low_level.avr.smartxeo/SMART_Response_XE_-_Raspberry_Pi_Zero_W.ino.hex:i
