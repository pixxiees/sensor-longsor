import sqlite3
import csv
import io
from datetime import datetime
from flask import Response
from database import DB_NAME

def generate_csv_response(session_id):
    conn = sqlite3.connect(DB_NAME)
    conn.row_factory = sqlite3.Row
    cursor = conn.cursor()

    try:
        # info session (nama & timestamp pembuatan) untuk nama file
        cursor.execute("SELECT session_name, created_at FROM sessions WHERE id = ?", (session_id,))
        session_info = cursor.fetchone()

        # Buat format timestamp
        if session_info and session_info['created_at']:
            try:
                dt = datetime.strptime(session_info['created_at'], '%Y-%m-%d %H:%M:%S')
                time_str = dt.strftime('%Y%m%d_%H%M%S')
            except ValueError:
                time_str = datetime.now().strftime('%Y%m%d_%H%M%S')
        else:
            time_str = datetime.now().strftime('%Y%m%d_%H%M%S')

        # Format Nama File
        filename = f"Session_{session_id}_{time_str}.csv"

        # Ambil data sensor dari tabel dinamis
        cursor.execute(f"SELECT * FROM data_session_{session_id} ORDER BY id ASC")
        rows = cursor.fetchall()

        output = io.StringIO()
        writer = csv.writer(output)

        # Header CSV
        writer.writerow(['ID', 'Waktu', 'Node ID', 'Counter', 'Roll (°)', 'Pitch (°)', 'Curah Hujan', 'Soil Moisture (%)', 'Temp (°C)', 'VBat (V)', 'RSSI (dBm)', 'SNR (dB)', 'Status Fuzzy'])

        for row in rows:
            writer.writerow(list(row))

        output.seek(0)
        return Response(
            output.getvalue(),
            mimetype="text/csv",
            headers={"Content-Disposition": f"attachment;filename={filename}"}
        )
    finally:
        conn.close()