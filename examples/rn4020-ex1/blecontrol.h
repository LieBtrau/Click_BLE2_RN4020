#ifndef BLECONTROL_H
#define BLECONTROL_H

#include "Arduino.h"

class bleControl
{
public:
    typedef enum
    {
        ST_NOTCONNECTED,
        ST_PASS_GENERATED,
        ST_PROV_BONDED,
        ST_BONDED
    }CONNECT_STATE;
    typedef enum
    {
        BLE_S_IMMEDIATE_ALERT_SERVICE,  //1802
        BLE_S_DEVICE_INFORMATION        //180A
    }BLE_SERVICES;
    typedef enum
    {
        BLE_CH_ALERT_LEVEL,             //2A06
        BLE_CH_SERIAL_NUMBER_STRING     //2A25
    }BLE_CHARACTERISTICS;
    bleControl();
    bool begin(bool bCentral);
    bool loop(void);
    bool getLocalMacAddress(byte* address, byte& length);
    bool findUnboundPeripheral(const char *remoteBtAddress);
    CONNECT_STATE secureConnect(const char* remoteBtAddress, CONNECT_STATE state);
    unsigned long getPasscode();
    bool writeServiceCharacteristic(BLE_SERVICES serv, BLE_CHARACTERISTICS chr, byte value);
    bool readServiceCharacteristic(BLE_SERVICES serv, BLE_CHARACTERISTICS chr, byte* value, byte& length);
    void disconnect();
private:
    word getRemoteHandle(BLE_SERVICES serv, BLE_CHARACTERISTICS chr);
};

#endif // BLECONTROL_H
