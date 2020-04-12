import matplotlib.pyplot as plt
import numpy as np
import serial
import time

sampletime = 100  # total sampling
Ts = 0.1
t = np.arange(0,10,Ts) # time vector; create Fs samples between 0 and 1.0 sec.
x = np.arange(0,10,Ts)
y = np.arange(0,10,Ts)
z = np.arange(0,10,Ts)
tilt = np.arange(0,10,Ts)

serdev = '/dev/ttyACM0'
s = serial.Serial(serdev)

for i in range(0, sampletime):
    line = s.readline() # Read an echo string from K66F terminated with '\n'
    x[i] = float(line)
    line = s.readline() 
    y[i] = float(line)    
    line = s.readline() 
    z[i] = float(line)    
    line = s.readline() 
    tilt[i] = float(line)

fig, ax = plt.subplots(2, 1)

ax[0].plot(t,x, label = 'x')
ax[0].plot(t,y, label = 'y')
ax[0].plot(t,z, label = 'z')
ax[0].set_xlabel('Time')
ax[0].set_ylabel('Acc Vector')
ax[0].legend()

ax[1].stem(t,tilt)
ax[1].set_xlabel('Time')
ax[1].set_ylabel('Tilt')

plt.show()

s.close()
