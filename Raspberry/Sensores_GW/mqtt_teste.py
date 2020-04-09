import paho.mqtt.client as mqtt
import threading, time
from datetime import datetime
import smbus
import time
import bme280
import board
import busio
import adafruit_ads1x15.ads1115 as ADS
from adafruit_ads1x15.analog_in import AnalogIn

address = 0x76
bus = smbus.SMBus(1)
bme280.load_calibration_params(bus,address)

#endereço: 0x1D, offset: 0x16, data: 0x01
bus.write_byte_data(0x1D, 0x16, 0x01)

i2c = busio.I2C(board.SCL, board.SDA)    
ads = ADS.ADS1115(i2c)

# Timer variable for periodic function
next_call = 0

# Period for publishing data to the MQTT broker in seconds
timePeriod = 30

temperature = 0
humidity = 0
lumin = 0
xAcc = 0
yAcc = 0
zAcc = 0
date_temp = 0
date_humid = 0
date_lumin = 0
date_vibr = 0

def calculateTemp():
    global temperature
    global date_temp
    
    bme280_data = bme280.sample(bus, address)
    temperature = bme280_data.temperature
    now = datetime.now() # current date and time
    date_temp = now.strftime("%d/%m/%Y  %H:%M:%S")
    temperature = round(temperature,2)

def calculateHumid():
    global humidity
    global date_humid
    
    bme280_data = bme280.sample(bus, address)
    humidity = bme280_data.humidity
    now = datetime.now() # current date and time
    date_humid = now.strftime("%d/%m/%Y  %H:%M:%S")
    humidity = round(humidity,2)

def calculateLumin():
    global lumin
    global date_lumin
    
    chan = AnalogIn(ads, ADS.P0)
    lumin = (chan.voltage/3.3)*100
    now = datetime.now() # current date and time
    date_lumin = now.strftime("%d/%m/%Y  %H:%M:%S")
    lumin = round(lumin,2)

def calculateVibrations():
    global xAcc
    global yAcc
    global zAcc
    global date_vibrations
    
    data=bus.read_i2c_block_data(0x1D, 0x00, 6)
    now = datetime.now() # current date and time
    date_vibrations = now.strftime("%d/%m/%Y  %H:%M:%S")

    #Convert the data to 10-bits
    xAcc = (data[1] & 0x03)*256 + data[0]
    if xAcc > 511:
        xAcc -= 1024

    yAcc = (data[3] & 0x03)*256 + data[2]
    if yAcc > 511:
        yAcc -= 1024

    zAcc = (data[5] & 0x03)*256 + data[4]
    if zAcc > 511:
        zAcc -= 1024

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    global next_call
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("resend_temp")
    client.subscribe("resend_humid")
    client.subscribe("resend_lumin")
    client.subscribe("resend_vibrations")
    
    # Time stamp when the periodAction function shall be called again
    next_call = time.time()
    
    # Start the periodic task for publishing MQTT messages
    periodicAction()

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    if msg.topic == "resend_temp":
        calculateTemp()
        client.publish("temp", temperature)
        print("Temperatura reenviada!")
        
    elif msg.topic == "resend_humid":
        calculateHumid()
        client.publish("humid", humidity)
        print("Humidade reenviada!")
    
    elif msg.topic == "resend_lumin":
        calculateLumin()
        client.publish("lumin", lumin)
        print("Luminosidade reenviada!")
        print()
    
    elif msg.topic == "resend_vibrations":    
        calculateVibrations()        
        client.publish("xAcc", xAcc)
        print("Aceleração X reenviada!")
        client.publish("yAcc", yAcc)
        print("Aceleração Y reenviada!")
        client.publish("zAcc", zAcc)
        print("Aceleração Z reenviada!")
        
def periodicAction():
    global next_call
    global temperature
    global humidity
    global lumin
    global xAcc
    global yAcc
    global zAcc

    ### BEGIN READING SENSOR DATA
    
    print("joao")
    calculateTemp()
    calculateHumid()
    
    bme280_data = bme280.sample(bus, address)
    pressure = bme280_data.pressure
    
    calculateLumin()
    calculateVibrations() 
    
    ### END READING SENSOR DATA
    
    print()
    client.publish("temp", temperature)
    print("Temperatura publicada!")
    client.publish("humid", humidity)
    print("Humidade publicada!")
    client.publish("lumin", lumin)
    print("Luminosidade publicada!")
    client.publish("xAcc", xAcc)
    print("Aceleração X publicada!")
    client.publish("yAcc", yAcc)
    print("Aceleração Y publicada!")
    client.publish("zAcc", zAcc)
    print("Aceleração Z publicada!")
    print()
    
#    client.publish("date_temp", date_temp)
#    print("Data temperatura publicada!")
#    client.publish("date_humid", date_humid)
#    print("Data humidade publicada!")
#    client.publish("date_ilumin", date_ilumin)
#    print("Data iluminação publicada!")
#    client.publish("date_vibr", date_vibrations)
#    print("Data vibrações publicada!")
    print()
    
    # Schedule next call
    next_call = next_call + timePeriod
    threading.Timer(next_call - time.time(), periodicAction).start()

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect("mqtt.eclipse.org", 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_forever()