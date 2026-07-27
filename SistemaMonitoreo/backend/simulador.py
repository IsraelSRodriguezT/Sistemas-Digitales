import json
import time
import random
import paho.mqtt.client as mqtt

client = mqtt.Client()
client.connect("localhost", 1883, 60)

humo = {1: 1200, 2: 800}
temp = {1: 28.0, 2: 30.0}

while True:
    for nodo in [1, 2]:
        if random.random() < 0.05:
            humo[nodo] += random.randint(1500, 2500)
        else:
            humo[nodo] += random.randint(-200, 200)
        humo[nodo] = max(0, min(4095, humo[nodo]))

        if random.random() < 0.05:
            temp[nodo] += random.uniform(15, 25)
        else:
            temp[nodo] += random.uniform(-2, 2)
        temp[nodo] = max(15, min(80, round(temp[nodo], 1)))

        datos = {"id": nodo, "temp": temp[nodo], "humo": humo[nodo], "ts": int(time.time())}
        client.publish("incendios/datos", json.dumps(datos))

        if humo[nodo] > 2800 and temp[nodo] > 45:
            alerta = {"id": nodo, "tipo": "FIRE", "temp": temp[nodo], "humo": humo[nodo]}
            client.publish("incendios/alerta", json.dumps(alerta))

    time.sleep(3)
