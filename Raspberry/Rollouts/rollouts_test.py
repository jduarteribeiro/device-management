import requests
import json
import time
import os

headers = {
    'Accept': 'application/hal+json',
    'Authorization': 'TargetToken 9c836a4af349b0c9076fc85be268da92',
}

url_2 = 0
device_id = 0
file_name = 0
url_3 = 0
file_content = 0
url_4 = 0
status_4 = 0

def poll_updates():
    global url_2
    print("URL_1: https://device.eu1.bosch-iot-rollouts.com/D60F7F26-9AFC-4FFB-8FA5-00A6FC6D774A/controller/v1/rasp01")
    response_1 = requests.get('https://device.eu1.bosch-iot-rollouts.com/D60F7F26-9AFC-4FFB-8FA5-00A6FC6D774A/controller/v1/rasp01', headers=headers)
    status_1 = response_1.status_code
    print("Resposta_1:",status_1)
    print("             |")
    print("             v")
    url_2 = json.loads(json.dumps(json.loads(response_1.text)))["_links"]["deploymentBase"]["href"]
    print("URL_2:",url_2)

def get_information():
    global device_id
    global file_name
    global url_3
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
    file_name = json.loads(json.dumps(url))[0]["filename"]
    url_3 = json.loads(json.dumps(url))[0]["_links"]["download"]["href"]
    print("File name:", file_name)
    print("URL_3:",url_3)

def download_artifact():
    global file_content
    response_3 = requests.get(url_3, headers=headers)
    status_3 = response_3.status_code
    print("Resposta_3:",status_3)
    print("             |")
    print("             v")
    file_content = response_3.text
    print(file_content)
    
def edit_file():
    global url_4
    
    if os.path.exists(file_name):
        os.remove(file_name)
        file = open(file_name, "w")
        file.write(file_content)
        file.close()
    else:        
        file = open(file_name, "w")
        file.write(file_content)
        file.close()
        
    print()
    url_4 = 'https://device.eu1.bosch-iot-rollouts.com/D60F7F26-9AFC-4FFB-8FA5-00A6FC6D774A/controller/v1/rasp01/deploymentBase/' + device_id +'/feedback'
    print("URL_4:",url_4)

def feedback():
    global status_4
    
    headers_4 = {
        'Content-Type': 'application/json;charset=UTF-8',
        'Accept': 'application/hal+json',
        'Authorization': 'TargetToken 9c836a4af349b0c9076fc85be268da92',
    }
    data = '{\n  "id" :'+device_id+',\n  "status" : {\n    "result" : {\n      "finished" : "success"\n    },\n    "execution" : "closed",\n    "details" : [ ]\n  }\n}'
    response_4 = requests.post(url_4, headers=headers_4, data=data)
    status_4 = response_4.status_code
    print("Resposta_4:",status_4)
    
    if (status_4 == 200):
        print()
        print("Updated device!")
        print()
        time.sleep(15)

def run_file():
    os.system('python3 ' + file_name) 

def main():
    while(1):
        try:
            poll_updates()
        except KeyError:        
            print("No update available!")
            print()
            time.sleep(15)
            main()       
        
        get_information()
        download_artifact()
        edit_file()
        feedback()
        run_file()
        print("ola")
        print()
        pass

if __name__ == "__main__":
    main()