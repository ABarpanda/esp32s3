import serial
import csv
from datetime import datetime

# === CONFIG ===
PORT = 'COM4'
BAUD_RATE = 115200
FILENAME = f"dht11_log_{datetime.now().strftime('%Y%m%d_%H%M%S')}.csv"

# === START LOGGING ===
with serial.Serial(PORT, BAUD_RATE, timeout=1) as ser, open(FILENAME, 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(["Timestamp", "Humidity (%)", "Temperature (°C)"])  # CSV header

    print(f"Logging to {FILENAME}... Press Ctrl+C to stop.\n")
    try:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        print(f"RAW: {line}")  # DEBUG: See what's coming from ESP32
        while True:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line and ',' in line:
                timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]
                humidity, temperature = line.split(',')  # splits the CSV line
                writer.writerow([timestamp, humidity, temperature])
                print(f"{timestamp}, {humidity}, {temperature}")
    except KeyboardInterrupt:
        print("\nLogging stopped.")
