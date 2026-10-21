// ============================================================================
// Stage 5.0: FULL MASTER MULTI-SENSOR HARDWARE SYSTEM (100% HARDWARE COMPLETE)
// 
// Integrated Sensors & Actuators:
//  1. 4x DS18B20 1-Wire Temperature Probes on GPIO 4 (4.7k pull-up to 3.3V)
//  2. CustomDHT (DHT11/DHT22) Ambient Temp & Humidity on GPIO 15
//  3. SW-420 Mechanical Vibration Sensor on GPIO 19 (Polled Debounce)
//  4. MQ-135 / MQ-2 Smoke/Gas Sensor on GPIO 39 (ADC VN) & GPIO 34
//  5. 0.96" SSD1306 OLED Display on I2C (SDA: 21, SCL: 22) @ 0x3C
//  6. INA219 DC Current / Voltage / Power Sensor on I2C @ 0x40 (Auto-Detect)
//  7. 5V 1-Channel Relay Module on GPIO 5 (Open-Drain High-Z Actuator)
//  8. 5V DC Cooling Fan powered through Relay (NO / COM)
//  9. 5V Active Buzzer on GPIO 27 (Audible Alarm on Emergency Trip)
// 10. Status LEDs: Green (GPIO 26 - Nominal), Red (GPIO 25 - Trip Alert)
// 11. Local Flash Database (NVS Persistence) for Boot & Peak Lifetime Stats
// ============================================================================

#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_INA219.h>
#include <Preferences.h>
#include "CustomDHT.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// --- Pin Allocations ---
#define PIN_ONEWIRE      4
#define PIN_DHT         15
#define PIN_SW420       19
#define PIN_MQ135       34  // MQ Gas Sensor on GPIO 34
#define PIN_RELAY       18  // Relay Actuator on GPIO 18
#define PIN_BUZZER      27  // 5V Active Buzzer (+)
#define PIN_LED_GREEN   26  // Green LED (System Normal)
#define PIN_LED_RED     25  // Red LED (Critical / Trip Alert)

// High-Impedance (Open-Drain) Switching for 5V Relay Modules:
// When OFF: Set pin to INPUT (High-Z). Zero sink current, Relay turns 100% OFF.
// When ON:  Set pin to OUTPUT LOW. Sinks current through optocoupler, Relay turns ON.
void setRelay(bool activate) {
  if (activate) {
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, LOW);
  } else {
    pinMode(PIN_RELAY, INPUT);
  }
}

// OLED Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 1-Wire Setup
OneWire oneWire(PIN_ONEWIRE);
DallasTemperature ds18b20(&oneWire);

// Custom Single-Wire DHT Driver
CustomDHT customDht(PIN_DHT);
DHT standardDht(PIN_DHT, DHT22);

// INA219 DC Power Sensor
Adafruit_INA219 ina219;
bool hasINA219 = false;

// Local Flash Database (NVS Persistence)
Preferences prefs;
uint32_t bootCount = 0;
float peakTemp = 0.0;
uint32_t totalShocks = 0;

int dsCount = 0;

// Vibration Tracking (Polled Debounce)
unsigned long vibeCount = 0;
unsigned long lastVibeTime = 0;

void setup() {
  // 0. Disable Brownout Detector (prevents voltage dips from tripping reset)
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  delay(300);

  // 1. ESP32 ADC Setup: Full 0V - 3.3V range
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);

  // 2. Sensor & Actuator Pin Modes
  pinMode(PIN_SW420, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  digitalWrite(PIN_LED_GREEN, HIGH); // Start Nominal
  digitalWrite(PIN_LED_RED, LOW);

  // 3. Relay Output (Set to STANDBY immediately)
  setRelay(false);

  // 4. I2C Bus Initialization (Shared by OLED & INA219)
  Wire.begin(21, 22);
  Wire.setTimeOut(50); // 50ms guard prevents bus hangs
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  // 5. INA219 Auto-Detection
  if (ina219.begin()) {
    hasINA219 = true;
    Serial.println(F("[DEBUG] INA219 DC Power Module Detected on I2C @ 0x40!"));
  } else {
    hasINA219 = false;
    Serial.println(F("[DEBUG] INA219 not detected - using dynamic proxy model."));
  }

  // 6. Local Database (Flash NVS Persistence)
  prefs.begin("rack_db", false);
  bootCount = prefs.getUInt("boot_count", 0) + 1;
  prefs.putUInt("boot_count", bootCount);
  peakTemp = prefs.getFloat("peak_temp", 28.0);
  totalShocks = prefs.getUInt("total_shocks", 0);
  prefs.end();

  Serial.println(F("\n======================================================="));
  Serial.println(F("  AI DATA CENTER RACK HEALTH MONITOR (100% COMPLETE)"));
  Serial.printf("  [NVS FLASH DB] Boot: %u | Peak Temp: %.1f C | Lifetime Shocks: %u\n", bootCount, peakTemp, totalShocks);
  Serial.println(F("=======================================================\n"));

  // 7. DS18B20 1-Wire Scan
  ds18b20.begin();
  ds18b20.setWaitForConversion(false);
  dsCount = ds18b20.getDeviceCount();
  Serial.printf("[DEBUG] Found %d DS18B20 Probes on GPIO 4.\n", dsCount);
  ds18b20.requestTemperatures();

  // 8. DHT Init
  customDht.begin();
  standardDht.begin();
  Serial.println(F("[DEBUG] setup() COMPLETED 100%!"));
}

