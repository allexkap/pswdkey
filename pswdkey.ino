#define RELEASE_TIMEOUT 400
#define LONG_TIMEOUT 400

#define GND A1
#define IO A0
#define VCC 9

#include <Keyboard.h>

// eeprom stub
const size_t PASSWORDS_LEN = 3;
const char *PASSWORDS[PASSWORDS_LEN] = {
    "first slot",
    "second slot",
    "veryverryverylongpasswordtwiceveryverryverylongpasswordtwice",
};

uint8_t readCode() {
  uint8_t code = 1;
  uint32_t ts_now, ts_prev;
  uint8_t button_now, button_prev = false;

  while (code == 1 || button_now || ts_now - ts_prev < RELEASE_TIMEOUT) {
    button_now = digitalRead(IO);
    if (!button_now && code == 1)
      return 0;

    ts_now = millis();
    if (button_now != button_prev) {
      if (button_now)
        code <<= 1;
      else if (ts_now - ts_prev > LONG_TIMEOUT)
        ++code;
      button_prev = button_now;
      ts_prev = ts_now;
    }

    digitalWrite(LED_BUILTIN, button_now && ts_now - ts_prev > LONG_TIMEOUT);
  }

  digitalWrite(LED_BUILTIN, HIGH);
  delay(100);
  digitalWrite(LED_BUILTIN, LOW);
  return code;
}

void passwordWrite(uint8_t num, uint32_t dt = 20) {
  if (num >= PASSWORDS_LEN)
    return;

  char *str = PASSWORDS[num];
  for (int i = 0; str[i]; ++i) {
    Keyboard.write(str[i]);
    delay(dt);
  }
}

void setup() {
  Serial.begin(115200);
  Keyboard.begin();

  pinMode(GND, OUTPUT);
  digitalWrite(VCC, LOW);
  pinMode(VCC, OUTPUT);
  digitalWrite(VCC, HIGH);
  pinMode(IO, INPUT);

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  uint8_t code = readCode();
  if (code) {
    Serial.println(code);
    passwordWrite(code - 2);
  }
}
