#include <Arduino.h>
#include <Wire.h>
#include <PN532.h>
#include "Utils.h"

uint8_t        _prevIDm[8];
unsigned long  _prevTime;

PN532 RF_Reader;

void setup() {
  Serial.begin(115200);
  Serial.println("BOOT STRAP");

  RF_Reader.SetDebugLevel(10);
  RF_Reader.begin(); //Initiates I2C using I2CClass to default pins 22 and 21
  byte type=0, v_hi=0, v_lo=0, pflag=0;
  uint32_t versiondata = RF_Reader.GetFirmwareVersion(&type, &v_hi, &v_lo, &pflag);
  Utils::Print("Identified PN532 as type: ");
  Utils::PrintHex8(type, LF);
  Utils::Print("Firmware Version: ");
  Utils::PrintHex8(v_hi, ", ");
  Utils::PrintHex8(v_lo, LF);
  Utils::Print("Supporting Following Modes: ");
  Utils::PrintHex8(pflag,LF);

  // Set the max number of retry attempts to read from a card
  // This prevents us from waiting forever for a card, which is
  // the default behaviour of the PN532.
  RF_Reader.SetPassiveActivationRetries();
  RF_Reader.SamConfig();

  memset(_prevIDm, 0, 8);
}

void loop() {
  // put your main code here, to run repeatedly:
  boolean success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  eCardType type=CARD_Unknown;
  
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = RF_Reader.ReadPassiveTargetID(&uid[0], &uidLength, &type);
  
  if (uidLength != 0) {
    Serial.println("Found a type A card!");
    Serial.print("UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i=0; i < uidLength; i++) 
    {
      Serial.print(" 0x");Serial.print(uid[i], HEX); 
    }
    Serial.print("UID Type: ");
    Utils::PrintDec(type,LF);
    Serial.println("");
    // Wait 1 second before continuing
    delay(1000);
  }
  else
  {
    // PN532 probably timed out waiting for a card
    Serial.println("Timed out waiting for type A card");
  }
   delay(1000);
}
