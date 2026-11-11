#include <BleKeyboard.h>

BleKeyboard bleKeyboard("SplitKeyboard-C3", "JeffSC", 100);

#define KEY_FN 0xFF

const int ROW_PINS[4] = {0, 1, 2, 3};
const int COL_PINS[6] = {4, 5, 6, 7, 8, 9};

bool layerFN = false;

// CAMADA 0 (Base) - Metade Esquerda
uint8_t keymap_left_base[4][6] = {
  { KEY_ESC,        'q', 'w', 'e', 'r', 't' },
  { KEY_TAB,        'a', 's', 'd', 'f', 'g' },
  { KEY_LEFT_SHIFT, 'z', 'x', 'c', 'v', 'b' },
  { 0,              0,   0,   KEY_LEFT_ALT, KEY_LEFT_CTRL, ' ' }
};

// CAMADA 1 (FN) - Metade Esquerda
uint8_t keymap_left_fn[4][6] = {
  { '`',            '1', '2', '3', '4', '5' },
  { KEY_TAB,        KEY_F1, KEY_F2, KEY_F3, KEY_F4, KEY_F5 },
  { KEY_LEFT_SHIFT, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10 },
  { 0,              0,   0,   KEY_LEFT_ALT, KEY_LEFT_CTRL, ' ' }
};

// CAMADA 0 (Base) - Metade Direita (Índices 0..23)
uint8_t keymap_right_base[24] = {
  KEY_BACKSPACE,   'p', 'o', 'i', 'u', 'y',
  KEY_RETURN,      ';', 'l', 'k', 'j', 'h',
  KEY_RIGHT_SHIFT, '/', '.', ',', 'm', 'n',
  0,               0,      0,             KEY_LEFT_GUI, KEY_FN, ' '
};

// CAMADA 1 (FN) - Metade Direita (Índices 0..23)
uint8_t keymap_right_fn[24] = {
  KEY_DELETE,     '[', ']', '-', '=', '6',
  KEY_RETURN,     '\\', KEY_UP_ARROW, ';', '9', '8',
  KEY_RIGHT_SHIFT, KEY_LEFT_ARROW, KEY_DOWN_ARROW, KEY_RIGHT_ARROW, '0', '7',
  0,              0,      0,             KEY_LEFT_GUI, KEY_FN, ' '
};

bool keyStateLeft[4][6] = {false};

// Key sent on press, so release always matches it even if the layer changed meanwhile
uint8_t sentKeyLeft[4][6] = {0};
uint8_t sentKeyRight[24] = {0};

void setup() {
  bleKeyboard.begin();
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
  // 1. Processa a Metade Esquerda
  for (int r = 0; r < 4; r++) {
    digitalWrite(ROW_PINS[r], LOW);
    delayMicroseconds(10);

    for (int c = 0; c < 6; c++) {
      if (r == 3 && c < 3) continue;

      bool pressed = (digitalRead(COL_PINS[c]) == LOW);

      if (pressed != keyStateLeft[r][c]) {
        keyStateLeft[r][c] = pressed;

        uint8_t key = pressed
          ? (layerFN ? keymap_left_fn[r][c] : keymap_left_base[r][c])
          : sentKeyLeft[r][c];
        sentKeyLeft[r][c] = pressed ? key : 0;

        if (key != 0 && bleKeyboard.isConnected()) {
          if (pressed) {
            bleKeyboard.press(key);
          } else {
            bleKeyboard.release(key);
          }
        }
      }
    }
    digitalWrite(ROW_PINS[r], HIGH);
  }

  // 2. Processa a Metade Direita (Serial TRRS a 9600 baud)
  while (Serial1.available() > 0) {
    uint8_t b = Serial1.read();
    bool pressed = (b & 0x80) != 0;
    uint8_t keyIndex = b & 0x7F;

    if (keyIndex < 24) {
      uint8_t baseKey = keymap_right_base[keyIndex];

      if (baseKey == KEY_FN) {
        layerFN = pressed;
      } else if (bleKeyboard.isConnected()) {
        uint8_t keyToSend = pressed
          ? (layerFN ? keymap_right_fn[keyIndex] : baseKey)
          : sentKeyRight[keyIndex];
        sentKeyRight[keyIndex] = pressed ? keyToSend : 0;

        if (keyToSend != 0) {
          if (pressed) {
            bleKeyboard.press(keyToSend);
          } else {
            bleKeyboard.release(keyToSend);
          }
        }
      }
    }
  }
  delay(10);
}
