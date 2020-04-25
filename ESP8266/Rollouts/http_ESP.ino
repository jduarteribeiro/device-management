#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecureBearSSL.h>
#include <Arduino_JSON.h>

const char* ssid = "Zenfone";
const char* password = "12345678";

//Your Domain name with URL path or IP address with path
//const char* serverName = "https://device.eu1.bosch-iot-rollouts.com/D60F7F26-9AFC-4FFB-8FA5-00A6FC6D774A/controller/v1/esp8266_01";
String serverName = "https://device.eu1.bosch-iot-rollouts.com/D60F7F26-9AFC-4FFB-8FA5-00A6FC6D774A/controller/v1/esp8266_01";
const uint8_t fingerprint[20] = {0xbe, 0xa7, 0x9a, 0x85, 0x9d, 0xc9, 0x1a, 0xcf, 0x91, 0x5f, 0xcf, 0xe5, 0x13, 0x27, 0xf5, 0x3f, 0x0b, 0xb1, 0x63, 0x51};

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Set timer to 5 seconds (5000)
unsigned long timerDelay = 5000;

String pollUpdates;
String getInformation;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Definições para ESP8266
  // LED indicador de progresso
  ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);

  // Callback - Início
  ESPhttpUpdate.onStart([] {
    Serial.println("Atualização iniciada (callback)");
  });

  // Callback - Fim
  ESPhttpUpdate.onEnd([] {
    Serial.println("Atualização concluída (callback)");
  });

  // Callback - Erro
  ESPhttpUpdate.onError([](int erro) {
    Serial.println("Erro na atualização (callback), código: " + String(erro));
  });

  // Callback - Progresso
  ESPhttpUpdate.onProgress([](size_t progresso, size_t total) {
    Serial.print(progresso * 100 / total);
    Serial.print(" ");
  });

  // Não reiniciar automaticamente
  ESPhttpUpdate.rebootOnUpdate(false);

}

void loop() {
  //Send an HTTP POST request every 10 minutes
  if ((millis() - lastTime) > timerDelay) {
    //Check WiFi connection status
    if (WiFi.status() == WL_CONNECTED) {
      atualizar_ESP();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
}

String httpGETRequest(String serverName) {
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

  client->setFingerprint(fingerprint);

  HTTPClient http;

  // Your IP address with path or Domain name with URL path
  http.begin(*client, serverName);
  http.addHeader("Accept", "application/hal+json");
  http.addHeader("Authorization", "TargetToken 16873cd42738d484f3436de00906f5e2");

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();

  return payload;
}

void httpPOSTRequest(String serverName, String id) {
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

  client->setFingerprint(fingerprint);

  HTTPClient http;

  // Your IP address with path or Domain name with URL path
  http.begin(*client, serverName);
  http.addHeader("Content-Type", "application/json;charset=UTF-8");
  http.addHeader("Accept", "application/hal+json");
  http.addHeader("Authorization", "TargetToken 16873cd42738d484f3436de00906f5e2");

  // Send HTTP POST request
  String data = ("{\"id\":\"" + id + "\",\"status\":{\"result\":{\"finished\":\"success\"},\"execution\":\"closed\",\"details\":[ ]}}");
  int httpResponseCode = http.POST(data);

  if (httpResponseCode == 200) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.println("Feedback enviado!");
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }

  // Free resources
  http.end();
}

void atualizar_ESP() {
  // Cria instância de Cliente seguro
  WiFiClientSecure client;
  client.setInsecure();

  pollUpdates = httpGETRequest(serverName);
  JSONVar updates = JSON.parse(pollUpdates);
  Serial.println(updates);

  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(updates) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }

  // myObject.keys() can be used to get an array of all the keys in the object
  JSONVar keys_1 = updates.keys();
  JSONVar links = updates[keys_1[1]];
  JSONVar keys_2 = links.keys();
  JSONVar deploymentBase = links[keys_2[0]];
  JSONVar keys_3 = deploymentBase.keys();
  JSONVar href = deploymentBase[keys_3[0]];
  Serial.print(keys_3[0]);
  Serial.print(" = ");
  Serial.println(href);
  String url_information = JSON.stringify(href);
  url_information.trim();
  url_information = url_information.substring(1, url_information.length() - 1);
  Serial.println(url_information);

  getInformation = httpGETRequest(url_information);
  JSONVar information = JSON.parse(getInformation);
  Serial.println(information);

  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(information) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }

  // myObject.keys() can be used to get an array of all the keys in the object
  JSONVar chaves = information.keys();
  JSONVar id = information[chaves[0]];
  String id_str = JSON.stringify(id);
  id_str.trim();
  id_str = id_str.substring(1, id_str.length() - 1);
  Serial.print("Id = "); 
  Serial.println(id_str);
  JSONVar deployment = information[chaves[1]];
  JSONVar chaves2 = deployment.keys();
  JSONVar chunks = deployment[chaves2[2]];
  JSONVar chaves3 = chunks[0].keys();
  JSONVar artifacts = chunks[0][chaves3[3]];
  JSONVar chaves4 = artifacts[0].keys();
  JSONVar links_2 = artifacts[0][chaves4[3]];
  JSONVar chaves5 = links_2.keys();
  JSONVar download = links_2[chaves5[0]];
  JSONVar chaves6 = download.keys();
  JSONVar href_2 = download[chaves6[0]];
  Serial.print(chaves6[0]);
  Serial.print(" = ");
  Serial.println(href_2);

  //Converter JSONVar em String
  String url_download = JSON.stringify(href_2);
  url_download.trim();
  url_download = url_download.substring(1, url_download.length() - 1);
  Serial.println(url_download);

  String feedback_url = "https://device.eu1.bosch-iot-rollouts.com/D60F7F26-9AFC-4FFB-8FA5-00A6FC6D774A/controller/v1/esp8266_01/deploymentBase/" + id_str + "/feedback";
  Serial.println(feedback_url);

  //String httpResponseCode = ("{\"api_key\":\"tPmAT5Ab3j7F9\",\"sensor\":\"BME280\",\"value1\":\"24.25\",\"value2\":\"49.54\",\"value3\":\"1005.14\"}");
  String httpResponseCode = ("{\"id\":\"186718\",\"status\":{\"result\":{\"finished\":\"success\"},\"execution\":\"closed\",\"details\":\"[ ]\"}}");
  Serial.println(httpResponseCode);
  

  // Atualização Sketch ---------------------------------
  Serial.println("*** Atualização do sketch ***");
  for (byte b = 5; b > 0; b--) {
    Serial.print(b);
    Serial.println("... ");
    delay(1000);
  }

  // Efetua atualização do Sketch
  t_httpUpdate_return resultado = ESPhttpUpdate.update(client, url_download);

  // Verifica resultado
  switch (resultado) {
    case HTTP_UPDATE_FAILED: {
        String s = ESPhttpUpdate.getLastErrorString();
        Serial.println("\nFalha: " + s);
      } break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("\nNenhuma atualização disponível");
      break;
    case HTTP_UPDATE_OK: {
        Serial.println("\nSucesso, reiniciando...");
        httpPOSTRequest(feedback_url, id_str);
        ESP.restart();
      } break;
  }
}
