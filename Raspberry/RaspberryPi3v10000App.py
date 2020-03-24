import netifaces
import paho.mqtt.client as mqtt
import datetime, threading, time
import model.functionblock.Acceleration as acceleration 
import model.functionblock.Humidity as humidity 
import model.functionblock.Illuminance as illuminance 
import model.functionblock.Temperature as temperature 
import model.infomodel.RaspberryPi3v10000 as RaspberryPi3v10000
import model.DittoSerializer as DittoSerializer

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

# DEVICE CONFIG GOES HERE
tenantId = "t0959f330dded42e2b42bb3d8a08619cd_hub"
device_password = "Tartaruga5*"
hub_adapter_host = "mqtt.bosch-iot-hub.com"
deviceId = "joao.RaspberryPi3v3.Tese:RaspberryPi3v1000"
clientId = deviceId
authId = "joao.RaspberryPi3v3.Tese_RaspberryPi3v1000"
certificatePath = "/home/pi/Desktop/Raspberry_IoT_Suite/iothub.crt"
ditto_topic = "joao.RaspberryPi3v3.Tese/RaspberryPi3v1000"
username = authId + "@" + tenantId

# Initialization of Information Model
infomodel = RaspberryPi3v10000.RaspberryPi3v10000()

# Create a serializer for the MQTT payload from the Information Model
ser = DittoSerializer.DittoSerializer()

# Timer variable for periodic function
next_call = 0

# Period for publishing data to the MQTT broker in seconds
timePeriod = 30

# Configuration of client ID and publish topic  
publishTopic = "telemetry/" + tenantId + "/" + deviceId

# Create the MQTT client
client = mqtt.Client(clientId)

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    global next_call

    if rc != 0:
        print("Connection to MQTT broker failed: " + str(rc))
        return

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.

    # BEGIN SAMPLE CODE
    #client.subscribe("commands/" + tenantId + "/")
    client.subscribe("control/+/+/req/#")
    # END SAMPLE CODE

    # Time stamp when the periodAction function shall be called again
    next_call = time.time()
    
    # Start the periodic task for publishing MQTT messages
    periodicAction()
     
# The function that will be executed periodically once the connection to the MQTT broker was established
def periodicAction():
    global next_call

    ### BEGIN READING SENSOR DATA
    bme280_data = bme280.sample(bus, address)
    humidity = bme280_data.humidity
    pressure = bme280_data.pressure
    ambient_temperature = bme280_data.temperature
    
    data=bus.read_i2c_block_data(0x1D, 0x00, 6)

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

    chan = AnalogIn(ads, ADS.P0)
    lumin = (chan.voltage/3.3)*100
    
    infomodel.acceleration.value = {
        "x" : xAcc,
        "y" : yAcc,
        "z" : zAcc
    }
    infomodel.humidity.value = {
        "currentMeasured" : humidity,
        "minMeasured" : 0,
        "maxMeasured" : 0
    }
    infomodel.illuminance.value = {
        "currentMeasured" : lumin,
        "minMeasured" : 0,
        "maxMeasured" : 0
    }
    infomodel.temperature.value = {
        "currentMeasured" : ambient_temperature,
        "minMeasured" : 0,
        "maxMeasured" : 0
    }

    ### END READING SENSOR DATA

    # Publish payload
    publishAcceleration()
    publishHumidity()
    publishIlluminance()
    publishTemperature()

    # Schedule next call
    next_call = next_call + timePeriod
    threading.Timer(next_call - time.time(), periodicAction).start()

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    
    """
    Function to run on when message is received
    
    Args:
        client: MQTT client
        userdata: User who published the message on broker
        msg: MQTT message payload

    """
        
    print("Message received:")
    print(msg.topic + " " + str(msg.payload))

    # parse Bosch IoT Hub's message ID for response
    messageId = msg.topic[msg.topic.find("req/")+4:]
    messageId = messageId[0:messageId.find("/")]

    # if this is a 2-way command, respond
    if (messageId != ""):
        print("Sender expects reply, responding to message with ID: " + messageId)

        # create MQTT response topic
        resTopic = "control///res/" + messageId + "/200"

        # parse Bosch IoT Things correlation ID for response
        reqPayload = str(msg.payload)

        # 17 is the length of 'correlation-id' and subsequent '":"'
            
        correlationId = reqPayload[reqPayload.find("correlation-id")+17:reqPayload.find("correlation-id")+17+36]

        print("Sender expects reply, responding to message with Correlation-Id: " + correlationId)
        
        #periodicAction()

        # create Ditto compliant MQTT response payload
        resPayload = "{\"topic\":\"" + ditto_topic + "/things/live/messages/switch\","
        resPayload += "\"headers\":{\"correlation-id\":\"" + correlationId + "\","
        resPayload += "\"version\":2,\"content-type\":\"text/plain\"},"
        resPayload += "\"path\":\"/inbox/messages/switch\","
        resPayload += "\"value\":\"" + "Alive" + "\","
        resPayload += "\"status\": 200 }"

        client.publish(resTopic, resPayload)
        
        print("Response published!")

# The functions to publish the functionblocks data
def publishAcceleration():
    payload = ser.serialize_functionblock("acceleration", infomodel.acceleration, ditto_topic, deviceId)
    print("Publish Payload: ", payload, " to Topic: ", publishTopic)
    client.publish(publishTopic, payload)
def publishHumidity():
    payload = ser.serialize_functionblock("humidity", infomodel.humidity, ditto_topic, deviceId)
    print("Publish Payload: ", payload, " to Topic: ", publishTopic)
    client.publish(publishTopic, payload)
def publishIlluminance():
    payload = ser.serialize_functionblock("illuminance", infomodel.illuminance, ditto_topic, deviceId)
    print("Publish Payload: ", payload, " to Topic: ", publishTopic)
    client.publish(publishTopic, payload)
def publishTemperature():
    payload = ser.serialize_functionblock("temperature", infomodel.temperature, ditto_topic, deviceId)
    print("Publish Payload: ", payload, " to Topic: ", publishTopic)
    client.publish(publishTopic, payload)

def main():
    client.on_connect = on_connect
    client.on_message = on_message

    # Output relevant information for consumers of our information
    print("Connecting client:    ", clientId)
    print("Publishing to topic:  ", publishTopic)

    client.username_pw_set(username, device_password)
    client.tls_set(certificatePath)

    # Connect to the MQTT broker
    client.connect(hub_adapter_host, 8883, 60)

    # Blocking call that processes network traffic, dispatches callbacks and
    # handles reconnecting.
    # Other loop*() functions are available that give a threaded interface and a
    # manual interface.
    client.loop_start()
    
    while (1):
        pass

if __name__ == '__main__':
    main()