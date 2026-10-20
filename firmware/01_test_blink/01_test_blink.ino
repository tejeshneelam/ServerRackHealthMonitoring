// Stage 1: ESP32 Hardware & Serial Communication Health Check
// Board: DOIT ESP32 DEVKIT V1 (30-pin)
// Onboard Blue LED is mapped to GPIO 2

#define ONBOARD_LED 2

void setup() {
  // Initialize Serial Monitor at 115200 baud
  Serial.begin(115200);
  delay(1000);

  // Set the LED pin as output
  pinMode(ONBOARD_LED, OUTPUT);

  Serial.println("\n==================================================");
  Serial.println("   AI-POWERED DATA CENTER RACK HEALTH MONITOR     ");
  Serial.println("   Edge Node Initialized Successfully!            ");
  Serial.println("==================================================");
  Serial.print("Chip Model:        "); Serial.println(ESP.getChipModel());
  Serial.print("Chip Cores:        "); Serial.println(ESP.getChipCores());
  Serial.print("CPU Frequency:     "); Serial.print(ESP.getCpuFreqMHz()); Serial.println(" MHz");
  Serial.print("Available Heap:    "); Serial.print(ESP.getFreeHeap() / 1024); Serial.println(" KB");
  Serial.println("Starting Heartbeat Loop...\n");
}

int cycleCount = 0;

void loop() {
  cycleCount++;
  
  // Turn LED ON
  digitalWrite(ONBOARD_LED, HIGH);
  Serial.print("[HEARTBEAT #");
  Serial.print(cycleCount);
  Serial.print("] LED ON  - Edge System Normal. Free Memory: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  delay(1000);

  // Turn LED OFF
  digitalWrite(ONBOARD_LED, LOW);
  Serial.print("[HEARTBEAT #");
  Serial.print(cycleCount);
  Serial.println("] LED OFF - Edge System Normal.");
  delay(1000);
}
