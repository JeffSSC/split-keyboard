#include <Arduino.h>

// Physical Matrix Pinout for Right Half
const int ROW_PINS[4] = {0, 1, 2, 3};
const int COL_PINS[6] = {4, 5, 6, 7, 8, 9};

// 4x6 matrix state tracking
bool keyStateRight[4][6] = {false};

void setup() {
  // Serial1 TRRS Link: RX=GPIO20, TX=GPIO21
  Serial1.begin(9600, SERIAL_8N1, 20, 21);

  for (int r = 0; r < 4; r++) {
    pinMode(ROW_PINS[r], OUTPUT);
    digitalWrite(ROW_PINS[r], HIGH);
  }

  for (int c = 0; c < 6; c++) {
    pinMode(COL_PINS[c], INPUT_PULLUP);
  }
}

void loop() {
  for (int r = 0; r < 4; r++) {
    digitalWrite(ROW_PINS[r], LOW);
    delayMicroseconds(10);

    for (int c = 0; c < 6; c++) {
      bool pressed = (digitalRead(COL_PINS[c]) == LOW);

      if (pressed != keyStateRight[r][c]) {
        keyStateRight[r][c] = pressed;

        // Protocol: Bit 7 = State (1=Pressed, 0=Released) | Bits 0-6 = Matrix Index (0..23)
        uint8_t index = (r * 6) + c;
        uint8_t packet = (pressed ? 0x80 : 0x00) | (index & 0x7F);

        Serial1.write(packet);
        Serial1.flush();
      }
    }
    digitalWrite(ROW_PINS[r], HIGH);
  }
  delay(10);
}
