/*
 * ====================================================================
 * TEST SKETCH 06: MQ GAS / SMOKE SENSOR ISOLATED DIAGNOSTIC
 * ====================================================================
 * 
 * Hardware Connections:
 *   MQ VCC  --> ESP32 5V / VIN (CRITICAL: Needs 5V for heater coil!)
 *   MQ GND  --> ESP32 GND
 *   MQ AO   --> ESP32 GPIO 34 (Analog Input)
 *   MQ DO   --> ESP32 GPIO 35 (Digital Threshold Input - Optional)
 * 
 * Note on MQ Sensors:
 *   - The metal mesh contains a micro-heating element. When running on 5V,
 *     the sensor will feel gently warm to the touch.
 *   - New modules need 20-30 seconds to burn off moisture on boot.
 * ====================================================================
 */

#define PIN_GAS_AO     34   // ADC1_CH6 (Input-only, ideal for analog sensors)
#define PIN_GAS_DO     35   // Digital threshold pin (optional)
#define PIN_BUILTIN_LED 2   // Built-in LED indicates high gas/smoke detection

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Set 11dB attenuation to read full 0.0V to 3.3V on ESP32 ADC
  analogSetAttenuation(ADC_11db);
  analogReadResolution(12);

  pinMode(PIN_GAS_AO, INPUT);
  pinMode(PIN_GAS_DO, INPUT);
  pinMode(PIN_BUILTIN_LED, OUTPUT);
  digitalWrite(PIN_BUILTIN_LED, LOW);

  Serial.println("\n==================================================");
  Serial.println("     MQ GAS / SMOKE SENSOR ISOLATED TEST TOOL     ");
  Serial.println("==================================================");
  Serial.println("Wiring:");
  Serial.println("  MQ VCC -> ESP32 5V / VIN  (MUST be 5V for heater!)");
  Serial.println("  MQ GND -> ESP32 GND");
  Serial.println("  MQ AO  -> ESP32 GPIO 34");
  Serial.println("  MQ DO  -> ESP32 GPIO 35 (optional)");
  Serial.println("--------------------------------------------------");
  Serial.println("Warming up internal heater coil (20 seconds)...");
  Serial.println("==================================================\n");

  // Warmup countdown
  for (int i = 20; i > 0; i--) {
    Serial.print("Heating coil stabilizing... ");
    Serial.print(i);
    Serial.print("s (Raw ADC: ");
    Serial.print(analogRead(PIN_GAS_AO));
    Serial.println(")");
    delay(1000);
  }

  Serial.println("\n>>> SENSOR READY! LIVE READINGS STARTING NOW <<<");
  Serial.println("Test using: Hand sanitizer vapor, perfume, or lighter gas.\n");
}

void loop() {
  // Take average of 10 samples for rock-solid stability
  long adcSum = 0;
  for (int i = 0; i < 10; i++) {
    adcSum += analogRead(PIN_GAS_AO);
    delay(10);
  }
  int rawADC = adcSum / 10;
  float voltage = (rawADC * 3.3) / 4095.0;
  int doState = digitalRead(PIN_GAS_DO);

  // Visual ASCII bar graph (scale 0 to 4095 across 25 bars)
  int bars = map(rawADC, 0, 4095, 0, 25);
  bars = constrain(bars, 0, 25);

  Serial.print("ADC: [");
  for (int i = 0; i < 25; i++) {
    if (i < bars) Serial.print("=");
    else Serial.print(" ");
  }
  Serial.print("] ");
  Serial.print(rawADC);
  Serial.print(" (");
  Serial.print(voltage, 2);
  Serial.print("V) | DO Pin: ");
  Serial.print(doState);

  // Threshold alert based on analog reading (Clean air baseline is ~200-400 ADC)
  if (rawADC > 1000) {
    digitalWrite(PIN_BUILTIN_LED, HIGH);
    Serial.println("  ===> [GAS / SMOKE SPIKE DETECTED!]");
  } else {
    digitalWrite(PIN_BUILTIN_LED, LOW);
    Serial.println("  [AIR CLEAN - NORMAL]");
  }

  delay(400); // Clean 2.5 Hz refresh
}
