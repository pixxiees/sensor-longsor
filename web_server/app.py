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
    # Menambahkan kolom temp, vbat, rssi, snr
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

@app.route('/')
def index():
    conn = sqlite3.connect(DB_FILE)
    conn.row_factory = sqlite3.Row
    c = conn.cursor()
    c.execute('SELECT * FROM sensor_logs ORDER BY id DESC LIMIT 50')
    rows = c.fetchall()
    
    # BARIS INI WAJIB ADA: Mengubah SQLite Row menjadi Dictionary
    data = [dict(row) for row in rows] 
    
    conn.close()
    return render_template('index.html', data=data)

if __name__ == '__main__':
    init_db()
    # Tambahkan use_reloader=False agar Flask tidak bentrok dengan aktivitas PlatformIO
    app.run(host='0.0.0.0', port=5000, debug=True, use_reloader=False)