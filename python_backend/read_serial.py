import serial
import time
import sys

PORT = "/dev/cu.usbserial-0001"
BAUD = 115200

def main():
    print(f"Connecting to ESP32 on {PORT} at {BAUD} baud...")
    try:
        ser = serial.Serial()
        ser.port = PORT
        ser.baudrate = BAUD
        ser.dtr = False
        ser.rts = False
        ser.timeout = 1
        ser.open()
        time.sleep(0.5)
        
        print("Connected! Listening for ESP32 serial data (Press Ctrl+C to stop):\n")
        print("-" * 50)
        
        while True:
            if ser.in_waiting > 0:
                line = ser.readline().decode('utf-8', errors='replace').strip()
                if line:
                    timestamp = time.strftime("%H:%M:%S")
                    print(f"[{timestamp}] {line}")
            time.sleep(0.01)
            
    except serial.SerialException as e:
        print(f"Serial Error: {e}")
    except KeyboardInterrupt:
        print("\nDisconnected.")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == "__main__":
    main()
