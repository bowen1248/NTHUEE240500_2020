import numpy as np
import serial
import time

waitTime = 0.1
# generate the waveform table
song_length = 7
custom_song_note = [261, 261, 392, 392, 440, 440, 392]
custom_note_length = [ 1, 1, 1, 1, 1, 1, 2]



# output formatter
formatter = lambda x: "%4d" % x

# send the waveform table to K66F
serdev = '/dev/ttyACM0'
s = serial.Serial(serdev, 9600)
print("Sending signal ...")
#print("It may take about %d seconds ..." % (int(song_length * waitTime * 2)))
s.write(bytes(formatter(song_length), 'UTF-8'))
#time.sleep(waitTime)
for note in custom_song_note:
  s.write(bytes(formatter(note), 'UTF-8'))
  #time.sleep(waitTime)
for data in custom_note_length:
  s.write(bytes(formatter(data), 'UTF-8'))
  #time.sleep(waitTime)
print("Signal sended")
s.close()
