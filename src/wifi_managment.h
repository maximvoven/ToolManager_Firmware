#ifndef WIFI_MANAGE
#define WIFI_MANAGE

#include <Arduino.h>
#include <Wire.h>
#include <PN532.h>
#include "Utils.h"
#include <iomanip>
#include <sstream>
#include <string>

bool initialize_wifi();
bool provisioning_session();
static bool check_wifi_configured();
bool check_wifi_connected();
bool brodcast_recover_me_message();
bool check_if_configured_wifi_available();
std::string hexStr(const uint8_t *data, int len);

#endif // UTILS_H