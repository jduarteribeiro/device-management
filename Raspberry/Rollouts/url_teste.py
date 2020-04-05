import requests
import json

headers = {
    'Accept': 'application/hal+json',
    'Authorization': 'TargetToken 9c836a4af349b0c9076fc85be268da92',
}

print("URL_1: https://device.eu1.bosch-iot-rollouts.com/D60F7F26-9AFC-4FFB-8FA5-00A6FC6D774A/controller/v1/rasp01")
response_1 = requests.get('https://device.eu1.bosch-iot-rollouts.com/D60F7F26-9AFC-4FFB-8FA5-00A6FC6D774A/controller/v1/rasp01', headers=headers)
status_1 = response_1.status_code
print("Resposta_1:",status_1)

print("             |")
print("             v")

url_2 = json.loads(json.dumps(json.loads(response_1.text)))["_links"]["deploymentBase"]["href"]
print("URL_2:",url_2)

response_2 = requests.get(url_2, headers=headers)
status_2 = response_2.status_code
print("Resposta_2:",status_2)

print("             |")
print("             v")

device_id = json.loads(json.dumps(json.loads(response_2.text)))["id"]
print("Device_id:", device_id)
url = json.loads(response_2.text)
url = json.loads(json.dumps(url))["deployment"]["chunks"]
url = json.loads(json.dumps(url))[0]["artifacts"]
url_3 = json.loads(json.dumps(url))[0]["_links"]["download"]["href"]
print("URL_3:",url_3)

response_3 = requests.get(url_3, headers=headers)
status_3 = response_3.status_code
print("Resposta_3:",status_3)

print("             |")
print("             v")

file_content = response_3.text
print(file_content)

print()
url_4 = 'https://device.eu1.bosch-iot-rollouts.com/D60F7F26-9AFC-4FFB-8FA5-00A6FC6D774A/controller/v1/rasp01/deploymentBase/' + device_id +'/feedback'
print("URL_4:",url_4)

headers_4 = {
    'Content-Type': 'application/json;charset=UTF-8',
    'Accept': 'application/hal+json',
    'Authorization': 'TargetToken 9c836a4af349b0c9076fc85be268da92',
}

#data = '{\n  "id" :'+device_id+',\n  "time" : "20200405T020314",\n  "status" : {\n    "result" : {\n      "progress" : {\n        "of" : 5,\n        "cnt" : 2\n      },\n      "finished" : "none"\n    },\n    "execution" : "closed",\n    "details" : [ "Feddback message" ]\n  }\n}'
data = '{\n  "id" :'+device_id+',\n  "status" : {\n    "result" : {\n      "finished" : "success"\n    },\n    "execution" : "closed",\n    "details" : [ ]\n  }\n}'
#response_4 = requests.post(url_4, headers=headers_4, data=data)
response_4 = requests.post(url_4, headers=headers_4, data=data)
status_4 = response_4.status_code
print("Resposta_4:",status_4)

print("             |")
print("             v")

resposta = response_4.text
print(resposta)