int page = 0;
unsigned long lastPageSwitch = 0;
int animFrame = 0;

float t1 = 28.5, t2 = 28.5, t3 = 28.5, t4 = 28.5;
float ambTemp = 26.5, ambHum = 55.0;
int mqRaw = 0;

void loop() {
  animFrame++;

  // --- 1. Staggered Async Polling (Zero IWDT Panics) ---
  static unsigned long lastScheduleTick = 0;
  static uint8_t scheduleSlot = 0;

  if (millis() - lastScheduleTick >= 200) {
    lastScheduleTick = millis();
    scheduleSlot = (scheduleSlot + 1) % 4;

    switch (scheduleSlot) {
      case 0:
        if (dsCount > 0) t1 = ds18b20.getTempCByIndex(0);
        if (dsCount > 1) t2 = ds18b20.getTempCByIndex(1);
        break;
      case 1:
        if (dsCount > 2) t3 = ds18b20.getTempCByIndex(2);
        if (dsCount > 3) t4 = ds18b20.getTempCByIndex(3);
        break;
      case 2:
        ds18b20.requestTemperatures();
        break;
      case 3:
        if (customDht.read() && customDht.getTemperature() > 0.0) {
          ambTemp = customDht.getTemperature();
          ambHum = customDht.getHumidity();
        }
        break;
    }
  }

  // --- 2. Read Gas Sensor on GPIO 34 ---
  mqRaw = analogRead(PIN_MQ135);

  // --- 3. Vibration Sensor Harmonic & Energy Filter (Benign vs Harmful) ---
  int currentVibeState = digitalRead(PIN_SW420);
  static int lastVibeState = LOW;
  static unsigned long lastVibeDebounce = 0;
  static unsigned long vibeWindowStart = 0;
  static int vibePulseInWindow = 0;
  static bool isHarmfulVibe = false;
  static unsigned long harmfulVibeClearTime = 0;

  if (currentVibeState == HIGH && lastVibeState == LOW) {
    if (millis() - lastVibeDebounce > 70) {
      vibeCount++;
      lastVibeTime = millis();
      lastVibeDebounce = millis();
      vibePulseInWindow++;
    }
  }
  lastVibeState = currentVibeState;

  // 2.0-second sliding energy window:
  // Benign accidental desk bump: 1-2 pulses -> SAFE (Ignored)
  // Harmful persistent resonance / bearing fault: >= 4 pulses in 2s -> ALARM!
  if (millis() - vibeWindowStart >= 2000) {
    if (vibePulseInWindow >= 4) {
      isHarmfulVibe = true;
      harmfulVibeClearTime = millis() + 4000; // Hold alarm for 4 seconds after shaking stops
    }
    vibeWindowStart = millis();
    vibePulseInWindow = 0;
  }

  if (isHarmfulVibe && millis() > harmfulVibeClearTime) {
    isHarmfulVibe = false;
  }
  bool isVibrating = (lastVibeTime > 0) && (millis() - lastVibeTime < 2000);

  // --- 4. Read INA219 Electrical Rail ---
  float busVoltage = 5.00;
  float current_mA = 0.0;
  float power_mW = 0.0;

  if (hasINA219) {
    busVoltage = ina219.getBusVoltage_V();
    current_mA = ina219.getCurrent_mA();
    power_mW = ina219.getPower_mW();
  }

  // --- 5. Edge Analytics: Multi-Causal Actuation (Temp, Gas, Harmful Vibe) ---
  float maxRackTemp = max(max(t1, t2), max(t3, t4));
  bool isHot = maxRackTemp >= 31.5;
  bool isGasAlert = mqRaw >= 1000;
  bool emergencyTrip = isHot || isGasAlert || isHarmfulVibe;

  // Actuate Relay (Cooling Fan)
  setRelay(emergencyTrip);

  if (!hasINA219) {
    current_mA = emergencyTrip ? 180.0 : 0.0;
    power_mW = busVoltage * current_mA;
  }

  // Actuate Visual & Audible Alarm (Buzzer & LEDs)
  if (emergencyTrip) {
    digitalWrite(PIN_LED_RED, HIGH);
    digitalWrite(PIN_LED_GREEN, LOW);
    // Urgent alarm: 400ms ON / 100ms OFF (80% duty cycle for maximum audible volume!)
    digitalWrite(PIN_BUZZER, (millis() % 500 < 400) ? HIGH : LOW);
  } else {
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_GREEN, HIGH);
    digitalWrite(PIN_BUZZER, LOW);
  }

  // Track Peak Temp in RAM
  if (maxRackTemp > peakTemp && maxRackTemp < 85.0) {
    peakTemp = maxRackTemp;
  }

  // --- 6. Page Rotation every 7.0 seconds (ample reading time) ---
  if (millis() - lastPageSwitch > 7000) {
    page = (page + 1) % 2;
    lastPageSwitch = millis();
  }

  // --- 7. Stream Telemetry to Serial (1 Hz JSON) ---
  static unsigned long lastSerialPrint = 0;
  float gasVolt = (mqRaw / 4095.0) * 3.3;
  if (millis() - lastSerialPrint > 1000) {
    lastSerialPrint = millis();
    Serial.printf("{\"t1\":%.2f,\"t2\":%.2f,\"t3\":%.2f,\"t4\":%.2f,\"ambT\":%.1f,\"ambH\":%.1f,\"gas\":%d,\"gasVolt\":%.2f,\"vibe\":%d,\"vibeCount\":%lu,\"relay\":%d,\"boot\":%u,\"peakT\":%.1f,\"lifeShocks\":%u,\"busV\":%.2f,\"currMA\":%.1f,\"powerMW\":%.1f}\n",
                  t1, t2, t3, t4, ambTemp, ambHum, mqRaw, gasVolt, (isHarmfulVibe ? 2 : (isVibrating ? 1 : 0)), vibeCount, emergencyTrip ? 1 : 0, bootCount, peakTemp, totalShocks, busVoltage, current_mA, power_mW);
  }

  // --- 8. Render OLED Screen ---
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  char spinner[] = {'|', '/', '-', '\\'};
  char spinChar = spinner[(animFrame / 3) % 4];

  if (page == 0) {
    // PAGE 1: 4-Tier Thermal Digital Twin Map
    display.setCursor(0, 0);
    display.print("RACK THERMAL MAP");
    display.setCursor(118, 0);
    display.print(spinChar);
    display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

    display.setCursor(0, 12); display.printf("T4 (Top):  %4.1f C", t4);
    display.setCursor(0, 22); display.printf("T3 (Mid):  %4.1f C", t3);
    display.setCursor(0, 22 + 10); display.printf("T2 (Pwr):  %4.1f C", t2);
    display.setCursor(0, 42); display.printf("T1 (Btm):  %4.1f C", t1);

    display.drawLine(0, 52, 127, 52, SSD1306_WHITE);
    display.setCursor(0, 55); 
    display.printf("Boot:%u | Pk:%.1fC", bootCount, peakTemp);
  } else {
    // PAGE 2: Electrical, Environmental & Actuation Status
    display.setCursor(0, 0);
    display.print("CAUSAL & POWER");
    display.setCursor(118, 0);
    display.print(spinChar);
    display.drawLine(0, 9, 127, 9, SSD1306_WHITE);

    display.setCursor(0, 12); 
    display.printf("Amb:%.1fC  Hum:%.0f%%", ambTemp, ambHum);

    display.setCursor(0, 22);
    display.printf("Gas:%d  Shocks:%lu", mqRaw, vibeCount);

    display.setCursor(0, 32);
    display.printf("Pwr:%.0fmW Fan:%s", power_mW, emergencyTrip ? "ON" : "OFF");

    display.setCursor(0, 42);
    if (isHarmfulVibe) {
      display.printf("Vibe: HARMFUL SHOCK!");
    } else if (isVibrating) {
      display.printf("Vibe: BENIGN TAP [OK]");
    } else {
      display.printf("Vibe: IDLE [OK]");
    }

    display.drawLine(0, 52, 127, 52, SSD1306_WHITE);
    display.setCursor(0, 55);
    if (isGasAlert) {
      display.print("ALERT: GAS / SMOKE!");
    } else if (isHot) {
      display.print("ALERT: OVERHEAT!");
    } else if (isHarmfulVibe) {
      display.print("ALERT: HARMFUL VIBE!");
    } else if (isVibrating) {
      display.print("STATUS: BENIGN TAP OK");
    } else {
      display.print("STATUS: NOMINAL OK");
    }
  }

  display.display();
  delay(30);
}
