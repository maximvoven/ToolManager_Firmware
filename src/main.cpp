#include <Arduino.h>
#include <Wire.h>
#include <PN532.h>
#include "Utils.h"
#include "read_card.h"
#include "wifi_managment.h"
#include <iomanip>
#include <sstream>
#include <string>
#include "main.h"

PN532 RF_Reader;
struct timeval tv_now;

// "System Timer"
int64_t time_us = 0;

// Red Light
int64_t event_time_1 = 0; 
// Green Light
int64_t event_time_2 = 0; 
// Blue Light
int64_t event_time_3 = 0; 
// Buzzer
int64_t event_time_4 = 0; 
// Event Timer
int64_t event_time_5 = 0; 


/*
Acceptable States
0: Virgin State (No Wifi Profile Installed, No ID Assigned)
1: Provisioned Waiting for Wifi Connection
2: Provisioned Waiting for Server Connection
3: Connected Waiting for ID
4: ID Accepted: Tool State Set to Desired State
5: Lockout (Waiting For Special ID) - Service or Web Design
*/
int state = 3;

/*
0: Flashing White 0.1s evrey 5 seconds (System Up and Operation) 
1: Solid White - Lockout/Special Mode
2: Solid Red White - ON No Wifi
3: Solid Red Green White - ON Wifi, No Connection to Master Server
4: Reg, Green, Blue in Sequence - Virgin
5: Authorized
6: Not Authorized
*/
int led_indicator_state = 4;


int red = 0;
int green = 0;
int blue = 0;
int buzzer = 0;
int relay = 0;

//0x8 0xE1 0x92 0x1B 
char* authorized_uid = new char[64];


void setup() {
  Serial.begin(115200);
  Serial.println("BOOT STRAP");

  setenv("TZ", "GMT+0",1);
  gettimeofday(&tv_now, NULL);
  time_us  = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;

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

  pinMode(RED_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN,1);
  pinMode(GREEN_LED_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN,1);
  pinMode(BLUE_LED_PIN, OUTPUT);
  digitalWrite(BLUE_LED_PIN, 1);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN,1);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN,1);

  pinMode(9, INPUT);
  pinMode(10, INPUT);

  //0x8 0xE1 0x92 0x1B
  authorized_uid[0] = 0x01;
  authorized_uid[1] = 0x23;
  authorized_uid[2] = 0x45;
  authorized_uid[3] = 0x67;


  // memset(_prevIDm, 0, 8);
}

void loop() {
  gettimeofday(&tv_now, NULL);
  time_us  = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
  // put your main code here, to run repeatedly:
  // 0: Virgin State (No Wifi Profile Installed, No ID Assigned)
  // 1: Provisioned Waiting for Wifi Connection
  // 2: Provisioned Waiting for Server Connection
  // 3: Connected Waiting for ID
  // 4: ID Accepted: Tool State Set to Desired State
  // 5: Lockout (Waiting For Special ID) - Service or Web Design
  printf("Main System Loop\n\r");

  if(state==0){
    printf("System Initialized: Waiting for Provisioning\n\r");

  }

  if(state==1){
    printf("Connecting to Wifi\n\r");
  }

  if(state==2){
    printf("Connecting to Server\n\r");
  }

  if (state==3){
    printf("Waiting for Authentication \n\r");
    led_indicator_state = 6;
    boolean success;
    char* uid = new char[64];
    byte uid_len;
    success = read_rdif_get_id(RF_Reader, uid, &uid_len);
    Serial.print("UID Value: ");
    for (uint8_t i=0; i < uid_len; i++) 
      {
        Serial.print(" 0x");Serial.print(uid[i], HEX); 
      }
    Serial.print("\r\n");
    if(success 
    && authorized_uid[0]==uid[0]
    && authorized_uid[1]==uid[1]
    && authorized_uid[2]==uid[2]
    && authorized_uid[3]==uid[3]){
      state = 4;
    }
  }

  if(state==4){
    printf("Access Authorized\n\r");
    static int failed_measurments = 0;
    led_indicator_state = 5;
    if (time_us > event_time_5){
        boolean success;
        char* uid = new char[64];
        byte uid_len;
        failed_measurments++;
        event_time_5 = time_us + 1000000L;
        success = read_rdif_get_id(RF_Reader, uid, &uid_len);
        if(success){
          Serial.print("UID Value: 0x");
          for (uint8_t i=0; i < uid_len; i++) 
          {
            Serial.print(" ");Serial.print(uid[i], HEX); 
          }
          Serial.print("\r\n");
          if(success 
          && authorized_uid[0]==uid[0]
          && authorized_uid[1]==uid[1]
          && authorized_uid[2]==uid[2]
          && authorized_uid[3]==uid[3]){
            state = 4;
            failed_measurments = 0;
          }
        }
    }
    if(failed_measurments>30){
      state = 3;
    }

  }
  if(state==5){
    printf("Lockout Mode\n\r");
  }

  // Led Indicator Blinking Code
  if(led_indicator_state==5){
    green = 1;
    red = 0;
    blue = 0;
  }
  if(led_indicator_state==6){
    green = 0;
    red = 0;
    blue = 1;
  }
  digitalWrite(GREEN_LED_PIN, green);
  digitalWrite(RED_LED_PIN, red);
  digitalWrite(BLUE_LED_PIN, blue);
  digitalWrite(RELAY_PIN, relay);
  digitalWrite(BUZZER_PIN, buzzer);
}
