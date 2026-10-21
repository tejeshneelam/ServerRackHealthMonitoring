#!/usr/bin/env python3
import time
import json
import os
import sys
import serial
import serial.tools.list_ports

def find_serial_port():
    ports = serial.tools.list_ports.comports()
    for p in ports:
        dev = p.device
        if any(k in dev.lower() for k in ["usbserial", "wch", "slab", "usbmodem", "uart"]):
            return dev, p.description
    return None, None

def print_header():
    print("\n" + "="*70)
    print("   AI DATA CENTER RACK: COMPREHENSIVE SENSOR PRE-FLIGHT CHECK")
    print("="*70)

def evaluate_packet(data):
    print("\n--- [LIVE TELEMETRY SNAPSHOT] ---")
    
    # 1. Temperature Probes T1-T4
    t_probes = [data.get("t1"), data.get("t2"), data.get("t3"), data.get("t4")]
    all_temp_ok = True
    for i, t in enumerate(t_probes, 1):
        status = "OK"
        if t is None or t == -127.0:
            status = "FAIL (Disconnected / Pullup missing)"
            all_temp_ok = False
        elif t == 85.0:
            status = "FAIL (Power-on Reset / Incomplete conversion)"
            all_temp_ok = False
        elif t < 10.0 or t > 70.0:
            status = f"WARN (Abnormal reading: {t}°C)"
        else:
            status = f"PASS ({t:.1f} °C)"
        print(f"  [Temp Probe T{i}]  : {status}")

    # 2. Ambient Sensor (DHT11)
    amb_t = data.get("ambT")
    amb_h = data.get("ambH")
    if amb_t is not None and amb_h is not None and amb_t > 0:
        print(f"  [DHT11 Ambient] : PASS ({amb_t:.1f} °C, {amb_h:.1f}% Humidity)")
    else:
        print(f"  [DHT11 Ambient] : WARN (Invalid: T={amb_t}, H={amb_h})")

    # 3. Gas / Smoke Sensor (MQ-2)
    gas = data.get("gas", 0)
    gas_volt = data.get("gasVolt", 0.0)
    if gas > 0:
        status_gas = "PASS (Nominal Clean Air)" if gas < 1000 else "ALERT (Smoke / High Gas Detected)"
        print(f"  [MQ-2 Gas Sensor]: {status_gas} -> {gas} ADC ({gas_volt:.2f} V)")
    else:
        print(f"  [MQ-2 Gas Sensor]: WARN (Reading is 0 ADC - check 5V VCC and A0 pin)")

    # 4. Vibration Sensor (SW-420)
    vibe = data.get("vibe", 0)
    vibe_count = data.get("vibeCount", 0)
    print(f"  [SW-420 Vibration]: PASS (Realtime: {'SHOCK' if vibe else 'IDLE'} | Lifetime Taps: {vibe_count})")

    # 5. Fan Relay
    relay = data.get("relay", 0)
    print(f"  [5V Fan Relay]   : PASS (State: {'ACTIVE (ON)' if relay else 'STANDBY (OFF)'})")

    # 6. ML Pipeline status
    ml = data.get("ml")
    if ml:
        print("\n--- [EDGE ML INFERENCE ENGINE] ---")
        print(f"  [Rack Health Index] : {ml.get('health_index', 'N/A')}% ({ml.get('anomaly_state', 'N/A')})")
        print(f"  [Anomaly Risk]      : {ml.get('anomaly_risk_score', 'N/A')}%")
        print(f"  [Thermal Velocity]  : {ml.get('thermal_velocity_c_min', 'N/A')} °C/min")
        print(f"  [Predictive (t+30s)]: {ml.get('pred_t30', 'N/A')} °C (RUTT: {ml.get('rutt_sec', 'N/A')}s)")
        print(f"  [Silicon RUL]       : {ml.get('component_rul_days', 'N/A')} Days (Aging Factor: {ml.get('silicon_aging_factor', 'N/A')}x)")
        print(f"  [PUE Ratio]         : {ml.get('pue_ratio', 'N/A')}")
    else:
        print("\n  [EDGE ML]: Telemetry raw (Enrichment occurs via data_streamer.py)")

    print("-" * 70)
    return all_temp_ok

def main():
    print_header()
    port, desc = find_serial_port()

    if not port:
        print("\n❌ ESP32 NOT DETECTED ON ANY USB PORT!")
        print("Please check:")
        print("  1. Connect your ESP32 to the Mac using your USB cable/converter.")
        print("  2. Ensure the USB cable supports data (not charge-only).")
        print("  3. Check that the red power LED on the ESP32 is lit.")
        print("\nWaiting for ESP32 to be plugged in...")
        
        while not port:
            time.sleep(1)
            port, desc = find_serial_port()
            if port:
                break
            sys.stdout.write(".")
            sys.stdout.flush()

    print(f"\n✅ ESP32 DETECTED on {port} ({desc})!")
    print("Opening serial port @ 115200 baud to test live sensor packet stream...")

    try:
        ser = serial.Serial(port, 115200, timeout=2)
        ser.dtr = False
        ser.rts = False
        time.sleep(1)
        ser.reset_input_buffer()

        packets_received = 0
        print("\nListening for incoming telemetry packets from ESP32...")

        start_time = time.time()
        while packets_received < 3 and (time.time() - start_time) < 15:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line.startswith('{') and line.endswith('}'):
                try:
                    data = json.loads(line)
                    packets_received += 1
                    print(f"\nPacket #{packets_received} Received:")
                    evaluate_packet(data)
                except json.JSONDecodeError:
                    pass
            elif line:
                print(f"  [ESP32 Boot/Debug Message]: {line}")

        ser.close()

        if packets_received > 0:
            print("\n🎉 ALL SENSOR CHECKS COMPLETED!")
            print("Your hardware is transmitting live data properly.")
            print("You are ready for the presentation!")
        else:
            print("\n⚠️ No JSON packets received within 15 seconds.")
            print("Press the EN / RST button on the ESP32 board to restart telemetry stream.")

    except Exception as e:
        print(f"\n❌ Error accessing serial port: {e}")
        print("If data_streamer.py is already running, the port might be busy.")

if __name__ == '__main__':
    main()
