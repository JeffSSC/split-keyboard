#include <BleKeyboard.h>

// Nome do dispositivo Bluetooth e Fabricante
BleKeyboard bleKeyboard("SplitKeyboard-C3", "JeffSC", 100);

// Pinos da Matriz da Metade Esquerda (4 linhas x 6 colunas)
const int ROW_PINS[4] = {0, 1, 2, 3};
const int COL_PINS[6] = {4, 5, 6, 7, 8, 10};

// Mapeamento das 21 Teclas (Matriz 4x6)
// Linha 0: ESC, Q, W, E, R, T
// Linha 1: TAB, A, S, D, F, G
// Linha 2: LShift, Z, X, C, V, B
// Linha 3: Thumbs (LAlt, LCtrl, Space)
uint8_t keymap[4][6] = {
  { KEY_ESC,        'q', 'w', 'e', 'r', 't' },
  { KEY_TAB,        'a', 's', 'd', 'f', 'g' },
  { KEY_LEFT_SHIFT, 'z', 'x', 'c', 'v', 'b' },
  { 0,              0,   0,   KEY_LEFT_ALT, KEY_LEFT_CTRL, ' ' }
};

bool keyState[4][6] = {false};

void setup() {
  // Inicializa o serviço Bluetooth HID
  bleKeyboard.begin();

  // Configura Linhas como saída (Estado Inicial HIGH)
  for (int r = 0; r < 4; r++) {
    pinMode(ROW_PINS[r], OUTPUT);
    digitalWrite(ROW_PINS[r], HIGH);
  }

  // Configura Colunas como entrada com Pull-Up interno
  for (int c = 0; c < 6; c++) {
    pinMode(COL_PINS[c], INPUT_PULLUP);
  }
}

void loop() {
  // A varredura só roda se o Bluetooth estiver conectado a um host
  if (bleKeyboard.isConnected()) {
    for (int r = 0; r < 4; r++) {
      digitalWrite(ROW_PINS[r], LOW);
      delayMicroseconds(10);

      for (int c = 0; c < 6; c++) {
        // Pula os 3 slots vazios da linha do polegar
        if (r == 3 && c < 3) continue;

        bool pressed = (digitalRead(COL_PINS[c]) == LOW);

        if (pressed != keyState[r][c]) {
          keyState[r][c] = pressed;
          uint8_t key = keymap[r][c];

          if (key != 0) {
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
  }
  delay(10); // Debounce de 10ms
}
