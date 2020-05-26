import serial

import time


# XBee setting

serdev = '/dev/ttyUSB0'

s = serial.Serial(serdev, 9600, timeout=3)


s.write('+++'.encode())

char = s.read(3)

print('Enter AT mode.')

print(char.decode())


s.write('ATMY 0x0\r\n'.encode())

char = s.read(3)

print('Set MY 0x0.')

print(char.decode())


s.write('ATDL 0xFFFF\r\n'.encode())

char = s.read(3)

print('Set DL 0xFFFF.')

print(char.decode())


s.write('ATID <PAN_ID>\r\n'.encode())

char = s.read(3)

print('Set PAN ID <PAN_ID>.')

print(char.decode())


s.write('ATWR\r\n'.encode())

char = s.read(3)

print('Write config.')

print(char.decode())


s.write('ATCN\r\n'.encode())

char = s.read(3)

print('Exit AT mode.')

print(char.decode())


print('')

print('Start Communication')

send = 'start'

while send!='stop':

    print('Which one do you want?')

    send = input()

    if (send=='a' or send == 'b'):

        # send to remote

        s.write(send.encode())

        line = s.read(20)

        if len(line) < 20:

            print('No response')

        else:

            print(line.decode())

        print('')


s.close()