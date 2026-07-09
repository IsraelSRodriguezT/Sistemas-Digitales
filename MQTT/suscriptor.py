import paho.mqtt.client as mqtt

BROKER = "192.168.97.128"
PUERTO = 1883
TOPICOS = [("laboratorio/temperatura", 0), ("laboratorio/led", 0)]

def al_recibir_mensaje(cliente, datos_usuario, mensaje):
    topico = mensaje.topic
    payload = mensaje.payload.decode()
    print(f"[Suscriptor] Mensaje recibido -> Tópico: {topico} | Payload: {payload}")

cliente = mqtt.Client()
cliente.on_message = al_recibir_mensaje
cliente.connect(BROKER, PUERTO)
cliente.subscribe(TOPICOS)
print(f"Suscriptor conectado a {BROKER}, esperando mensajes...")
cliente.loop_forever()
