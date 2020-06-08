import time
import matplotlib.pyplot as plt
import numpy as np
import paho.mqtt.client as paho

data = np.arange(0, 80, 0.5)
t = np.arange(0, 20, 0.5)
x_acc = np.arange(0, 20, 0.5)
y_acc = np.arange(0, 20, 0.5)
z_acc = np.arange(0, 20, 0.5)
tilt = np.arange(0, 20, 0.5)

# Settings for connection
mqttc = paho.Client()
host = "localhost"
topic= "Mbed"
port = 1883

# Callbacks
def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))

# When get message
def on_message(mosq, obj, msg):
    #print("[Received] Topic: " + msg.topic + ", Message: " + str(msg.payload) + "\n")
    # get message
    data = str(msg.payload)
    # split message to each array
    data = data.split()
    # dispatch bad data
    x_acc[0] = 0
    for i in range(1, 40):
        x_acc[i] = float(data[i])
    for i in range(0, 40):
        y_acc[i] = float(data[i + 40])
    for i in range(0, 40):
        z_acc[i] = float(data[i + 80])
    for i in range(0, 39):
        if float(data[i + 120]) == 1.5:
            tilt[i] = 1
        else: tilt[i] = 0
    tilt[39] = 0
    # draw figure
    fig, ax = plt.subplots(2, 1)
    ax[0].plot(t, x_acc, label = 'x')
    ax[0].plot(t, y_acc, label = 'y')
    ax[0].plot(t, z_acc, label = 'z')
    ax[0].set_ylabel('Acc Vector')
    ax[0].set_xlabel('Time')
    ax[0].legend()     # add legend
    ax[1].stem(t, tilt, use_line_collection = True)
    ax[1].set_ylabel('Tilt')
    ax[1].set_xlabel('Time')
    plt.show()
    
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
mqttc.subscribe(topic, 0)
mqttc.loop_forever()

