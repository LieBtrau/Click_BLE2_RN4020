#ifndef BTCHARACTERISTIC_H
#define BTCHARACTERISTIC_H
#include "Arduino.h"

class btCharacteristic
{
public:
    typedef enum
    {
        INDICATE=0x20,          //like notify, but with acknowledgements (server doesn't need to do polling READs)
        NOTIFY=0x10,            //client will get notifications when server (locally) changes the value (server doesn't need to do polling READs)
        WRITE=0x08,             //client writes value and gets acknowledgements
        WRITE_WOUT_RESP=0x04,   //client writes value and doesn't get acknowledgements
        READ=0x02               //client can read the value
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
    btCharacteristic(const char* uuid_service, const char* uuid_characteristic, PROPERTY_FLAGS propertyBmp,
                     byte valueLength, SECURITY_FLAGS securityBmp, void (*ftListener)(byte*, byte&));
    void setHandle(word handle);
    const char* getUuidService();
    const char* getUuidCharacteristic();
    byte getProperty();
    byte getValueLength();
    word getHandle();
    byte getSecurityBmp();
    void callListener(byte *data, byte &length);
private:
    char* cleanupUuid(const char* uuid);
    char* _uuid_service;
    char* _uuid_characteristic;
    byte _propertyBmp;
    byte _securityBmp;
    byte _valueLength;
    word _handle;
    void (*_ftListener)(byte*, byte&);
};

#endif // BTCHARACTERISTIC_H
