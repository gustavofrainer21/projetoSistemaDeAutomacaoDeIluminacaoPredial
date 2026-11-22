# Nós utilizados no Node-RED

## Paleta adicional (instalar)

| Pacote | Uso |
|---|---|
| `node-red-contrib-influxdb` | nós `influxdb` (configuração) e `influxdb out` (gravação no InfluxDB Cloud) |

Instalação: menu > Manage palette > Install, ou `npm install node-red-contrib-influxdb` na pasta do Node-RED.

## Nós nativos do Node-RED

`inject`, `mqtt in`, `mqtt out`, `mqtt-broker` (configuração), `function`, `http request`, `debug`, `comment`.

## Variável de ambiente

| Variável | Descrição |
|---|---|
| `OPENWEATHER_KEY` | chave da API OpenWeather (modelo em `.env.example`) |

A URL, a organização e o token do InfluxDB são preenchidos no editor do Node-RED após a importação do `flows.json`.
