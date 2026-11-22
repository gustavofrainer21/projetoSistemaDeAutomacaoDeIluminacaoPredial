/*
  Dispositivo 1 - SENSORES (ESP32 simulado no Wokwi)
  Projeto: Sistema de Automação de Iluminação por Luminosidade Ambiente
           para Eficiência Energética Predial (ODS 7, meta 7.3)

  Sensores:
    - LDR (módulo wokwi-photoresistor-sensor)  -> GPIO34 (AO)
    - Potenciômetro (simula o sensor de potência da luminária) -> GPIO35 (SIG)

  Publica via MQTT (broker público HiveMQ):
    <PREFIXO>/luminosidade  {"luminosidade": <lux>, "bruto": <0-4095>, "dispositivo": "dev1_sensor"}
    <PREFIXO>/potencia      {"potencia_W": <W>,     "bruto": <0-4095>, "dispositivo": "dev1_sensor"}

  IMPORTANTE: troque TOPIC_PREFIX por um prefixo exclusivo do grupo
  (o broker é público e qualquer pessoa pode ler/escrever nos tópicos).
  O MESMO prefixo deve ser usado no dispositivo 2 e no Node-RED.
*/

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <math.h>

// ---------- CONFIGURAÇÕES ----------
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";
const int   WIFI_CHANNEL = 6;

const char* MQTT_HOST = "broker.hivemq.com";
const int   MQTT_PORT = 1883;

// >>> TROQUE pelo prefixo do seu grupo <<<
#define TOPIC_PREFIX "iot/ilumq7x/ods7"
const char* TOPIC_LUX = TOPIC_PREFIX "/luminosidade";
const char* TOPIC_POT = TOPIC_PREFIX "/potencia";

const int PIN_LDR = 34;
const int PIN_POT = 35;

const float P_MAX_W = 60.0;            // potência máxima representada pelo potenciômetro (W)
const unsigned long PERIODO_MS = 5000; // intervalo entre publicações

// Constantes do LDR (modelo do Wokwi, ver docs.wokwi.com - Photoresistor)
const float GAMMA = 0.7;
const float RL10 = 50;                 // kOhm a 10 lux

WiFiClient espClient;
PubSubClient mqtt(espClient);
unsigned long ultimo = 0;

void conectaWifi() {
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASS, WIFI_CHANNEL);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(" OK");
  Serial.print("IP do dispositivo: ");
  Serial.println(WiFi.localIP());   // anote este IP para a tabela de topologia do artigo
}

void conectaMqtt() {
  while (!mqtt.connected()) {
    // client ID único para evitar desconexão por conflito no broker público
    String cid = "ods7-dev1-" + String((uint32_t)esp_random(), HEX);
    Serial.print("Conectando ao broker MQTT... ");
    if (mqtt.connect(cid.c_str())) {
      Serial.println("OK (" + cid + ")");
    } else {
      Serial.print("falhou rc=");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

// média de várias leituras para reduzir ruído
int leituraMedia(int pino, int n = 8) {
  long soma = 0;
  for (int i = 0; i < n; i++) {
    soma += analogRead(pino);
    delay(2);
  }
  return soma / n;
}

float bruto2lux(int bruto) {
  // evita divisão por zero nos extremos do ADC
  if (bruto < 1) bruto = 1;
  if (bruto > 4094) bruto = 4094;
  float tensao = bruto / 4095.0 * 3.3;
  float resistencia = 2000.0 * tensao / (1.0 - tensao / 3.3);
  float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistencia, (1.0 / GAMMA));
  return lux;
}

void publicaLuminosidade() {
  int bruto = leituraMedia(PIN_LDR);
  float lux = bruto2lux(bruto);
  StaticJsonDocument<128> doc;
  doc["luminosidade"] = round(lux * 10.0) / 10.0;
  doc["bruto"] = bruto;
  doc["dispositivo"] = "dev1_sensor";
  char buf[128];
  serializeJson(doc, buf);
  mqtt.publish(TOPIC_LUX, buf);
  Serial.printf("[LDR] bruto=%d  lux=%.1f\n", bruto, lux);
}

void publicaPotencia() {
  int bruto = leituraMedia(PIN_POT);
  float watts = bruto / 4095.0 * P_MAX_W;   // adaptação: potenciômetro -> watts
  StaticJsonDocument<128> doc;
  doc["potencia_W"] = round(watts * 10.0) / 10.0;
  doc["bruto"] = bruto;
  doc["dispositivo"] = "dev1_sensor";
  char buf[128];
  serializeJson(doc, buf);
  mqtt.publish(TOPIC_POT, buf);
  Serial.printf("[POT] bruto=%d  potencia=%.1f W\n", bruto, watts);
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  conectaWifi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setKeepAlive(60);
  conectaMqtt();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) conectaWifi();
  if (!mqtt.connected()) conectaMqtt();
  mqtt.loop();

  if (millis() - ultimo >= PERIODO_MS) {
    ultimo = millis();
    publicaLuminosidade();
    publicaPotencia();
  }
}
