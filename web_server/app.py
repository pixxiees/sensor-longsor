from datetime import datetime, timezone, timedelta
import os
import sqlite3
from flask import Flask, request, render_template, jsonify

# 1. Tentukan lokasi database yang pasti
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DB_FILE = os.path.join(BASE_DIR, 'ews_data.db')

app = Flask(__name__)

# 2. Deklarasi fungsi init_db
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
            soil_moisture REAL,
            temp REAL,
            vbat REAL,
            rssi REAL,
            snr REAL
        )
    ''')
    conn.commit()
    conn.close()

# 3. PANGGIL FUNGSINYA DI SINI! (Pastikan nempel di pinggir kiri, tidak ada spasi/tab)
init_db()

# 4. Baru masuk ke routing

@app.route('/')
def index():
    conn = sqlite3.connect(DB_FILE)
    conn.row_factory = sqlite3.Row
    c = conn.cursor()
    c.execute('SELECT * FROM sensor_logs ORDER BY id DESC LIMIT 50')
    rows = c.fetchall()

    # PERBAIKAN: Ubah format row SQLite menjadi format Dictionary biasa
    # agar fitur 'tojson' di HTML tidak error
    data = [dict(row) for row in rows]

    conn.close()
    return render_template('index.html', data=data)

@app.route('/api/data', methods=['POST'])
def receive_data():
    try:
        # Menangkap paket JSON dari ESP32
        data = request.get_json()

        # Mengekstrak data (sekarang termasuk node_id)
        node_id = data.get('node_id', 'NODE_UNKNOWN')
        counter = data.get('counter', 0)
        roll = data.get('roll', 0.0)
        pitch = data.get('pitch', 0.0)
        rain = data.get('rain', 0.0)
        soil = data.get('soil_moisture', 0.0)
        temp = data.get('temp', 0.0)
        vbat = data.get('vbat', 0.0)
        rssi = data.get('rssi', 0.0)
        snr = data.get('snr', 0.0)

        # Menyimpan ke database SQLite
        # Buat definisi zona waktu WIB (UTC +7)
        wib = timezone(timedelta(hours=7))

        # Ambil waktu saat ini berdasarkan zona waktu tersebut
        waktu_sekarang = datetime.now(wib).strftime("%Y-%m-%d %H:%M:%S")

        # Menyimpan ke database SQLite
        conn = sqlite3.connect(DB_FILE)
        c = conn.cursor()
        c.execute('''
         INSERT INTO sensor_logs
         (timestamp, node_id, counter, roll, pitch, rain, soil_moisture, temp, vbat, rssi, snr)
         VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (waktu_sekarang, node_id, counter, roll, pitch, rain, soil, temp, vbat, rssi, snr))
        conn.commit()
        conn.close()

        return jsonify({"status": "success", "message": "Data tersimpan"}), 200

    except Exception as e:
        print("Error:", e)
        return jsonify({"status": "error", "message": str(e)}), 500
    return render_template('index.html', data=data)

if __name__ == '__main__':
    # Tambahkan use_reloader=False agar Flask tidak bentrok dengan aktivitas PlatformIO
    app.run(host='0.0.0.0', port=5000, debug=True, use_reloader=False)