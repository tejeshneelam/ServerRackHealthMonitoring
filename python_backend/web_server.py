import http.server
import socketserver
import os
import json
import urllib.parse
from ml_engine import process_telemetry

PORT = 8000
DATA_FILE = os.path.join(os.path.dirname(__file__), 'latest_data.json')
HTML_FILE = os.path.join(os.path.dirname(__file__), 'index.html')

SCENARIOS = {
    "nominal": {
        "t1": 26.2, "t2": 26.5, "t3": 26.9, "t4": 27.4,
        "ambT": 25.0, "ambH": 52.0,
        "gas": 340, "gasVolt": 0.28,
        "vibe": 0, "vibeCount": 0,
        "relay": 0, "boot": 105, "peakT": 27.4, "lifeShocks": 12,
        "busV": 5.0, "currMA": 0.0, "powerMW": 0.0
    },
    "thermal_spike": {
        "t1": 28.5, "t2": 31.8, "t3": 34.5, "t4": 37.2,
        "ambT": 25.5, "ambH": 48.0,
        "gas": 490, "gasVolt": 0.40,
        "vibe": 0, "vibeCount": 0,
        "relay": 1, "boot": 105, "peakT": 37.2, "lifeShocks": 12,
        "busV": 5.0, "currMA": 182.0, "powerMW": 910.0
    },
    "bearing_friction": {
        "t1": 29.0, "t2": 29.6, "t3": 30.8, "t4": 31.4,
        "ambT": 25.2, "ambH": 50.0,
        "gas": 380, "gasVolt": 0.31,
        "vibe": 1, "vibeCount": 18,
        "relay": 0, "boot": 105, "peakT": 31.4, "lifeShocks": 32,
        "busV": 5.0, "currMA": 0.0, "powerMW": 0.0
    },
    "cable_smolder": {
        "t1": 26.8, "t2": 27.1, "t3": 27.4, "t4": 27.9,
        "ambT": 25.0, "ambH": 54.0,
        "gas": 1450, "gasVolt": 1.85,
        "vibe": 0, "vibeCount": 0,
        "relay": 1, "boot": 105, "peakT": 27.9, "lifeShocks": 12,
        "busV": 5.0, "currMA": 180.0, "powerMW": 900.0
    },
    "stack_inversion": {
        "t1": 34.5, "t2": 32.8, "t3": 29.2, "t4": 26.5,
        "ambT": 28.0, "ambH": 62.0,
        "gas": 420, "gasVolt": 0.35,
        "vibe": 0, "vibeCount": 0,
        "relay": 1, "boot": 105, "peakT": 34.5, "lifeShocks": 12,
        "busV": 5.0, "currMA": 180.0, "powerMW": 900.0
    }
}

class HealthMonitorHandler(http.server.BaseHTTPRequestHandler):
    def end_headers(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Cache-Control', 'no-cache, no-store, must-revalidate')
        super().end_headers()

    def do_GET(self):
        parsed = urllib.parse.urlparse(self.path)
        clean_path = parsed.path
        query_params = urllib.parse.parse_qs(parsed.query)

        if clean_path in ('/api/scenario', '/scenario'):
            scenario_name = query_params.get('name', ['nominal'])[0]
            if scenario_name in SCENARIOS:
                data = dict(SCENARIOS[scenario_name])
                data = process_telemetry(data)
                with open(DATA_FILE, 'w') as f:
                    json.dump(data, f)
                self.send_response(200)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"status": "ok", "scenario": scenario_name, "data": data}).encode('utf-8'))
                return
            else:
                self.send_response(400)
                self.send_header('Content-Type', 'application/json')
                self.end_headers()
                self.wfile.write(json.dumps({"error": f"Unknown scenario: {scenario_name}"}).encode('utf-8'))
                return

        if clean_path in ('/data', '/api/data'):
            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.end_headers()
            try:
                if os.path.exists(DATA_FILE):
                    with open(DATA_FILE, 'r') as f:
                        data = json.load(f)
                else:
                    data = dict(SCENARIOS["nominal"])
            except Exception:
                data = dict(SCENARIOS["nominal"])
            
            try:
                data = process_telemetry(data)
            except Exception:
                pass

            self.wfile.write(json.dumps(data).encode('utf-8'))
            
        elif clean_path in ('/', '/index.html'):
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.end_headers()
            if os.path.exists(HTML_FILE):
                with open(HTML_FILE, 'rb') as f:
                    self.wfile.write(f.read())
            else:
                self.wfile.write(b"<h1>Dashboard HTML not found</h1>")
        else:
            self.send_response(404)
            self.end_headers()

    def log_message(self, format, *args):
        # Suppress routine console spam
        pass

def run_server():
    socketserver.TCPServer.allow_reuse_address = True
    with socketserver.TCPServer(("", PORT), HealthMonitorHandler) as httpd:
        print(f"Edge ML Cyber-Physical Server active at http://localhost:{PORT}")
        httpd.serve_forever()

if __name__ == '__main__':
    run_server()
