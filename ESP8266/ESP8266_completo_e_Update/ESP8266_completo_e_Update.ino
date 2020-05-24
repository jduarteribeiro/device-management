#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <MMA_7455.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

//BIBLIOTECAS PARA UPDATES + ESP8266WiFi.h
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

Adafruit_BME280 bme;
MMA_7455 accel = MMA_7455(i2c_protocol);

const char* ssid = "DESKTOP-C6SOVQS 6152";
const char* password = "12345678";
const char* mqtt_server = "192.168.137.40";

WiFiClient espClient;
PubSubClient client(espClient);

long lastMsg = 0;
char msg[50];
int value = 0;

int16_t x10, y10, z10;
float cTemp;
float humid_rel;
float luz;

//VARIÁVEIS PARA UPDATES----------------------------------------------------------------------------------------------------
//Your Domain name with URL path or IP address with path
const char* host = "device.eu1.bosch-iot-rollouts.com";
const char* fingerprint = "BE A7 9A 85 9D C9 1A CF 91 5F CF E5 13 27 F5 3F 0B B1 63 51";
String serverName = "https://device.eu1.bosch-iot-rollouts.com/D60F7F26-9AFC-4FFB-8FA5-00A6FC6D774A/controller/v1/esp8266";

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastTime = 0;
// Set timer to 10 minutes
unsigned long timerDelay = 600000;

String links_1;
int httpResponseCode_1;
int httpResponseCode_2;
String id_str;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  //UPDATES--------------------------------------------------------------------
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

void callback(String topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);

  if (topic == "resend_temp") {
    Serial.println("Reenviar temperatura\n");
    temp_humid_calculate();
    char Temperatura[6];
    snprintf(Temperatura, 6, "%f", cTemp);
    client.publish("temp", Temperatura);
  }
  else if (topic == "resend_humid") {
    Serial.println("Reenviar humidade\n");
    temp_humid_calculate();
    char Humidade[6];
    snprintf(Humidade, 6, "%f", humid_rel);
    client.publish("humid", Humidade);
  }
  else if (topic == "resend_lumin") {
    Serial.println("Reenviar luminosidade\n");
    luz = analogRead(A0);
    luz = (luz / 1024.0) * 100.0;
    char Luminosidade[6];
    snprintf(Luminosidade, 6, "%f", luz);
    client.publish("lumin", Luminosidade);
  }
  else if (topic == "resend_vibrations") {
    Serial.println("Reenviar vibrações\n");
    vibrations_calculate();
    char AceleracaoX[4];
    char AceleracaoY[4];
    char AceleracaoZ[4];
    snprintf(AceleracaoX, 4, "%d", x10);
    snprintf(AceleracaoY, 4, "%d", y10);
    snprintf(AceleracaoZ, 4, "%d", z10);
    client.publish("xAcc", AceleracaoX);
    client.publish("yAcc", AceleracaoY);
    client.publish("zAcc", AceleracaoZ);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("AtcTempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // AtcTempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      Serial.println();
      // Once connected, publish an announcement...
      /*
      client.publish("temp", "0");
      client.publish("humid", "0");
      client.publish("lumin", "0");
      client.publish("xAcc", "0");
      client.publish("yAcc", "0");
      client.publish("zAcc", "0");
      */
      // ... and resubscribe
      client.subscribe("resend_temp");
      client.subscribe("resend_humid");
      client.subscribe("resend_lumin");
      client.subscribe("resend_vibrations");

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void MMA7455_setup() {
  accel.begin();
  /* Set accelerometer sensibility */
  accel.setSensitivity(2);
  /* Verify sensibility - optional */
  if (accel.getSensitivity() != 2)   Serial.println("Sensitivity failure");
  /* Set accelerometer mode */
  accel.setMode(measure);
  /* Verify accelerometer mode - optional */
  if (accel.getMode() != measure)    Serial.println("Set mode failure");
  /* Set axis offsets */
  /*Estes valores foram obtidos através do exemplo de calibração*/
  accel.setAxisOffset(22, 50, 0);
}

void vibrations_calculate() {
  /* Get 10-bit axis raw values */
  x10 = accel.readAxis10('x');
  y10 = accel.readAxis10('y');
  z10 = accel.readAxis10('z');
}

void BME280_setup() {
  unsigned status;
  status = bme.begin(0x76);
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    while (1);
  }
}

void temp_humid_calculate() {
  cTemp = bme.readTemperature();
  humid_rel = bme.readHumidity();
}

//FUNÇÕES UPDATES-----------------------------------------------------------------------------------
String Poll_for_Updates(String serverName) {

  HTTPClient http;

  // Your IP address with path or Domain name with URL path
  http.begin(wifiClient, serverName);
  http.addHeader("Accept", "application/hal+json");
  http.addHeader("Authorization", "TargetToken cddeb4c8f3281757a3d5f67e8fe65b36");

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
  http.addHeader("Authorization", "TargetToken cddeb4c8f3281757a3d5f67e8fe65b36");

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
  http.addHeader("Authorization", "TargetToken cddeb4c8f3281757a3d5f67e8fe65b36");

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

  String feedback_url = "https://device.eu1.bosch-iot-rollouts.com/D60F7F26-9AFC-4FFB-8FA5-00A6FC6D774A/controller/v1/esp8266/deploymentBase/" + id_str + "/feedback";

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



void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  MMA7455_setup();
  BME280_setup();
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  char Temperatura[6];
  char Humidade[6];
  char Luminosidade[6];
  char AceleracaoX[4];
  char AceleracaoY[4];
  char AceleracaoZ[4];

  long now = millis();
  if (now - lastMsg > 300000) {
    lastMsg = now;

    temp_humid_calculate();
    vibrations_calculate();

    luz = analogRead(A0);
    luz = (luz / 1024.0) * 100.0;

    ++value;
    snprintf (msg, 50, "%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    Serial.print("Temperatura: ");
    Serial.print(cTemp);
    Serial.println("ºC");
    Serial.print("Humidade: ");
    Serial.print(humid_rel);
    Serial.println("%");
    Serial.print("Luminosidade: ");
    Serial.print(luz);
    Serial.println("%");
    Serial.print("Aceleração X: ");
    Serial.println(x10);
    Serial.print("Aceleração Y: ");
    Serial.println(y10);
    Serial.print("Aceleração Z: ");
    Serial.println(z10);
    Serial.println();
    snprintf(Temperatura, 6, "%f", cTemp);
    snprintf(Humidade, 6, "%f", humid_rel);
    snprintf(Luminosidade, 6, "%f", luz);
    snprintf(AceleracaoX, 4, "%d", x10);
    snprintf(AceleracaoY, 4, "%d", y10);
    snprintf(AceleracaoZ, 4, "%d", z10);
    client.publish("temp", Temperatura);
    client.publish("humid", Humidade);
    client.publish("lumin", Luminosidade);
    client.publish("xAcc", AceleracaoX);
    client.publish("yAcc", AceleracaoY);
    client.publish("zAcc", AceleracaoZ);
  }

  //UPDATES-----------------------------------------------------------------------------
  //Send an HTTP POST request every 1 minute
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
}
