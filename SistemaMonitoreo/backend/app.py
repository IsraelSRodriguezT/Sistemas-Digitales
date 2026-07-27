import sqlite3
import json
import os
from datetime import datetime
from threading import Thread

import paho.mqtt.client as mqtt
from flask import Flask, jsonify, render_template, request

app = Flask(__name__)
DB_PATH = os.path.join(os.path.dirname(__file__), 'monitoreo.db')


def init_db():
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute('''
        CREATE TABLE IF NOT EXISTS lecturas (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            id_nodo INTEGER,
            temperatura REAL,
            nivel_humo INTEGER,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    ''')
    c.execute('''
        CREATE TABLE IF NOT EXISTS alertas (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            id_nodo INTEGER,
            tipo TEXT,
            mensaje TEXT,
            timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    ''')
    conn.commit()
    conn.close()


def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        conn = sqlite3.connect(DB_PATH)
        c = conn.cursor()
        now = datetime.now().strftime('%Y-%m-%d %H:%M:%S')

        if msg.topic == 'incendios/datos':
            c.execute('''
                INSERT INTO lecturas (id_nodo, temperatura, nivel_humo, timestamp)
                VALUES (?, ?, ?, ?)
            ''', (payload['id'], payload['temp'], payload['humo'], now))

        elif msg.topic in ('incendios/alerta', 'incendios/cambio_brusco'):
            c.execute('''
                INSERT INTO alertas (id_nodo, tipo, mensaje, timestamp)
                VALUES (?, ?, ?, ?)
            ''', (payload['id'], payload['tipo'], json.dumps(payload), now))

        conn.commit()
        conn.close()
    except Exception as e:
        print(f"Error procesando mensaje: {e}")


def mqtt_loop():
    client = mqtt.Client()
    client.on_message = on_message
    client.connect("localhost", 1883, 60)
    client.subscribe("incendios/#")
    client.loop_forever()


@app.route('/api/datos')
def api_datos():
    nodo = request.args.get('nodo', type=int)
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    c = conn.cursor()
    if nodo:
        c.execute('''
            SELECT * FROM lecturas WHERE id_nodo = ?
            ORDER BY timestamp DESC LIMIT 50
        ''', (nodo,))
    else:
        c.execute('''
            SELECT * FROM lecturas ORDER BY timestamp DESC LIMIT 100
        ''')
    rows = [dict(r) for r in c.fetchall()]
    conn.close()
    return jsonify(rows)


@app.route('/api/alertas')
def api_alertas():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    c = conn.cursor()

    query = "SELECT * FROM alertas WHERE 1=1"
    params = []

    nodo = request.args.get('nodo', type=int)
    if nodo:
        query += " AND id_nodo = ?"
        params.append(nodo)

    desde = request.args.get('desde')
    if desde:
        query += " AND timestamp >= ?"
        params.append(desde)

    hasta = request.args.get('hasta')
    if hasta:
        query += " AND timestamp <= ?"
        params.append(hasta)

    query += " ORDER BY timestamp DESC LIMIT 100"
    c.execute(query, params)
    rows = [dict(r) for r in c.fetchall()]
    conn.close()
    return jsonify(rows)


@app.route('/api/resumen')
def api_resumen():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    c = conn.cursor()
    c.execute('''
        SELECT id_nodo, temperatura, nivel_humo, timestamp
        FROM lecturas
        WHERE id IN (SELECT MAX(id) FROM lecturas GROUP BY id_nodo)
    ''')
    rows = [dict(r) for r in c.fetchall()]
    conn.close()
    return jsonify(rows)


@app.route('/')
def index():
    return render_template('index.html')


if __name__ == '__main__':
    init_db()
    t = Thread(target=mqtt_loop, daemon=True)
    t.start()
    app.run(host='0.0.0.0', port=5000, debug=False)
