import serial
import time
# XBee setting
serdev = '/dev/ttyUSB0'

s = serial.Serial(serdev, 9600, timeout = 5)

s.write("+++".encode())
char = s.read(2)
print("Enter AT mode.")
print(char.decode())

s.write("ATMY 0x140\r\n".encode())
char = s.read(3)
print("Set MY 0x140.")
print(char.decode())


s.write("ATDL 0x141\r\n".encode())
char = s.read(3)
print("Set DL 0x141.")
print(char.decode())

s.write("ATID 0x3\r\n".encode())
char = s.read(3)
print("Set PAN ID 0x3.")
print(char.decode())

s.write("ATWR\r\n".encode())
char = s.read(3)
print("Write config.")
print(char.decode())

s.write("ATMY\r\n".encode())
char = s.read(4)
print("MY :")
print(char.decode())

s.write("ATDL\r\n".encode())
char = s.read(4)
print("DL : ")
print(char.decode())

s.write("ATCN\r\n".encode())
char = s.read(3)
print("Exit AT mode.")
print(char.decode())

time.sleep(0.5)
# send to remote
package = "abcdefghijklmnop\r\n"
s.write(package.encode())
#s.write("abcd".encode())
print("abcd\r\n")
line = s.read(10)
print('Get:', line.decode())
s.close()
