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
        cursor.execute("SELECT session_name, created_at FROM sessions WHERE id = ?", (session_id,))
        session_info = cursor.fetchone()

        if session_info and session_info['created_at']:
            try:
                dt = datetime.strptime(session_info['created_at'], '%Y-%m-%d %H:%M:%S')
                time_str = dt.strftime('%Y%m%d_%H%M%S')
            except ValueError:
                time_str = datetime.now().strftime('%Y%m%d_%H%M%S')
        else:
            time_str = datetime.now().strftime('%Y%m%d_%H%M%S')

        filename = f"Session_{session_id}_{time_str}.csv"

        cursor.execute(f"SELECT * FROM data_session_{session_id} ORDER BY id ASC")
        rows = cursor.fetchall()

        output = io.StringIO()
        writer = csv.writer(output)

        # Header CSV mencakup Yaw (°)
        writer.writerow([
            'ID', 'Waktu', 'Node ID', 'Counter',
            'Roll (°)', 'Pitch (°)', 'Yaw (°)',
            'Curah Hujan', 'Soil Moisture (%)',
            'Suhu Box (°C)', 'RH Box (%)',
            'RSSI (dBm)', 'SNR (dB)', 'Status Fuzzy'
        ])

        for row in rows:
            row_dict = dict(row)
            writer.writerow([
                row_dict.get('id'),
                row_dict.get('waktu'),
                row_dict.get('node_id'),
                row_dict.get('counter'),
                row_dict.get('roll'),
                row_dict.get('pitch'),
                row_dict.get('yaw', 0.0),
                row_dict.get('rain'),
                row_dict.get('soil_moisture'),
                row_dict.get('temp'),
                row_dict.get('vbat'),
                row_dict.get('rssi'),
                row_dict.get('snr'),
                row_dict.get('fuzzy_status')
            ])

        output.seek(0)
        return Response(
            output.getvalue(),
            mimetype="text/csv",
            headers={"Content-Disposition": f"attachment;filename={filename}"}
        )
    finally:
        conn.close()