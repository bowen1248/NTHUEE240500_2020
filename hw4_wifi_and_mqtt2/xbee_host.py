import serial
import time
import matplotlib.pyplot as plt
import numpy as np
import paho.mqtt.client as paho
import time


# XBee setting
serdev = '/dev/ttyUSB0'

s = serial.Serial(serdev, 9600, timeout = 5)
t1 = np.arange(0, 20, 1)
t2 = np.arange(0, 20, 0.5)
sample_times = np.arange(0, 20, 1)
x_acc = np.arange(0, 20, 0.5)
y_acc = np.arange(0, 20, 0.5)
z_acc = np.arange(0, 20, 0.5)

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

package = "/get_sample_num/run\r"
s.write(package.encode())
print("send rpc\r\n")

for i in range(0,20):
    time.sleep(1)
    package = "/get_sample_num/run\r"
    s.write(package.encode())
    print("send rpc")
    line = s.readline()
    sample_times[i] = int(line.decode())
    print(sample_times[i])

for i in range(0,40):
    line = s.readline()
    x_acc[i] = int(line.decode()) / 4096
    print(x_acc[i])
    line = s.readline()
    y_acc[i] = int(line.decode()) / 4096
    print(y_acc[i])
    line = s.readline()
    z_acc[i] = int(line.decode()) / 4096
    print(z_acc[i])


mqttc = paho.Client()

# Settings for connection
host = "localhost"
topic= "Mbed"
port = 1883

# Callbacks
def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))

def on_message(mosq, obj, msg):
    print("[Received] Topic: " + msg.topic + ", Message: " + str(msg.payload) + "\n");

def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
    print("Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# Connect and subscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
for i in range(0,40):
    mesg = str(x_acc[i])
    mqttc.publish(topic, mesg)
    time.sleep(0.1)
    mesg = str(y_acc[i])
    mqttc.publish(topic, mesg)
    time.sleep(0.1)
    mesg = str(z_acc[i])
    mqttc.publish(topic, mesg)
    time.sleep(0.1)

fig, ax = plt.subplots(1, 1)
ax.plot(t1,sample_times)
ax.set_xlabel('Time')
ax.set_ylabel('Sample time')
plt.show()
s.close()
