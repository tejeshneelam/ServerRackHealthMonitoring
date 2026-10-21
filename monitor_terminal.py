#!/usr/bin/env python3
import time
import json
import os
import sys

DATA_FILE = os.path.join(os.path.dirname(__file__), 'python_backend', 'latest_data.json')

def clear_screen():
    sys.stdout.write("\033[2J\033[H")
    sys.stdout.flush()

def main():
    print("Connecting to live telemetry feed... Press Ctrl+C to exit.")
    time.sleep(1)
    
    last_boot = -1
    last_time = 0

    while True:
        try:
            if os.path.exists(DATA_FILE):
                with open(DATA_FILE, 'r') as f:
                    data = json.load(f)

                t1 = data.get("t1", 0.0)
                t2 = data.get("t2", 0.0)
                t3 = data.get("t3", 0.0)
                t4 = data.get("t4", 0.0)
                ambT = data.get("ambT", 0.0)
                ambH = data.get("ambH", 0.0)
                gas = data.get("gas", 0)
                gasVolt = data.get("gasVolt", 0.0)
                vibe = data.get("vibe", 0)
                vibeCount = data.get("vibeCount", 0)
                relay = data.get("relay", 0)
                boot = data.get("boot", 0)
                peakT = data.get("peakT", 0.0)

                ml = data.get("ml", {})
                rhi = ml.get("health_index", 0.0)
                risk = ml.get("anomaly_risk_score", 0.0)
                vel = ml.get("thermal_velocity_c_min", 0.0)
                pred30 = ml.get("pred_t30", 0.0)
                rutt = ml.get("rutt_sec", 999)
                state = ml.get("anomaly_state", "NOMINAL")
                rul = ml.get("component_rul_days", 1825.0)

                vibe_str = "\033[1;31mSHOCK ACTIVE\033[0m" if vibe else "IDLE [OK]"
                relay_str = "\033[1;32mON (Cooling)\033[0m" if relay else "\033[1;34mSTANDBY (Off)\033[0m"
                gas_str = f"\033[1;32m{gas} ADC ({gasVolt:.2f}V) [SAFE]\033[0m" if gas < 1000 else f"\033[1;31m{gas} ADC ({gasVolt:.2f}V) [HAZARD!]\033[0m"
                rutt_str = "\033[1;32mSAFE (>10 min)\033[0m" if rutt > 300 else f"\033[1;31m{rutt}s UNTIL SHUTDOWN\033[0m"
                state_str = f"\033[1;32m{state}\033[0m" if "NOMINAL" in state else f"\033[1;31m{state}\033[0m"

                clear_screen()
                print("=" * 68)
                print("   AI DATA CENTER RACK HEALTH MONITOR — LIVE TERMINAL CONSOLE")
                print("=" * 68)
                print(f" OPERATIONAL STATE : {state_str}")
                print(f" RACK HEALTH INDEX : \033[1;36m{rhi:.1f}%\033[0m   |   ANOMALY RISK : \033[1;33m{risk:.1f}%\033[0m")
                print("-" * 68)
                print(" [CORE THERMAL STRATIFICATION]")
                print(f"   Tier 1 (Top Exhaust) : \033[1;37m{t1:.2f} °C\033[0m   |   Tier 2 (Upper Mid)  : {t2:.2f} °C")
                print(f"   Tier 3 (Lower Mid)   : {t3:.2f} °C   |   Tier 4 (Bottom Int) : {t4:.2f} °C")
                print(f"   Thermal Velocity     : \033[1;35m{vel:+.2f} °C/min\033[0m  |   Forecast (t+30s)    : {pred30:.2f} °C")
                print(f"   RUTT Countdown       : {rutt_str}")
                print("-" * 68)
                print(" [ENVIRONMENT & EDGE SENSORS]")
                print(f"   Ambient Intake Air   : {ambT:.1f} °C  ({ambH:.0f}% RH)")
                print(f"   Gas / Smoke Sensor   : {gas_str}")
                print(f"   Vibration (SW-420)   : {vibe_str}  (Lifetime Taps: {vibeCount})")
                print(f"   Cooling Fan Relay    : {relay_str}")
                print(f"   Silicon Aging RUL    : {rul:.0f} Days")
                print(f"   NVS Black-Box        : Boot #{boot} | Peak Temp: {peakT:.1f} °C")
                print("=" * 68)
                print(" Streaming live at 1 Hz from ESP32 via data_streamer.py")
                print(" Web Dashboard active at: http://localhost:8000")

            time.sleep(1)
        except KeyboardInterrupt:
            print("\nExiting Terminal Monitor.")
            break
        except Exception:
            time.sleep(1)

if __name__ == '__main__':
    main()
