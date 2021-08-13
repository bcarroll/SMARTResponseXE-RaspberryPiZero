#!/usr/bin/env python3

import socket
import serial
import time

baud_rate = 9600

srxe = serial.Serial('/dev/serial0', baud_rate, timeout=1)
srxe.flush()

def getIp():
  s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
  s.connect(("8.8.8.8", 80))
  ip=s.getsockname()[0]
  s.close()
  return ip

ip = getIp()
while True:
  data = srxe.readline().decode('utf-8').rstrip()
  if data:
    print(data)
  srxe.write(b"Raspberry\n")
  time.sleep(1)
