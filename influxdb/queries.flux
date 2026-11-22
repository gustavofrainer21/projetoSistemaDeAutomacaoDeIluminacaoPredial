// =====================================================================
// Consultas Flux usadas nos painéis do Grafana (data source InfluxDB, Flux)
// Bucket: iluminacao | Medição: iluminacao_ods7
// Campos: luminosidade, potencia_medida_W, potencia_efetiva_W, estado_cmd,
//         estado_real, limiar_on, limiar_off, nuvens_pct | Tag: cenario (A|B)
// =====================================================================

// ---------------- DASHBOARD 1: Monitoramento da iluminação ----------------

// P1.1 Luminosidade x tempo com limiar (Time series)
from(bucket: "iluminacao")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "iluminacao_ods7")
  |> filter(fn: (r) => r._field == "luminosidade" or r._field == "limiar_on" or r._field == "limiar_off")

// P1.2 Estado atual da luminária (Stat; value mapping: 1 = LIGADA, 0 = DESLIGADA)
from(bucket: "iluminacao")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "iluminacao_ods7" and r._field == "estado_real")
  |> last()

// P1.3 Potência medida e efetiva (Time series, unidade W)
from(bucket: "iluminacao")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "iluminacao_ods7")
  |> filter(fn: (r) => r._field == "potencia_medida_W" or r._field == "potencia_efetiva_W")

// P1.4 Número de acionamentos no período (Stat)
from(bucket: "iluminacao")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "iluminacao_ods7" and r._field == "estado_real" and r.cenario == "B")
  |> difference()
  |> filter(fn: (r) => r._value == 1.0)
  |> group()
  |> count()

// P1.5 Cobertura de nuvens atual (Gauge, 0-100 %)
from(bucket: "iluminacao")
  |> range(start: -1h)
  |> filter(fn: (r) => r._measurement == "iluminacao_ods7" and r._field == "nuvens_pct")
  |> last()

// ---------------- DASHBOARD 2: Eficiência energética (ODS 7, meta 7.3) ----------------

// P2.1 Energia consumida por cenário, em kWh (Bar chart / Stat)
// Compare períodos de MESMA duração para A e B (ver README).
from(bucket: "iluminacao")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "iluminacao_ods7" and r._field == "potencia_efetiva_W")
  |> group(columns: ["cenario"])
  |> integral(unit: 1h)
  |> map(fn: (r) => ({r with _value: r._value / 1000.0}))
  |> keep(columns: ["cenario", "_value"])

// P2.2 Economia percentual (Stat, unidade %): compara a potência média efetiva de A e B
from(bucket: "iluminacao")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "iluminacao_ods7" and r._field == "potencia_efetiva_W")
  |> group(columns: ["cenario"])
  |> mean()
  |> pivot(rowKey: ["_start"], columnKey: ["cenario"], valueColumn: "_value")
  |> map(fn: (r) => ({_value: (r.A - r.B) / r.A * 100.0}))

// P2.3 Economia absoluta em kWh (Stat): (E_A - E_B) para períodos de mesma duração
from(bucket: "iluminacao")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "iluminacao_ods7" and r._field == "potencia_efetiva_W")
  |> group(columns: ["cenario"])
  |> integral(unit: 1h)
  |> pivot(rowKey: ["_start"], columnKey: ["cenario"], valueColumn: "_value")
  |> map(fn: (r) => ({_value: (r.A - r.B) / 1000.0}))

// P2.4 Energia acumulada por cenário, em kWh (Time series)
from(bucket: "iluminacao")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "iluminacao_ods7" and r._field == "potencia_efetiva_W")
  |> group(columns: ["cenario"])
  |> aggregateWindow(every: 1m, fn: mean, createEmpty: false)
  |> map(fn: (r) => ({r with _value: r._value / 60.0 / 1000.0}))   // W médio x 1/60 h / 1000
  |> cumulativeSum()

// P2.5 Horas com a luminária ligada (cenário B) x horas totais do período (Bar gauge)
// Obs.: amostragem de 5 s.
from(bucket: "iluminacao")
  |> range(start: v.timeRangeStart, stop: v.timeRangeStop)
  |> filter(fn: (r) => r._measurement == "iluminacao_ods7" and r._field == "estado_real" and r.cenario == "B")
  |> group()
  |> integral(unit: 1h)
