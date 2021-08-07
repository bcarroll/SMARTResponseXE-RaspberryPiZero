
---

### Raspberry Pi optimization

#### Disable rainbow screen when the Pi boots
- Add `disable_splash=1` to /boot/config.txt

- Add ` consoleblank=1 logo.nologo quiet loglevel=0 plymouth.enable=0 vt.global_cursor_default=0 plymouth.ignore-serial-consoles splash fastboot noatime nodiratime noram` to the end of `/boot/cmdline.txt`
  
  **Note:** `/boot/cmdline.txt` *contains a single line of arguments*

#### Enable and create ZRAM swap
```
git clone https://github.com/foundObjects/zram-swap.git
cd zram-swap && sudo ./install.sh
```

#### Add kernel parameters to make better use of ZRAM
 - Add the following lines to the end of `/etc/sysctl.conf`
   ```
   vm.vfs_cache_pressure=500
   vm.swappiness=100
   vm.dirty_background_ratio=1
   vm.dirty_ratio=50
   ```

Reboot to enable the changes

---

### References:
 - [arduino-cli documentation](https://arduino.github.io/arduino-cli/latest/)
 - [Sparkfun ATMEGA128RF1 Arduino board URL](https://raw.githubusercontent.com/sparkfun/Arduino_Boards/main/IDE_Board_Manager/package_sparkfun_index.json)
 - [Program Arduino via ISP using Raspberry Pi GPIO](https://learn.adafruit.com/program-an-avr-or-arduino-using-raspberry-pi-gpio-pins)
 - [Raspberry Pi Zero GPIO Pinout](https://www.etechnophiles.com/raspberry-pi-zero-gpio-pinout-specifications-programming-language/)
 - [SMART-Response-XE-Low_level](https://github.com/fdufnews/SMART-Response-XE-Low_level)
