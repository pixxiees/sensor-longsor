from flask import Flask, request, jsonify, render_template
import sqlite3
import datetime
import os

app = Flask(__name__)
DB_FILE = 'ews_data.db'

# Fungsi untuk inisialisasi tabel SQLite
def init_db():
    conn = sqlite3.connect(DB_FILE)
    c = conn.cursor()
    c.execute('''
        CREATE TABLE IF NOT EXISTS sensor_logs (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT,
            node_id TEXT,
            counter INTEGER,
            roll REAL,
            pitch REAL,
            rain REAL,
            soil_moisture REAL
        )
    ''')
    conn.commit()
    conn.close()

@app.route('/')
def index():
    conn = sqlite3.connect(DB_FILE)
    conn.row_factory = sqlite3.Row
    c = conn.cursor()
    # Mengambil 50 data terbaru
    c.execute('SELECT * FROM sensor_logs ORDER BY id DESC LIMIT 50')
    data = c.fetchall()
    conn.close()
    return render_template('index.html', data=data)

@app.route('/api/data', methods=['POST'])
def receive_data():
    content = request.json
    if content:
        timestamp = datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        conn = sqlite3.connect(DB_FILE)
        c = conn.cursor()
        c.execute('''
            INSERT INTO sensor_logs (timestamp, node_id, counter, roll, pitch, rain, soil_moisture)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (
            timestamp, 
            content.get('node_id', 'UNKNOWN'),
            content.get('counter', 0),
            content.get('roll', 0.0),
            content.get('pitch', 0.0),
            content.get('rain', 0.0),
            content.get('soil_moisture', 0.0)
        ))
        conn.commit()
        conn.close()
        
        print(f"[{timestamp}] Data tersimpan dari {content.get('node_id')}")
        return jsonify({"status": "success", "message": "Data tersimpan di SQLite"}), 200
        
    return jsonify({"status": "error", "message": "Format data salah"}), 400

if __name__ == '__main__':
    init_db()
    # Tambahkan use_reloader=False agar Flask tidak bentrok dengan aktivitas PlatformIO
    app.run(host='0.0.0.0', port=5000, debug=True, use_reloader=False)