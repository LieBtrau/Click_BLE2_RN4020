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
    typedef enum
    {
        NOTHING=0x00,
        ENCR_R=0x01,
        AUTH_R=0x02,
        ENCR_W=0x10,
        AUTH_W=0x20
    }SECURITY_FLAGS;
    btCharacteristic(const char* uuid_service, const char* uuid_characteristic, PROPERTY_FLAGS propertyBmp,
                     byte valueLength, SECURITY_FLAGS securityBmp);
    void setListener(void (*ftListener)(char*, byte&));
    void setHandle(word handle);
    const char* getUuidService();
    const char* getUuidCharacteristic();
    byte getProperty();
    byte getValueLength();
    word getHandle();
    byte getSecurityBmp();
    void callListener(char*data, byte &length);
private:
    char* cleanupUuid(const char* uuid);
    char* _uuid_service;
    char* _uuid_characteristic;
    byte _propertyBmp;
    byte _securityBmp;
    byte _valueLength;
    word _handle;
    void (*_ftListener)(char*, byte&);
};

#endif // BTCHARACTERISTIC_H
