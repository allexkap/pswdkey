#define RELEASE_TIMEOUT 400
#define LONG_TIMEOUT 400
#define ERROR_TIMEOUT 4000

#define GND A1
#define BTN A0
#define VCC 9
#define LED LED_BUILTIN

#include <Keyboard.h>
#include <EEPROM.h>

const size_t PASSWORDS_COUNT = 30;
String PASSWORDS[PASSWORDS_COUNT] = {};

void panic() {
  for (;;) {
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(500);
  }
}

void savePasswords() {
  size_t eeprom_addr = 0;
  for (size_t i = 0; i < PASSWORDS_COUNT; ++i) {
    for (size_t j = 0; !j || PASSWORDS[i][j - 1]; ++j) {
      EEPROM.update(eeprom_addr++, PASSWORDS[i][j]);
      if (eeprom_addr == EEPROM.length()) panic();
    }
  }
}

void loadPasswords() {
  size_t eeprom_addr = 0;
  for (size_t i = 0; i < PASSWORDS_COUNT; ++i) {
    char ch;
    while (ch = EEPROM.read(eeprom_addr++)) {
      if (eeprom_addr == EEPROM.length()) panic();
      PASSWORDS[i] += ch;
    }
  }
}

size_t readInput() {
  size_t code = 1;
  uint32_t ts_now = 0, ts_prev = 0;
  bool status = false;

  for (;;) {
    ts_now = millis();

    if (status != digitalRead(BTN)) {
      status = !status;
      if (status) {
        code <<= 1;
      } else if (ts_now - ts_prev > LONG_TIMEOUT) {
        ++code;
      }

      ts_prev = ts_now;
    } else if (!status && ts_now - ts_prev > RELEASE_TIMEOUT) {
      return code;
    }

    digitalWrite(LED, status && ts_now - ts_prev > LONG_TIMEOUT);
  }
}

void sendPassword(size_t num, uint32_t ms = 20) {
  String str;
  if (num >= PASSWORDS_COUNT || !(str = PASSWORDS[num]).length()) str = "Slot " + String(num);

  for (size_t i = 0; str[i]; ++i) {
    Keyboard.write(str[i]);
    delay(ms);
  }
}

void setup() {
  Serial.begin(115200);
  Keyboard.begin();

  pinMode(GND, OUTPUT);
  digitalWrite(VCC, LOW);
  pinMode(VCC, OUTPUT);
  digitalWrite(VCC, HIGH);
  pinMode(BTN, INPUT);

  loadPasswords();
}

void loop() {
  size_t code = readInput();
  if (code < 2) return;

  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);

  Serial.println(code);
  sendPassword(code - 2);
}
