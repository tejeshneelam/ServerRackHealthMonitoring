// Stage 2: I2C Bus & OLED Display (SSD1306) Verification
// Board: DOIT ESP32 DEVKIT V1
// I2C Pins: SDA = GPIO 21, SCL = GPIO 22

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C // Standard I2C address for 0.96" OLED

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n--- [STAGE 2] I2C & OLED DIAGNOSTIC ---");

  // Initialize I2C
  Wire.begin(21, 22);

  // Scan I2C bus first to verify wiring
  Serial.println("Scanning I2C bus on GPIO 21 (SDA) and GPIO 22 (SCL)...");
  byte count = 0;
  for (byte i = 8; i < 127; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found I2C device at address: 0x");
      if (i < 16) Serial.print("0");
      Serial.println(i, HEX);
      count++;
    }
  }
  if (count == 0) {
    Serial.println("WARNING: No I2C devices found. Check your 4 wires (VCC, GND, SDA, SCL)!");
  } else {
    Serial.printf("Success: %d device(s) found on I2C bus.\n", count);
  }

  // Initialize OLED Display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed. Check OLED wiring."));
    while (true); // Halt if OLED not found
  }

  Serial.println("OLED Display initialized successfully!");

  // Display Splash Screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(10, 5);
  display.println("DATA CENTER RACK");
  display.setCursor(18, 18);
  display.println("HEALTH MONITOR");

  display.drawLine(0, 30, 127, 30, SSD1306_WHITE);

  display.setCursor(5, 36);
  display.println("PI-SSET Digital Twin");
  display.setCursor(5, 48);
  display.println("E-CSGI Causal Engine");

  display.display();
  delay(2500);
}

int frame = 0;

void loop() {
  frame++;

  display.clearDisplay();

  // Header
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("RACK #1 ");
  display.setCursor(64, 0);
  display.print("[HEALTHY]");

  display.drawLine(0, 10, 127, 10, SSD1306_WHITE);

  // Simulated 4-Tier Rack Thermal Status
  display.setCursor(0, 15);
  display.print("U4 (Top):  27.4 C");

  display.setCursor(0, 26);
  display.print("U3 (Mid):  26.8 C");

  display.setCursor(0, 37);
  display.print("U2 (Pwr):  26.2 C");

  display.setCursor(0, 48);
  display.print("U1 (Btm):  25.5 C");

  // Bottom Activity Bar
  int barWidth = (frame % 30) * 4;
  display.drawRect(0, 59, 128, 5, SSD1306_WHITE);
  display.fillRect(0, 59, barWidth, 5, SSD1306_WHITE);

  display.display();
  delay(200);
}
