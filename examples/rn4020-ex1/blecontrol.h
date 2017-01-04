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
    typedef enum
    {
        FR_CENTRAL=0x80000000,
        FR_AUTH_KEYB_DISP=0x00480000,
        FR_SERV_ONLY=0x00002000
    }FEATURES;
    typedef enum    //Starting from FW-version 1.33BEC, only these two services are supported.  Other ones must be created by the user.
    {
        SRV_DEVICE_INFO=0x80000000,
        SRV_BATTERY=0x40000000,
        SRV_USR_PRIV_SERV=0x00000001
    }SERVICES;
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
