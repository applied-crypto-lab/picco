from flask import Flask, request, render_template, jsonify, make_response, redirect
import os
import json
import threading
import pandas as pd  # For handling CSV files
import time
import sys

app = Flask(__name__)

if len(sys.argv) < 6:
    print("Usage: python3 server.py <share_base_name> <config_json_file> <passcode_file> [host] [port]")
    sys.exit(1)

# Default host and port
HOST = '127.0.0.1'
PORT = 8000

# If user provided custom host and/or port
if len(sys.argv) >= 5:
    HOST = sys.argv[4]
if len(sys.argv) == 6:
    try:
        PORT = int(sys.argv[5])
    except ValueError:
        print("[ERROR] Invalid port number.")
        sys.exit(1)

share_base_name_counter = 1
SHARE_BASE_NAME = sys.argv[1]
SHARE_DIR, SHARE_PREFIX = os.path.split(SHARE_BASE_NAME)
SHARE_DIR = os.path.abspath(SHARE_DIR)
os.makedirs(SHARE_DIR, exist_ok=True)

CONFIG_FILE_PATH = sys.argv[2]
SECRET_FILE_PATH = sys.argv[3]

# Get the data and upload it
try:
    with open(SECRET_FILE_PATH) as f:
        auth_data = json.load(f)['users']
except Exception as e:
    print(f"[ERROR] Failed to read secret/passcode file: {SECRET_FILE_PATH}")
    print(e)
    sys.exit(1)

# Folder to store uploaded CSVs
UPLOAD_FOLDER = 'uploads'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)

try:
    with open(CONFIG_FILE_PATH) as f:
        config_data = json.load(f)
except Exception as e:
    print(f"[ERROR] Failed to load config file: {CONFIG_FILE_PATH}")
    print(e)
    sys.exit(1)

@app.route('/')
def index():
    party_id = request.cookies.get('party_id')
    if not party_id:
        return redirect('/register')

    party_id = int(party_id)

    # Just filter config_data in memory for this party
    party_vars = [
        var for var in config_data.get('variables', [])
        if isinstance(var, dict) and var.get('input_party') == user_party
    ]

    response_data = {
        "settings": config_data.get("settings", []),
        "peers": config_data.get("peers", []),
        "variables": party_vars
    }

    print(f"Server-side response for party {user_party}:")
    print(json.dumps(response_data, indent=2))

    return render_template('dynamic_form.html', config_data=response_data)

@app.route('/get-config', methods=['GET'])
def get_config():
    return jsonify(config_data)

@app.route('/favicon.ico')
def favicon():
    return app.send_static_file('favicon.ico')

@app.route('/register')
def register():
    return render_template('login.html')

@app.route('/login', methods=['POST'])
def login():
    user_id = request.form.get('user_id')

    with open(SECRET_FILE_PATH) as f:
        auth_data = json.load(f)['users']

    # Find user by passcode
    user = next((u for u in auth_data if u['passcode'] == user_id), None)

    if not user:
        return "Invalid Passcode Number!", 401

    user_party = user['input_party']

    # Filter config data at runtime based on input_party
    party_vars = [
        var for var in config_data.get('variables', [])
        if isinstance(var, dict) and var.get('input_party') == user_party
    ]

    response_data = {
        "settings": config_data.get("settings", []),
        "peers": config_data.get("peers", []),
        "variables": party_vars
    }

    print(f"Server-side response for party {user_party}:")
    print(json.dumps(response_data, indent=2))

    # Set party_id in cookie
    response = make_response(render_template('dynamic_form.html', config_data=response_data))
    response.set_cookie('party_id', str(user_party))

    return response


@app.route('/submit-config', methods=['POST'])
def submit_config():
    try:
        data = request.get_json()

        print(f"Received data type: {type(data)}")
        print(f"Received data: {data}")

        if not data:
            return jsonify({"error": "No data received"}), 400

        # --- MODIFICATION START ---
        # The client sends data wrapped in an object with the key 'encrypted_variable_shares'
        encrypted_shares_array = data.get('encrypted_variable_shares')

        if not encrypted_shares_array:
            return jsonify({"error": "Missing 'encrypted_variable_shares' in the request body"}), 400

        # Now, iterate over the actual array of encrypted shares
        # Ensure it's a list as expected
        if not isinstance(encrypted_shares_array, list):
            return jsonify({"error": "Value of 'encrypted_variable_shares' is not an array"}), 400
        # --- MODIFICATION END ---

        # Define the directory where server.py is running.
        OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__)) # __file__ is the path to the current script
        # Ensure the output directory exists
        os.makedirs(OUTPUT_DIR, exist_ok=True)

        global share_base_name_counter

        # Iterate through each peer's data in the extracted array
        for peer_data in encrypted_shares_array: # Now iterating over the correct array
            if not isinstance(peer_data, dict):
                return jsonify({"error": "Expected each item in the array to be a JSON object"}), 400

            peer_number = peer_data.get('peer')
            shares = peer_data.get('shares')

            if peer_number is None or shares is None:
                return jsonify({"error": "Missing 'peer' number or 'shares' array in one of the data items"}), 400
            
            if not isinstance(shares, list):
                return jsonify({"error": f"Shares for peer {peer_number} is not an array"}), 400

            # Create a file for each peer
            file_name = f"{SHARE_PREFIX}_{share_base_name_counter}_{peer_number}"
            file_path = os.path.join(SHARE_DIR, file_name)

            with open(file_path, 'w') as f:
                # Write each share for the current peer on a new line
                # Each 'share' here could be an array of encrypted components (for floats)
                # or a single-element array (for integers)
                for share_item in shares:
                    if isinstance(share_item, list):
                        # If it's an array (e.g., float components), join them
                        f.write(f"{','.join(map(str, share_item))}\n")
                    else:
                        # If it's a single share (though your JS wraps int shares in [encrypted]),
                        # this provides a fallback for single values.
                        f.write(f"{share_item}\n")

            print(f"Saved shares for Peer {peer_number} to {file_path}")
        
        # Increment the counter *after* processing all shares for the current submission
        share_base_name_counter += 1 

        return jsonify({"message": "Inputs received and stored."}), 200

    except Exception as e:
        import traceback
        print("Error occurred:", e)
        traceback.print_exc()
        return jsonify({"error": str(e)}), 500


if __name__ == '__main__':
    app.run(host=HOST, port=PORT, debug=True)

