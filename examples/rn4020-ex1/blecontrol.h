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
        BLE_S_IMMEDIATE_ALERT_SERVICE  //1802
    }BLE_SERVICES;
    typedef enum
    {
        BLE_CH_ALERT_LEVEL  //2A06
    }BLE_CHARACTERISTICS;
    bleControl();
    bool begin(bool bCentral);
    bool loop(void);
    bool getLocalMacAddress(byte* address, byte& length);
    bool findUnboundPeripheral(const char *remoteBtAddress);
    CONNECT_STATE secureConnect(const char* remoteBtAddress, CONNECT_STATE state);
    unsigned long getPasscode();
    bool writeServiceCharacteristic(BLE_SERVICES serv, BLE_CHARACTERISTICS chr, byte value);
    void disconnect();
};

#endif // BLECONTROL_H
