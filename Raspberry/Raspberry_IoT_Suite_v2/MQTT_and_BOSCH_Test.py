import netifaces
import paho.mqtt.client as mqtt
import datetime, threading, time
import model.functionblock.Acceleration as acceleration 
import model.functionblock.Humidity as humidity 
import model.functionblock.Illuminance as illuminance 
import model.functionblock.Temperature as temperature 
import model.infomodel.RaspberryPi3v10000 as RaspberryPi3v10000
import model.DittoSerializer as DittoSerializer

# DEVICE CONFIG GOES HERE
tenantId = "t5f7ed5ae8357493c914ff61b8287a3e9_hub"
device_password = "Tartaruga5*"
hub_adapter_host = "mqtt.bosch-iot-hub.com"
deviceId = "joao.Tese.Devices:Raspberry_as_Gateway"
clientId = deviceId
authId = "joao.Tese.Devices_Raspberry_as_Gateway"
certificatePath = "/home/pi/Desktop/Raspberry_IoT_Suite_v2/iothub.crt"
ditto_topic = "joao.Tese.Devices/Raspberry_as_Gateway"
username = authId + "@" + tenantId

# Initialization of Information Model
infomodel = RaspberryPi3v10000.RaspberryPi3v10000()

# Create a serializer for the MQTT payload from the Information Model
ser = DittoSerializer.DittoSerializer()

# Timer variable for periodic function
next_call = 0

# Period for publishing data to the MQTT broker in seconds
timePeriod = 300

# Configuration of client ID and publish topic  
publishTopic = "telemetry/" + tenantId + "/" + deviceId

# Create the MQTT client
client = mqtt.Client(clientId)
client_mosquitto = mqtt.Client()

temperature = 0
humidity = 0
lumin = 0
xAcc = 0
yAcc = 0
zAcc = 0

# The callback for when the client receives a CONNACK response from the server.
def on_connect_mqtt(client, userdata, flags, rc):

    if rc != 0:
        print("Connection to MQTT broker failed: " + str(rc))
        return

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    
    client_mosquitto.subscribe("temp")
    client_mosquitto.subscribe("humid")
    client_mosquitto.subscribe("lumin")
    client_mosquitto.subscribe("xAcc")
    client_mosquitto.subscribe("yAcc")
    client_mosquitto.subscribe("zAcc")


# The callback for when the client receives a CONNACK response from the server.
def on_connect_mqtt_bosch(client, userdata, flags, rc):
    global next_call

    if rc != 0:
        print("Connection to MQTT broker failed: " + str(rc))
        return

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    
    # BEGIN SAMPLE CODE
    client.subscribe("command/+/+/req/#")
    # END SAMPLE CODE    

    # Time stamp when the periodAction function shall be called again
    next_call = time.time()
    
    # Start the periodic task for publishing MQTT messages
    periodicAction()

     
# The function that will be executed periodically once the connection to the MQTT broker was established
def periodicAction():
    global next_call
    global temperature
    global humidity
    global lumin
    global xAcc
    global yAcc
    global zAcc
  
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
        "currentMeasured" : temperature,
        "minMeasured" : 0,
        "maxMeasured" : 0
    }

    # Publish payload
    publishAcceleration()
    publishHumidity()
    publishIlluminance()
    publishTemperature()
    print()
    x = datetime.datetime.now()
    print(x)
    print()

    # Schedule next call
    next_call = next_call + timePeriod
    #t = threading.Timer(next_call - time.time(), periodicAction)
    t = threading.Timer(300, periodicAction)
    t.start()


# The callback for when a PUBLISH message is received from the server.
def on_message_mqtt(client, userdata, msg):
    global temperature
    global humidity
    global lumin
    global xAcc
    global yAcc
    global zAcc
    """
    Function to run on when message is received
    
    Args:
        client: MQTT client
        userdata: User who published the message on broker
        msg: MQTT message payload

    """
        
    print("Message received:")
    print(msg.topic + " " + str(msg.payload))
    
    
    if (msg.topic == "temp"):    
        temperature_str = msg.payload
        temperature = float(temperature_str)
    
    elif (msg.topic == "humid"):
        humidity_str = msg.payload
        humidity = float(humidity_str)
        
    elif (msg.topic == "lumin"):
        lumin_str = msg.payload
        lumin = float(lumin_str)
        
    elif (msg.topic == "xAcc"):
        xAcc_str = msg.payload
        xAcc = float(xAcc_str)
    
    elif (msg.topic == "yAcc"):
        yAcc_str = msg.payload
        yAcc = float(yAcc_str)
    
    elif (msg.topic == "zAcc"):
        zAcc_str = msg.payload
        zAcc = float(zAcc_str)


# The callback for when a PUBLISH message is received from the server.
def on_message_mqtt_bosch(client, userdata, msg):
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
        resTopic = "command///res/" + messageId + "/200"

        # parse Bosch IoT Things correlation ID for response
        reqPayload = str(msg.payload)

        # 17 is the length of 'correlation-id' and subsequent '":"'
            
        correlationId = reqPayload[reqPayload.find("correlation-id")+17:reqPayload.find("correlation-id")+17+36]

        print("Sender expects reply, responding to message with Correlation-Id: " + correlationId)

        # create Ditto compliant MQTT response payload
        resPayload = "{\"topic\":\"" + ditto_topic + "/things/live/messages/switch\","
        resPayload += "\"headers\":{\"correlation-id\":\"" + correlationId + "\","
        resPayload += "\"version\":2,\"content-type\":\"text/plain\"},"
        resPayload += "\"path\":\"/inbox/messages/switch\","
        resPayload += "\"value\":\"" + "Alive" + "\","
        resPayload += "\"status\": 200 }"

        client.publish(resTopic, resPayload)
        
        print("Response published!")
        print()


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
    

client.on_connect = on_connect_mqtt_bosch
client.on_message = on_message_mqtt_bosch
    
client_mosquitto.on_connect = on_connect_mqtt
client_mosquitto.on_message = on_message_mqtt 

# Output relevant information for consumers of our information
print("Connecting client:    ", clientId)
print("Publishing to topic:  ", publishTopic)
print()

client.username_pw_set(username, device_password)
client.tls_set(certificatePath)

# Connect to the MQTT broker
client.connect(hub_adapter_host, 8883, 30)
client_mosquitto.connect('localhost', 1883, 60)

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.
client.loop_start()
client_mosquitto.loop_start()
    
while (1):
    pass