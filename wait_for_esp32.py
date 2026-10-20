import time
import sys
import serial.tools.list_ports

print("Scanning for ESP32 USB connection... Plug in your cable now.")
seen_ports = set()

while True:
    ports = serial.tools.list_ports.comports()
    current_ports = {p.device: p.description for p in ports}
    for dev, desc in current_ports.items():
        if "usb" in dev.lower() or "slab" in dev.lower() or "wch" in dev.lower() or "modem" in dev.lower():
            print(f"\n>>> FOUND ESP32 PORT: {dev} ({desc}) <<<")
            sys.exit(0)
    time.sleep(1)
