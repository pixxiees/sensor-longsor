import sqlite3
from datetime import datetime

DB_NAME = 'gateway_data.db'

def get_db():
    conn = sqlite3.connect(DB_NAME)
    conn.row_factory = sqlite3.Row
    return conn

def init_db():
    """Inisialisasi tabel master sessions jika belum ada."""
    with get_db() as conn:
        conn.execute('''
            CREATE TABLE IF NOT EXISTS sessions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                session_name TEXT NOT NULL,
                created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        conn.commit()

def create_new_session(name=None):
    """Membuat session/tabel baru khusus untuk pengujian yang baru dinyalakan."""
    if not name:
        name = f"Session_{datetime.now().strftime('%Y%m%d_%H%M%S')}"

    with get_db() as conn:
        cursor = conn.execute("INSERT INTO sessions (session_name) VALUES (?)", (name,))
        session_id = cursor.lastrowid

        # Membuat tabel dinamis khusus dengan kolom fuzzy_status
        conn.execute(f'''
            CREATE TABLE IF NOT EXISTS data_session_{session_id} (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                waktu TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                node_id TEXT, counter INT, roll REAL, pitch REAL,
                rain INT, soil_moisture REAL, temp REAL, vbat REAL,
                rssi REAL, snr REAL, fuzzy_status TEXT
            )
        ''')
        conn.commit()
    return session_id

def save_gateway_data(data):
    """Menyimpan data dari ESP32 murni ke sesi aktif saat ini."""
    with get_db() as conn:
        cursor = conn.execute("SELECT id FROM sessions ORDER BY id DESC LIMIT 1")
        active = cursor.fetchone()

        # Jika belum pernah ada sesi sama sekali di awal, baru buatkan
        session_id = active['id'] if active else create_new_session()

        # Otomatis menambahkan kolom fuzzy_status jika tabel dibuat sebelum update
        try:
            conn.execute(f"ALTER TABLE data_session_{session_id} ADD COLUMN fuzzy_status TEXT")
        except sqlite3.OperationalError:
            pass # Kolom sudah ada

        # Ambil status fuzzy dari payload (default 'AMAN' jika kosong)
        fuzzy_status = data.get('fuzzy_status', 'AMAN')

        conn.execute(f'''
            INSERT INTO data_session_{session_id}
            (node_id, counter, roll, pitch, rain, soil_moisture, temp, vbat, rssi, snr, fuzzy_status)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (
            data.get('node_id'), data.get('counter'), data.get('roll'), data.get('pitch'),
            data.get('rain'), data.get('soil_moisture'), data.get('temp'), data.get('vbat'),
            data.get('rssi'), data.get('snr'), fuzzy_status
        ))
        conn.commit()

def get_all_sessions():
    """Mengambil daftar seluruh session yang tersimpan."""
    with get_db() as conn:
        return conn.execute("SELECT * FROM sessions ORDER BY id DESC").fetchall()

def get_session_data(session_id):
    """Mengambil data sensor khusus dari tabel session tertentu."""
    with get_db() as conn:
        try:
            return conn.execute(f"SELECT * FROM data_session_{session_id} ORDER BY id DESC").fetchall()
        except sqlite3.OperationalError:
            return []

def delete_session_db(session_id):
    """Menghapus record session dan tabel dinamisnya."""
    with get_db() as conn:
        conn.execute("DELETE FROM sessions WHERE id = ?", (session_id,))
        conn.execute(f"DROP TABLE IF EXISTS data_session_{session_id}")
        conn.commit()