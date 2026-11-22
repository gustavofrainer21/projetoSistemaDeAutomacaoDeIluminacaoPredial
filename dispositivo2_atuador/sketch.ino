/*
  Dispositivo 2 - ATUADOR (ESP32 simulado no Wokwi)
  Projeto: Sistema de Automação de Iluminação por Luminosidade Ambiente
           para Eficiência Energética Predial (ODS 7, meta 7.3)

  Atuador:
    - Módulo relé (wokwi-relay-module), IN -> GPIO26
    - LED (luminária) alimentado pelo contato NO do relé

  MQTT:
    assina   <PREFIXO>/comando   payload: "LIGAR" ou "DESLIGAR"
    publica  <PREFIXO>/estado    payload: "LIGADO" ou "DESLIGADO" (retido)

*/

#include <WiFi.h>
#include <PubSubClient.h>

// ---------- CONFIGURAÇÕES ----------
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASS = "";
const int   WIFI_CHANNEL = 6;

const char* MQTT_HOST = "broker.hivemq.com";
const int   MQTT_PORT = 1883;

#define TOPIC_PREFIX "iot/ilumq7x/ods7"
const char* TOPIC_CMD = TOPIC_PREFIX "/comando";
const char* TOPIC_EST = TOPIC_PREFIX "/estado";

const int PIN_RELE = 26;
const bool RELE_ATIVO_ALTO = true;   // módulo relé do Wokwi: nível alto energiza

const unsigned long HEARTBEAT_MS = 5000;

WiFiClient espClient;
PubSubClient mqtt(espClient);
bool ligado = false;
unsigned long ultimoHb = 0;

void aplicaRele(bool on) {
  digitalWrite(PIN_RELE, (on == RELE_ATIVO_ALTO) ? HIGH : LOW);
}

void publicaEstado() {
  mqtt.publish(TOPIC_EST, ligado ? "LIGADO" : "DESLIGADO", true);  // retido
  Serial.println(ligado ? "[ESTADO] LIGADO" : "[ESTADO] DESLIGADO");
}

void aoReceber(char* topic, byte* payload, unsigned int len) {
  String msg;
  for (unsigned int i = 0; i < len; i++) msg += (char)payload[i];
  msg.trim();
  msg.toUpperCase();
  Serial.println("[CMD] " + msg);

  bool novo = ligado;
  if (msg == "LIGAR") novo = true;
  else if (msg == "DESLIGAR") novo = false;
  else return;                      

  if (novo != ligado) {
    ligado = novo;
    aplicaRele(ligado);
  }
  publicaEstado();  // confirma sempre o estado real
}

void conectaWifi() {
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(WIFI_SSID, WIFI_PASS, WIFI_CHANNEL);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(" OK");
  Serial.print("IP do dispositivo: ");
  Serial.println(WiFi.localIP());
}

void conectaMqtt() {
  while (!mqtt.connected()) {
    String cid = "ods7-dev2-" + String((uint32_t)esp_random(), HEX);
    Serial.print("Conectando ao broker MQTT... ");
    if (mqtt.connect(cid.c_str())) {
      Serial.println("OK (" + cid + ")");
      mqtt.subscribe(TOPIC_CMD);
      publicaEstado();
    } else {
      Serial.print("falhou rc=");
      Serial.println(mqtt.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_RELE, OUTPUT);
  aplicaRele(false);                 // inicia desligado
  conectaWifi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setKeepAlive(60);
  mqtt.setCallback(aoReceber);
  conectaMqtt();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) conectaWifi();
  if (!mqtt.connected()) conectaMqtt();
  mqtt.loop();

  if (millis() - ultimoHb >= HEARTBEAT_MS) {
    ultimoHb = millis();
    publicaEstado();
  }
}
