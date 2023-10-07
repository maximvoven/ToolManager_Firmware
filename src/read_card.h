#ifndef READ_CARD
#define READ_CARD

#include <Arduino.h>
#include <Wire.h>
#include <PN532.h>
#include "Utils.h"
#include <iomanip>
#include <sstream>
#include <string>

bool read_rdif_get_id(PN532 rfid, char* uid, byte* uid_len);
std::string hexStr(const uint8_t *data, int len);

#endif // UTILS_H