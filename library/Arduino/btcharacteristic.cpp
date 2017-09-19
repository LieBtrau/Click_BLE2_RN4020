#include "btcharacteristic.h"

btCharacteristic::btCharacteristic(const char* uuid_service,
                                   const char* uuid_characteristic,
                                   PROPERTY_FLAGS propertyBmp,
                                   byte valueLength,
                                   SECURITY_FLAGS securityBmp):
    btCharacteristic(uuid_service, uuid_characteristic, propertyBmp, valueLength, securityBmp, 0)
{}

btCharacteristic::btCharacteristic(const char* uuid_service,
                                   const char* uuid_characteristic,
                                   PROPERTY_FLAGS propertyBmp,
                                   byte valueLength,
                                   SECURITY_FLAGS securityBmp,
                                   void (*ftListener)(byte*, byte&)):
    _propertyBmp(propertyBmp),
    _valueLength(valueLength),
    _securityBmp(securityBmp)
{
    cleanupUuid(uuid_service, _uuid_service);
    cleanupUuid(uuid_characteristic, _uuid_characteristic);
    _ftListener=ftListener;
}


const char *btCharacteristic::getUuidService()
{
    return _uuid_service;
}

const char *btCharacteristic::getUuidCharacteristic()
{
    return _uuid_characteristic;
}

byte btCharacteristic::getProperty()
{
    return _propertyBmp;
}

byte btCharacteristic::getValueLength()
{
    return _valueLength;
}

word btCharacteristic::getHandle()
{
    return _handle;
}

byte btCharacteristic::getSecurityBmp()
{
    return _securityBmp;
}

void btCharacteristic::setHandle(word handle)
{
    _handle=handle;
}


void btCharacteristic::callListener(byte *data, byte& length)
{
    if(!_ftListener)
    {
        return;
    }
    _ftListener(data, length);
}

void btCharacteristic::cleanupUuid(const char* uuid, char* &out)
{
    char* pch;
    char temp[37];
    if(strlen(uuid)<1 || strlen(uuid)>36)
    {
        return;
    }
    //Remove the '-' from the UUIDs
    strcpy(temp, uuid);
    do{
        pch=strchr(temp,'-');
        if(pch)
        {
            memmove(pch,pch+1,strlen(temp)-(pch-temp));
        }
    }while(pch);
    //Set to uppercase
    pch=temp;
    do
    {
        *pch=toupper(*pch);
        pch++;
    }while(*pch);
    out=(char*)malloc(strlen(temp)+1);
    strcpy(out, temp);
}
