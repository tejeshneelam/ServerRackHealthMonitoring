/*
 * ====================================================================
 * TEST SKETCH 05: SW-420 VIBRATION SENSOR ISOLATED DIAGNOSTIC
 * ====================================================================
 * 
 * Hardware Connections:
 *   SW-420 VCC  --> ESP32 3V3 (Recommended for clean stable logic)
 *   SW-420 GND  --> ESP32 GND
 *   SW-420 DO   --> ESP32 GPIO 19
 * 
 * Features:
 *   1. Hardware Interrupt + Debounce for 100% accurate tap counting
 *   2. ESP32 Onboard Blue LED (GPIO 2) blinks on every tap
 *   3. Real-time diagnostic messages to guide potentiometer tuning
 * ====================================================================
 */

#define PIN_VIBE_DO    19   // Vibration sensor DO pin
#define PIN_BUILTIN_LED 2   // ESP32 onboard blue LED for visual verification

volatile unsigned long tapCount = 0;
volatile unsigned long lastTapTime = 0;
volatile unsigned long lastEdgeTime = 0;
volatile bool newTapDetected = false;

// Hardware interrupt with High-Frequency Oscillation / Partial-Glow Suppression
void IRAM_ATTR onVibrationChange() {
  unsigned long now = millis();
  unsigned long deltaEdge = now - lastEdgeTime;
  lastEdgeTime = now;

  // 1. REJECT COMPARATOR OSCILLATION ("Partial Glow" state):
  // When the LM393 enters its linear region, edges arrive at >10 kHz (<1ms apart).
  // Any edge arriving within 25ms of the previous edge is oscillation chatter or ringing!
  if (deltaEdge < 25) {
    return; // Completely suppress high-frequency oscillation!
  }

  // 2. TRUE PHYSICAL TAP:
  // Must be preceded by quiet resting state (deltaEdge >= 25ms)
  // and spaced at least 150ms from the previous registered tap.
  if (now - lastTapTime > 150) {
    tapCount++;
    lastTapTime = now;
    newTapDetected = true;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_VIBE_DO, INPUT);
  pinMode(PIN_BUILTIN_LED, OUTPUT);
  digitalWrite(PIN_BUILTIN_LED, LOW);

  // Attach interrupt to catch ANY transition (RISING or FALLING)
  attachInterrupt(digitalPinToInterrupt(PIN_VIBE_DO), onVibrationChange, CHANGE);

  Serial.println("\n==================================================");
  Serial.println("   SW-420 VIBRATION SENSOR ISOLATED TEST TOOL    ");
  Serial.println("==================================================");
  Serial.println("Wiring:");
  Serial.println("  SW-420 VCC -> ESP32 3V3");
  Serial.println("  SW-420 GND -> ESP32 GND");
  Serial.println("  SW-420 DO  -> ESP32 GPIO 19");
  Serial.println("--------------------------------------------------");
  Serial.println("Tuning Guide:");
  Serial.println("1. If DO LED is continuously ON: Turn blue pot CCW");
  Serial.println("   until DO LED turns OFF.");
  Serial.println("2. Tap the blue cylinder: DO LED should blink once,");
  Serial.println("   and the ESP32 blue LED will flash!");
  Serial.println("==================================================\n");
}

unsigned long lastPrintTime = 0;
int stableIdleCounter = 0;

void loop() {
  int rawState = digitalRead(PIN_VIBE_DO);

  // If a tap was registered by interrupt, flash onboard LED
  if (newTapDetected) {
    newTapDetected = false;
    digitalWrite(PIN_BUILTIN_LED, HIGH);
    Serial.print(">>> [TAP DETECTED!] Total Count: ");
    Serial.print(tapCount);
    Serial.print(" | DO Pin: ");
    Serial.println(rawState);
    delay(100); // Visual flash persistence
    digitalWrite(PIN_BUILTIN_LED, LOW);
  }

  // Periodic status display every 1 second
  if (millis() - lastPrintTime >= 1000) {
    lastPrintTime = millis();

    Serial.print("[STATUS] DO Pin = ");
    Serial.print(rawState);
    Serial.print(" | Tap Count = ");
    Serial.print(tapCount);

    if (rawState == HIGH) {
      Serial.println(" | Warning: Pin is HIGH (DO LED active). If not tapping, rotate pot counter-clockwise!");
    } else {
      Serial.println(" | Sensor is IDLE & READY. Tap cylinder to test!");
    }
  }
}
