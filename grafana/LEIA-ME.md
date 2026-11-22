# Dashboards do Grafana

Arquivos exportados:

- `dashboard1_monitoramento.json` – luminosidade x limiares, estado da luminária, potência, acionamentos e nuvens.
- `dashboard2_eficiencia_energetica.json` – energia por cenário, economia (% e Wh), energia acumulada e tempo ligado.

## Como importar
1. No Grafana, crie o data source **InfluxDB** com linguagem **Flux** (URL do cluster, organização, bucket `iluminacao` e um token de **leitura**).
2. **Dashboards > New > Import** > envie o JSON.
3. Em *InfluxDB (Flux)*, escolha o data source criado no passo 1 e clique em **Import**.

Os JSONs não contêm tokens. As consultas de cada painel também estão em `../influxdb/queries.flux`.
