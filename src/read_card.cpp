#include "read_card.h"

// uint8_t        _prevIDm[8];
unsigned int  attempt=0;


std::string hexStr(const uint8_t *data, int len)
{
     std::stringstream ss;
     ss << std::hex;

     for( int i(0) ; i < len; ++i )
         ss << std::setw(2) << std::setfill('0') << (int)data[i];

     return ss.str();
}

bool read_rdif_get_id(PN532 rfid, char* uid, byte* uid_len){
    printf("read_rdif_get_id()\r\n");
    bool success;
    byte uid_value[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }; 
    eCardType uid_type;
    success = rfid.ReadPassiveTargetID(&uid_value[0], uid_len, &uid_type);
    if(!success){
        Utils::Print("ERROR: Failed to Communicate with RFID Reader");
        return false;
    }
    strcpy(uid, (const char*) &uid_value);
    return true;
}
