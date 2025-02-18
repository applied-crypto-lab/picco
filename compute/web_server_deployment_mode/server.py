from flask import Flask, request, render_template, jsonify
import os
import json
import pandas as pd  # For handling CSV files

app = Flask(__name__)

# Folder to store uploaded CSVs
UPLOAD_FOLDER = 'uploads'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

CONFIG_FILE_PATH = '_picco_input_config.json'

with open(CONFIG_FILE_PATH) as f:
    config_data = json.load(f)

@app.route('/')
def index():
    return render_template('dynamic_form.html', config_data=config_data)

@app.route('/get-config', methods=['GET'])
def get_config():
    return jsonify(config_data)

@app.route('/submit-config', methods=['POST'])
def submit_config():
    form_data = request.form.to_dict()
    files = request.files

    # Process submitted data (add validation here)
    return jsonify({"message": "Form submitted successfully!", "data": form_data})

if __name__ == '__main__':
    app.run(host='127.0.0.1', port=8000, debug=True)
