"""Gera docs/diagrama_topologia_logica.png com FQDN e IP de cada elemento.
Valores levantados em 22/11/2026 (DNS publico para os servicos em nuvem; Configuracoes do Windows para o Node-RED).
    pip install matplotlib
    python gerar_topologia.py
"""
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.patches import FancyBboxPatch, FancyArrowPatch

ELEMENTOS = {
    "d1":   ("Dispositivo 1 (sensores)\nESP32 - Wokwi",   "FQDN: n/a (cliente)",                         "IP: 10.10.0.2 (rede do Wokwi)"),
    "d2":   ("Dispositivo 2 (atuador)\nESP32 - Wokwi",    "FQDN: n/a (cliente)",                         "IP: 10.10.0.2 (rede do Wokwi)"),
    "mqtt": ("Broker MQTT\nHiveMQ público :1883",         "FQDN: broker.hivemq.com",                     "IP: 3.124.112.32"),
    "nr":   ("Node-RED\n(regras de negócio) :1880",       "FQDN: ANDRE",                                 "IP: 192.168.15.200"),
    "owm":  ("OpenWeather API",                           "FQDN: api.openweathermap.org",                "IP: 169.197.141.196"),
    "db":   ("InfluxDB Cloud",                            "FQDN: us-east-1-1.aws.\ncloud2.influxdata.com", "IP: 34.196.233.7"),
    "gf":   ("Grafana Cloud",                             "FQDN: swiftstew1692\n.grafana.net",           "IP: 104.18.12.97"),
}


def box(ax, x, y, w, h, key, fc, ec):
    t, f, i = ELEMENTOS[key]
    ax.add_patch(FancyBboxPatch((x, y), w, h, boxstyle="round,pad=0.02,rounding_size=0.08", fc=fc, ec=ec, lw=1.4))
    ax.text(x + w / 2, y + h * 0.72, t, ha="center", va="center", fontsize=8.5, fontweight="bold")
    ax.text(x + w / 2, y + h * 0.38, f, ha="center", va="center", fontsize=6 if max(len(l) for l in f.split("\n")) > 22 and w < 2 else 7)
    ax.text(x + w / 2, y + h * 0.12, i, ha="center", va="center", fontsize=7)


def arrow(ax, p1, p2, txt="", both=False, off=(0, 0.15)):
    ax.add_patch(FancyArrowPatch(p1, p2, arrowstyle="<|-|>" if both else "-|>", mutation_scale=12, lw=1.3, color="#333"))
    if txt:
        ax.text((p1[0] + p2[0]) / 2 + off[0], (p1[1] + p2[1]) / 2 + off[1], txt, ha="center", va="center",
                fontsize=7.5, bbox=dict(fc="white", ec="none", pad=1))


fig, ax = plt.subplots(figsize=(13, 7))
ax.set_xlim(0, 13)
ax.set_ylim(0, 7)
ax.axis("off")
box(ax, 0.2, 4.6, 2.8, 1.6, "d1", "#FFF2CC", "#8A6D00")
box(ax, 0.2, 0.6, 2.8, 1.6, "d2", "#FFF2CC", "#8A6D00")
box(ax, 4.6, 2.6, 2.8, 1.6, "mqtt", "#DDEBF7", "#1F3A5F")
box(ax, 8.2, 2.6, 2.6, 1.6, "nr", "#E2F0D9", "#2E6B2E")
box(ax, 8.2, 5.0, 2.6, 1.4, "owm", "#FCE4D6", "#9C4A1A")
box(ax, 11.2, 2.6, 1.7, 1.6, "db", "#EADCF4", "#5B2C83")
box(ax, 11.2, 5.0, 1.7, 1.4, "gf", "#EADCF4", "#5B2C83")
arrow(ax, (3.0, 5.2), (4.6, 4.0), "publica luminosidade\ne potência", off=(-0.2, 0.5))
arrow(ax, (4.6, 2.8), (3.0, 1.6), "assina comando\npublica estado", both=True, off=(0.2, -0.6))
arrow(ax, (7.4, 3.4), (8.2, 3.4), "MQTT", both=True)
arrow(ax, (9.5, 4.2), (9.5, 5.0), "HTTP GET", both=True, off=(0.6, 0))
arrow(ax, (10.8, 3.4), (11.2, 3.4), "")
arrow(ax, (12.05, 4.2), (12.05, 5.0), "Flux", off=(0.4, 0))
ax.text(6.5, 6.75, "Topologia lógica do sistema", ha="center", fontsize=12, fontweight="bold")
fig.savefig("diagrama_topologia_logica.png", dpi=200, bbox_inches="tight", facecolor="white")
