import sqlite3
import csv
import io
from flask import Response
from database import DB_NAME

def generate_csv_response(session_id):
    conn = sqlite3.connect(DB_NAME)
    cursor = conn.cursor()
    
    try:
        cursor.execute(f"SELECT * FROM data_session_{session_id} ORDER BY id ASC")
        rows = cursor.fetchall()
        
        output = io.StringIO()
        writer = csv.writer(output)
        
        # Header CSV (Menambahkan kolom 'Status Fuzzy')
        writer.writerow(['ID', 'Waktu', 'Node ID', 'Counter', 'Roll (°)', 'Pitch (°)', 'Curah Hujan', 'Soil Moisture (%)', 'Temp (°C)', 'VBat (V)', 'RSSI (dBm)', 'SNR (dB)', 'Status Fuzzy'])
        writer.writerows(rows)
        
        output.seek(0)
        return Response(
            output.getvalue(),
            mimetype="text/csv",
            headers={"Content-Disposition": f"attachment;filename=dataset_session_{session_id}.csv"}
        )
    finally:
        conn.close()