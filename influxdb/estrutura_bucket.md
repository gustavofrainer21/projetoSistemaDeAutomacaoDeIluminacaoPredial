# Estrutura do bucket

| Item | Valor |
|---|---|
| Bucket | `iluminacao` |
| Medição (`_measurement`) | `iluminacao_ods7` |
| Tag | `cenario` (`A` sem automação, `B` com automação) |

## Campos (`_field`)

| Campo | Tipo | Descrição |
|---|---|---|
| `luminosidade` | float | luminosidade medida (lux) |
| `potencia_medida_W` | float | potência lida no potenciômetro (W) |
| `potencia_efetiva_W` | float | potência considerada no cálculo de energia (W) |
| `estado_cmd` | int | último comando enviado (1 ligar, 0 desligar) |
| `estado_real` | int | estado confirmado pelo atuador (1 ligada, 0 desligada) |
| `limiar_on` | float | limiar de acionamento vigente (lux) |
| `limiar_off` | float | limiar de desligamento vigente (lux) |
| `nuvens_pct` | float | cobertura de nuvens da OpenWeather (%) |

## Tokens

- escrita no bucket `iluminacao`: usado pelo Node-RED;
- leitura no bucket `iluminacao`: usado pelo Grafana.

As consultas Flux estão em `queries.flux`.
