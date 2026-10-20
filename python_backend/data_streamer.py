import serial
import time
import json
import os
from ml_engine import process_telemetry

SERIAL_PORT = '/dev/cu.usbserial-0001'
BAUD_RATE = 115200
DATA_FILE = os.path.join(os.path.dirname(__file__), 'latest_data.json')
TEMP_FILE = os.path.join(os.path.dirname(__file__), 'latest_data.tmp.json')

def get_default_data():
    return {
        "t1": 25.0, "t2": 25.0, "t3": 25.0, "t4": 25.0,
        "ambT": 25.0, "ambH": 50.0,
        "gas": 0, "gasVolt": 0.0,
        "vibe": 0, "vibeCount": 0
    }

def main():
    print(f"Starting Resilient Data Streamer on {SERIAL_PORT} @ {BAUD_RATE}")
    
    if not os.path.exists(DATA_FILE):
        with open(DATA_FILE, 'w') as f:
            json.dump(get_default_data(), f)

    while True:
        ser = None
        try:
            ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
            ser.dtr = False
            ser.rts = False
            time.sleep(1) # Settle
            ser.reset_input_buffer()
            print("Connected! Listening for live telemetry...")

            while True:
                line = ser.readline().decode('utf-8', errors='ignore').strip()
                if line.startswith('{') and line.endswith('}'):
                    try:
                        data = json.loads(line)
                        # Enrich with Cyber-Physical Edge ML Inference
                        data = process_telemetry(data)
                        with open(TEMP_FILE, 'w') as f:
                            json.dump(data, f)
                        os.replace(TEMP_FILE, DATA_FILE)
                        ml = data.get("ml", {})
                        print(f"[EDGE ML] RHI: {ml.get('health_index')}% | Anomaly Risk: {ml.get('anomaly_risk_score')}% | dT/dt: {ml.get('thermal_velocity_c_min')}°C/min | Pred@30s: {ml.get('pred_t30')}°C | State: {ml.get('anomaly_state')}")
                    except json.JSONDecodeError:
                        pass
                elif line:
                    print(f"[ESP32 RAW]: {line}")
                    
        except (serial.SerialException, OSError) as e:
            print(f"Serial port disconnected or busy ({e}). Reconnecting in 2s...")
            time.sleep(2)
        except KeyboardInterrupt:
            print("\nStopping Streamer.")
            break
        finally:
            if ser and ser.is_open:
                try:
                    ser.close()
                except Exception:
                    pass

if __name__ == '__main__':
    main()
