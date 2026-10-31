from flask import Flask, request, jsonify, render_template
import database as db
import exporter as exp

app = Flask(__name__)

# Inisialisasi tabel master & migrasi kolom saat aplikasi start
db.init_db()

@app.route('/api/data', methods=['POST'])
def receive_data():
    data = request.get_json()
    if not data:
        return jsonify({"status": "error", "message": "No JSON payload"}), 400
    
    db.save_gateway_data(data)
    return jsonify({"status": "success"}), 200

@app.route('/')
def index():
    sessions = db.get_all_sessions()
    selected_id = request.args.get('session_id', type=int)
    
    if not selected_id and sessions:
        selected_id = sessions[0]['id']
        
    sensor_data = db.get_session_data(selected_id) if selected_id else []
    return render_template('index.html', sessions=sessions, selected_session_id=selected_id, sensor_data=sensor_data)

@app.route('/new_session', methods=['POST'])
def new_session():
    name = request.form.get('session_name')
    session_id = db.create_new_session(name)
    return jsonify({"status": "success", "session_id": session_id})

@app.route('/download_csv/<int:session_id>')
def download_csv(session_id):
    return exp.generate_csv_response(session_id)

@app.route('/delete_session/<int:session_id>', methods=['POST'])
def delete_session(session_id):
    db.delete_session_db(session_id)
    return jsonify({"status": "success"})

if __name__ == '__main__':
    app.run(debug=True)