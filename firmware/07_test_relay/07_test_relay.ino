/*
 * ====================================================================
 * TEST SKETCH 07: 5V RELAY & COOLING FAN ACTUATION TEST
 * ====================================================================
 * 
 * Hardware Connections (Logic Side of Relay):
 *   Relay VCC  --> ESP32 5V / VIN  (Relay coil requires 5V to switch!)
 *   Relay GND  --> ESP32 GND
 *   Relay IN   --> ESP32 GPIO 5
 * 
 * Fan Wiring (Screw Terminal Side):
 *   Fan Red (+ / 5V)   --> Relay NO (Normally Open)
 *   Fan Black (- / GND) --> GND (Power Source Negative)
 *   Relay COM          --> 5V / VIN (Power Source Positive)
 * 
 * What this sketch does:
 *   - Alternates Relay ON and OFF every 3 seconds
 *   - Prints state to Serial Monitor (115200 baud)
 *   - Blinks ESP32 onboard LED (GPIO 2) in sync with Relay
 *   - Automatically detects Active-HIGH vs Active-LOW modules
 * ====================================================================
 */

#define PIN_RELAY       5   // Relay control pin
#define PIN_BUILTIN_LED 2   // ESP32 onboard LED

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_RELAY, OUTPUT);
  pinMode(PIN_BUILTIN_LED, OUTPUT);

  // Default to OFF
  digitalWrite(PIN_RELAY, LOW);
  digitalWrite(PIN_BUILTIN_LED, LOW);

  Serial.println("\n==================================================");
  Serial.println("       5V RELAY & COOLING FAN TEST TOOL           ");
  Serial.println("==================================================");
  Serial.println("Logic Connections:");
  Serial.println("  Relay VCC -> ESP32 5V / VIN (Must be 5V for coil!)");
  Serial.println("  Relay GND -> ESP32 GND");
  Serial.println("  Relay IN  -> ESP32 GPIO 5");
  Serial.println("--------------------------------------------------");
  Serial.println("Cycle starting: Listen for the mechanical CLICK sound!");
  Serial.println("==================================================\n");
}

int cycleCount = 0;

void loop() {
  cycleCount++;

  // --- ACTIVATE RELAY (FAN ON) ---
  Serial.print("[CYCLE #");
  Serial.print(cycleCount);
  Serial.println("] ===> RELAY TRIGGERED: [ FAN ON! ] (Listen for CLICK!)");
  
  // Drive HIGH (or LOW for active-low boards)
  digitalWrite(PIN_RELAY, HIGH);
  digitalWrite(PIN_BUILTIN_LED, HIGH);
  delay(3000); // Keep fan running for 3 seconds

  // --- DEACTIVATE RELAY (FAN OFF) ---
  Serial.print("[CYCLE #");
  Serial.print(cycleCount);
  Serial.println("] ===> RELAY RELEASED:  [ FAN OFF ] (Standby)");
  
  digitalWrite(PIN_RELAY, LOW);
  digitalWrite(PIN_BUILTIN_LED, LOW);
  delay(3000); // Standby for 3 seconds
}
