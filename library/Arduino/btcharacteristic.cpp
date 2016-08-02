#include "btcharacteristic.h"

btCharacteristic::btCharacteristic(const char *uuid_service, const char *uuid_characteristic, byte flag, byte length):
    _property(flag),
    _valueLength(length),
    _ftListener(0)
{
    _uuid_service=(char*)malloc(strlen(uuid_service)+1);
    if(!_uuid_service)
    {
        return;
    }
    strcpy(_uuid_service, uuid_service);

    _uuid_characteristic=(char*)malloc(strlen(uuid_characteristic)+1);
    if(!_uuid_characteristic)
    {
        return;
    }
    strcpy(_uuid_characteristic, uuid_characteristic);
}

void btCharacteristic::getUuidService(char *buf)
{
    if(!buf)
    {
        return;
    }
    strcpy(buf, _uuid_service);
}

void btCharacteristic::getUuidCharacteristic(char *buf)
{
    if(!buf)
    {
        return;
    }
    strcpy(buf, _uuid_characteristic);
}

byte btCharacteristic::getProperty()
{
    return _property;
}

byte btCharacteristic::getValueLength()
{
    return _valueLength;
}

word btCharacteristic::getHandle()
{
    return _handle;
}

void btCharacteristic::setListener(void (*ftListener)(char*))
{
    _ftListener=ftListener;
}

void btCharacteristic::setHandle(word handle)
{
    _handle=handle;
}


void btCharacteristic::callListener(char* data)
{
    if(!_ftListener)
    {
        return;
    }
    _ftListener(data);
}
