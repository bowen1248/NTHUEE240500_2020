import serial
import time

# XBee setting
serdev = '/dev/ttyUSB0'
s = serial.Serial(serdev, 9600)

# send to remote
package = "abcdefghijklmnop\r\n"
s.write(package.encode())
print("abcd\r")
line = s.read(10)
print('Get:', line.decode())
s.close()
