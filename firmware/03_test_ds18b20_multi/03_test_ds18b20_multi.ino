// Stage 3: Multi-Sensor 1-Wire Bus Verification (4x DS18B20 Probes)
// Core Sensing Engine for PI-SSET Digital Twin
// Protocol: Dallas 1-Wire on GPIO 4 with a 4.7k pull-up to 3.3V
// Displays live temperature readings on both Serial and OLED!

#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 1-Wire Setup
#define ONE_WIRE_BUS 4 // GPIO 4 on ESP32
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

int deviceCount = 0;
DeviceAddress tempDeviceAddress;

// Helper to print 64-bit ROM Address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
    if (i < 7) Serial.print(":");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n--- [STAGE 3] PI-SSET 1-WIRE MULTI-TEMP SENSOR CHECK ---");

  // Init OLED
  Wire.begin(21, 22);
  if (display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Scanning 1-Wire...");
    display.display();
  }

  // Start 1-Wire bus
  sensors.begin();
  deviceCount = sensors.getDeviceCount();

  Serial.print("Locating DS18B20 devices on GPIO 4... Found: ");
  Serial.println(deviceCount);

  if (deviceCount == 0) {
    Serial.println("WARNING: No DS18B20 sensors detected!");
    Serial.println("Check: 1) Is the 4.7k resistor between 3.3V and Data?");
    Serial.println("       2) Are Red to 3.3V, Black to GND, Data to GPIO 4?");
  }

  for (int i = 0; i < deviceCount; i++) {
    if (sensors.getAddress(tempDeviceAddress, i)) {
      Serial.print("  [Probe #");
      Serial.print(i + 1);
      Serial.print("] ROM ID: ");
      printAddress(tempDeviceAddress);
      Serial.print(" | Resolution: ");
      Serial.print(sensors.getResolution(tempDeviceAddress));
      Serial.println(" bits");
    }
  }
  Serial.println("----------------------------------------------------------\n");
}

void loop() {
  // Request temperature from all sensors on bus
  sensors.requestTemperatures();

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("PI-SSET THERMAL TIERS");
  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  Serial.print("[THERMAL SNAPSHOT] ");

  for (int i = 0; i < 4; i++) {
    float tempC = -127.0;
    if (i < deviceCount) {
      tempC = sensors.getTempCByIndex(i);
    }

    // Print to Serial
    Serial.print("T");
    Serial.print(i + 1);
    Serial.print(": ");
    if (tempC == -127.00) {
      Serial.print("N/A  ");
    } else {
      Serial.print(tempC, 2);
      Serial.print(" C  ");
    }

    // Print to OLED
    display.setCursor(0, 15 + (i * 12));
    display.print("Tier ");
    display.print(i + 1);
    display.print(": ");
    if (tempC == -127.00) {
      display.print("--.- C");
    } else {
      display.print(tempC, 1);
      display.print(" C");
    }
  }

  Serial.println();
  display.display();
  delay(1000);
}
