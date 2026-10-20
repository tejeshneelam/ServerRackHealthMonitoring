// Quick Diagnostic Test for Green LED (26), Red LED (25), and Buzzer (27)
#define PIN_BUZZER    27
#define PIN_LED_GREEN 26
#define PIN_LED_RED   25

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  Serial.println("\n--- INDICATOR HARDWARE DIAGNOSTIC TEST ---");
}

void loop() {
  // 1. Test Green LED
  Serial.println(">> Testing GREEN LED (GPIO 26) - SHOULD BE ON");
  digitalWrite(PIN_LED_GREEN, HIGH);
  digitalWrite(PIN_LED_RED, LOW);
  digitalWrite(PIN_BUZZER, LOW);
  delay(1000);

  // 2. Test Red LED
  Serial.println(">> Testing RED LED (GPIO 25) - SHOULD BE ON");
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_RED, HIGH);
  digitalWrite(PIN_BUZZER, LOW);
  delay(1000);

  // 3. Test Buzzer Beep
  Serial.println(">> Testing BUZZER (GPIO 27) - SHOULD BEEP");
  digitalWrite(PIN_LED_RED, HIGH);
  digitalWrite(PIN_BUZZER, HIGH);
  delay(300);
  digitalWrite(PIN_BUZZER, LOW);
  delay(700);
}
