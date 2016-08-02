#ifndef BTCHARACTERISTIC_H
#define BTCHARACTERISTIC_H
#include "Arduino.h"

class btCharacteristic
{
public:
    typedef enum
    {
        INDICATE=0x20,
        NOTIFY=0x10,
        WRITE=0x08,
        WRITE_WOUT_RESP=0x04,
        READ=0x02
    }PROPERTY_FLAGS;
    btCharacteristic(const char* uuid_service, const char* uuid_characteristic, byte flag, byte length);
    void setListener(void (*ftListener)(char*));
    void setHandle(word handle);
    void getUuidService(char* buf);
    void getUuidCharacteristic(char* buf);
    byte getProperty();
    byte getValueLength();
    word getHandle();
    void callListener(char*data);
private:
    char* _uuid_service;
    char* _uuid_characteristic;
    byte _property;
    byte _valueLength;
    word _handle;
    void (*_ftListener)(char*);
};

#endif // BTCHARACTERISTIC_H
