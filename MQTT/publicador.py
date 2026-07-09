import paho.mqtt.client as mqtt
import time
import math

BROKER = "192.168.97.128"
PUERTO = 1883
TOPICO_TEMPERATURA = "laboratorio/temperatura"
INTERVALO_SEGUNDOS = 2

cliente = mqtt.Client()
cliente.connect(BROKER, PUERTO)

fase_angulo = 0
while True:
    temperatura = 25 + 5 * math.sin(fase_angulo)
    mensaje = f"{temperatura:.2f}"
    cliente.publish(TOPICO_TEMPERATURA, mensaje)
    print(f"[Publicador] Temperatura enviada: {mensaje} °C a {TOPICO_TEMPERATURA}")
    fase_angulo += 0.3
    time.sleep(INTERVALO_SEGUNDOS)
