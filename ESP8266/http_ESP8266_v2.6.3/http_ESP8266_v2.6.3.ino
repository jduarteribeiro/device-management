#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <Arduino_JSON.h>

#define USE_SECURE_CONNECTION 1

#if (USE_SECURE_CONNECTION == 1)
#include <WiFiClientSecure.h>
#endif

#if (USE_SECURE_CONNECTION == 1)
WiFiClientSecure wifiClient;
#else
WiFiClient wifiClient;
#endif

const char* ssid = "DESKTOP-C6SOVQS 6152";
const char* password = "12345678";

//Your Domain name with URL path or IP address with path
const char* host = "device.eu1.bosch-iot-rollouts.com";
const char* fingerprint = "BE A7 9A 85 9D C9 1A CF 91 5F CF E5 13 27 F5 3F 0B B1 63 51";
String serverName = "https://device.eu1.bosch-iot-rollouts.com/D60F7F26-9AFC-4FFB-8FA5-00A6FC6D774A/controller/v1/test240520";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Set timer to 1 minute
unsigned long timerDelay = 60000;

String links_1;
int httpResponseCode_1;
int httpResponseCode_2;
String id_str;

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
      
#if (USE_SECURE_CONNECTION == 1)
      wifiClient.setInsecure();
      if (!wifiClient.connect(host, 443)) {
        Serial.println("Secure connection failed, restart Device");
        ESP.restart();
      } else {
        Serial.println("Successfully established secure connection to Rollouts");
      }

      if (!wifiClient.verify(fingerprint, host)) {
        Serial.println("Verification failed, restart Device");
        ESP.restart();
      } else {
        Serial.println("Successfully verified server certificate\n");
      }
#endif

      String url_information = Poll_for_Updates(serverName);

      if (httpResponseCode_1 == 200 && links_1 == "deploymentBase") {
        String url_download = Get_Information(url_information);

        if (httpResponseCode_2 == 200) {
          Download_and_Update_Sketch(url_download);
        }
        else {
          return;
        }
      }
      else {
        Serial.println("No updates available!\n");
        lastTime = millis();
      }
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
  else {
    Serial.println("Running...");
    delay(10000);
  }
}


String Poll_for_Updates(String serverName) {

  HTTPClient http;

  // Your IP address with path or Domain name with URL path
  http.begin(wifiClient, serverName);
  http.addHeader("Accept", "application/hal+json");
  http.addHeader("Authorization", "TargetToken a61bcf05710e953bfd64daff72414ffc");

  // Send HTTP POST request
  httpResponseCode_1 = http.GET();

  String pollUpdates = "{}";
  String url_information = "{}";

  if (httpResponseCode_1 == 200) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode_1);
    pollUpdates = http.getString();

    JSONVar updates = JSON.parse(pollUpdates);
    Serial.println(updates);

    // myObject.keys() can be used to get an array of all the keys in the object
    JSONVar keys_1 = updates.keys();
    JSONVar links = updates[keys_1[1]];
    JSONVar keys_2 = links.keys();
    JSONVar deploymentBase = links[keys_2[0]];
    links_1 = keys_2[0];
    JSONVar keys_3 = deploymentBase.keys();
    JSONVar href = deploymentBase[keys_3[0]];
    Serial.print(keys_3[0]);
    Serial.print(" = ");
    Serial.println(href);
    url_information = JSON.stringify(href);
    url_information.trim();
    url_information = url_information.substring(1, url_information.length() - 1);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode_1);
  }

  // Free resources
  http.end();

  return url_information;
}

String Get_Information(String url_information) {

  HTTPClient http;

  // Your IP address with path or Domain name with URL path
  http.begin(wifiClient, url_information);
  http.addHeader("Accept", "application/hal+json");
  http.addHeader("Authorization", "TargetToken a61bcf05710e953bfd64daff72414ffc");

  // Send HTTP POST request
  httpResponseCode_2 = http.GET();

  String getInformation = "{}";
  String url_download = "{}";

  if (httpResponseCode_2 == 200) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode_2);
    getInformation = http.getString();

    JSONVar information = JSON.parse(getInformation);
    Serial.println(information);

    // myObject.keys() can be used to get an array of all the keys in the object
    JSONVar chaves = information.keys();
    JSONVar id = information[chaves[0]];
    id_str = JSON.stringify(id);
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
    url_download = JSON.stringify(href_2);
    url_download.trim();
    url_download = url_download.substring(1, url_download.length() - 1);
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode_2);
  }

  // Free resources
  http.end();

  return url_download;
}

void httpPOSTRequest(String feedback_url, String id) {

  HTTPClient http;

  // Your IP address with path or Domain name with URL path
  http.begin(wifiClient, feedback_url);
  http.addHeader("Content-Type", "application/json;charset=UTF-8");
  http.addHeader("Accept", "application/hal+json");
  http.addHeader("Authorization", "TargetToken a61bcf05710e953bfd64daff72414ffc");

  // Send HTTP POST request
  String data = ("{\"id\":\"" + id + "\",\"status\":{\"result\":{\"finished\":\"success\"},\"execution\":\"closed\",\"details\":[ ]}}");
  int httpResponseCode_3 = http.POST(data);

  if (httpResponseCode_3 == 200) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode_3);
    Serial.println("Feedback enviado!");
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode_3);
  }

  // Free resources
  http.end();
}

void Download_and_Update_Sketch(String url_download) {
  // Cria instância de Cliente seguro
  WiFiClientSecure client;
  client.setInsecure();

  String feedback_url = "https://device.eu1.bosch-iot-rollouts.com/D60F7F26-9AFC-4FFB-8FA5-00A6FC6D774A/controller/v1/test240520/deploymentBase/" + id_str + "/feedback";

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
