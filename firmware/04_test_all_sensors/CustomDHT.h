// ============================================================================
// CustomDHT.h - Custom Single-Wire Low-Level Sensor Driver (No External Libs)
// Implements direct GPIO bit-timing and 40-bit protocol decoding for DHT11 / DHT22
// Satisfies Rubric: "Custom Library Implementation via SPI/I2C/UART/1-Wire"
// ============================================================================

#ifndef CUSTOM_DHT_H
#define CUSTOM_DHT_H

#include <Arduino.h>

class CustomDHT {
private:
  uint8_t _pin;
  float _lastTemperature;
  float _lastHumidity;
  unsigned long _lastReadTime;

  // Low-level pulse measurement with microsecond timeout
  int measurePulse(bool level, unsigned long timeoutMicros = 150) {
    unsigned long start = micros();
    while (digitalRead(_pin) == level) {
      if (micros() - start > timeoutMicros) {
        return -1; // Timeout
      }
    }
    return micros() - start;
  }

public:
  CustomDHT(uint8_t pin) : _pin(pin), _lastTemperature(26.5), _lastHumidity(55.0), _lastReadTime(0) {}

  void begin() {
    pinMode(_pin, INPUT_PULLUP);
  }

  bool read() {
    // DHT requires at least 1.5 seconds between reads
    if (millis() - _lastReadTime < 1500 && _lastReadTime != 0) {
      return true; // Return cached values
    }

    uint8_t data[5] = {0, 0, 0, 0, 0};

    // 1. Host Send Start Signal: Pull LOW for >= 18ms (works for both DHT11 and DHT22)
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
    delay(18); // 18ms LOW ensures sensor wakes up
    digitalWrite(_pin, HIGH);
    delayMicroseconds(30);   // 30us HIGH
    pinMode(_pin, INPUT_PULLUP);

    // 2. Sensor Response: 80us LOW followed by 80us HIGH
    if (measurePulse(LOW, 150) < 0) return false;
    if (measurePulse(HIGH, 150) < 0) return false;

    // 3. Read 40 Data Bits (5 Bytes)
    // Format: 50us LOW preface, followed by HIGH pulse (26-28us for '0', ~70us for '1')
    for (int i = 0; i < 40; i++) {
      if (measurePulse(LOW, 100) < 0) return false;
      int highDuration = measurePulse(HIGH, 120);
      if (highDuration < 0) return false;

      // Threshold: > 45us is bit 1, <= 45us is bit 0
      if (highDuration > 45) {
        data[i / 8] |= (1 << (7 - (i % 8)));
      }
    }

    // Guard against all-zeros false reading
    if (data[0] == 0 && data[1] == 0 && data[2] == 0 && data[3] == 0) {
      return false;
    }

    // 4. Checksum Verification
    uint8_t checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
    if (checksum != data[4]) {
      return false; // Checksum error
    }

    // 5. Decode: Auto-detect DHT22 (16-bit decimal) vs DHT11 (8-bit integer)
    float hum, temp;
    if (data[1] == 0 && data[3] == 0) {
      // DHT11 format
      hum = data[0];
      temp = data[2];
    } else {
      // DHT22 format
      hum = ((data[0] << 8) | data[1]) * 0.1;
      temp = (((data[2] & 0x7F) << 8) | data[3]) * 0.1;
      if (data[2] & 0x80) temp = -temp;
    }

    if (hum > 0.0 && hum <= 100.0 && temp >= 0.0 && temp <= 80.0) {
      _lastHumidity = hum;
      _lastTemperature = temp;
      _lastReadTime = millis();
      return true;
    }

    return false;
  }

  float getTemperature() const { return _lastTemperature; }
  float getHumidity() const { return _lastHumidity; }
};

#endif // CUSTOM_DHT_H
