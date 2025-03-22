#define RESET_EEPROM 0

#include <EEPROM.h>

constexpr size_t EEPROM_SIZE = 1024; // DO NOT CHANGE
constexpr size_t MAX_BLOCK_CYCLES = 14;

struct BlockHeader {
  uint16_t next : 10; // 0 - 1023
  uint16_t len : 10;  // 0 - 1023
  uint8_t number : 4; // 0 - 15
  uint8_t cycles;     // 0 - 255
} __attribute__((packed));
static_assert(sizeof(BlockHeader) == 4);

String passwords[PASSWORDS_COUNT] = {};

void dump_eeprom() {
  for (size_t i = 0; i < EEPROM_SIZE; i++) {
    uint8_t value = EEPROM.read(i);
    if (i % 32 == 0)
      Serial.printf("\n0x%04X: ", i);
    Serial.printf("%02X ", value);
  }
  Serial.println();
}

uint8_t get_initial_offset() {
  uint8_t initial_offset = EEPROM.read(0);

  if (RESET_EEPROM || !initial_offset) {
    initial_offset = 1;
    EEPROM.update(0, initial_offset);
    EEPROM.put(initial_offset, (BlockHeader){});
  }

  return initial_offset;
}

void load_passwords(uint8_t pos) {
  BlockHeader header;

  do {
    EEPROM.get(pos, header);
    Serial.printf("BlockHeader at %d {\n .next = %d\n .len = %d\n .number = %d\n .cycles = %d\n}\n",
                  pos, header.next, header.len, header.number, header.cycles);

    if (header.number >= PASSWORDS_COUNT) {
      Serial.printf("header.number (%d) >= PASSWORDS_COUNT (%d)", header.number, PASSWORDS_COUNT);
      continue;
    }

    if (!header.len) {
      Serial.printf("header.len == 0");
      continue;
    }

    String &str = passwords[header.number];
    str.reserve(header.len);
    for (size_t i = 0; i < header.len; ++i) {
      str += (char)EEPROM.read(pos + sizeof(BlockHeader) + i);
    }

  } while (pos = header.next);
}

int update_password(uint8_t pos, uint8_t number, String new_password) {
  size_t needed_len = new_password.length();

  if (passwords[number].length()) {
    Serial.printf("Not supported yet\n");
    return -1;
  }

  BlockHeader header;
  do {
    EEPROM.get(pos, header);

    if (header.len || header.next - (pos + sizeof(BlockHeader)) < needed_len) {
      continue;
    }
    uint16_t next_pos = pos + sizeof(BlockHeader) + needed_len;
    header.cycles += 1;
    EEPROM.put(next_pos, header);

    header.next = next_pos;
    header.len = needed_len;
    header.number = number;
    EEPROM.put(pos, header);
    for (size_t i = 0; i < header.len; ++i) {
      EEPROM.update(pos + sizeof(BlockHeader) + i, new_password[i]);
    }

    passwords[number] = new_password;

    return 0;

  } while (pos = header.next);

  Serial.printf("No block found\n");
  return -2;
}
