import serial
import time

# open a file
fp = open("log.txt", "w")

# XBee setting
serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev, 9600)

while(1):
    line = s.readline().decode()
    fp.write(line)
    print(line)
    if (line == "END\r\n"):
        break;

fp.close()
