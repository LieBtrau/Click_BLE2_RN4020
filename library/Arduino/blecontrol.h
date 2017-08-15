#ifndef BLECONTROL_H
#define BLECONTROL_H

#include "Arduino.h"
#include "rn4020.h"

class bleControl
{
public:
    typedef enum
    {
        FR_CENTRAL=0x80000000,
        FR_AUTH_KEYB_DISP=0x00480000,
        FR_SERV_ONLY=0x00002000
    }FEATURES;
    typedef enum
    {
        EV_PASSCODE_WANTED,
        EV_PASSCODE_GENERATED,
        EV_CONNECTION_UP,
        EV_CONNECTION_DOWN
    }EVENT;
    typedef enum    //Starting from FW-version 1.33BEC, only these two services are supported.  Other ones must be created by the user.
    {
        SRV_DEVICE_INFO=0x80000000,
        SRV_BATTERY=0x40000000,
        SRV_USR_PRIV_SERV=0x00000001
    }SERVICES;
    bleControl(rn4020 *prn);
    bool init();
    bool beginCentral();
    bool programPeripheral();
    bool programCentral();
    bool beginPeripheral(btCharacteristic **localCharacteristics, byte nrOfCharacteristics);
    bool loop(void);
    bool getLocalMacAddress(byte* address, byte& length);
    void setEventListener(void(*ftEventReceived)(EVENT));
    void setPasscode(unsigned long pass);
    bool findUnboundPeripheral(const byte *remoteBtAddress);
    unsigned long getPasscode();
    bool getBluetoothDeviceName(char* btName);
    bool setBluetoothDeviceName(const char* btName);
    bool addLocalCharacteristics(btCharacteristic **localCharacteristics, byte nrOfCharacteristics);
    bool writeLocalCharacteristic(btCharacteristic* bt, byte value);
    bool writeRemoteCharacteristic(btCharacteristic* bt, byte value);
    bool readRemoteCharacteristic(btCharacteristic* bt, byte* value, byte& length);
    bool readLocalCharacteristic(btCharacteristic *bt, byte* value, byte& length);
    bool secureConnect(const byte *peripheralMac);
    void disconnect();
private:
    typedef enum
    {
        ST_NOTCONNECTED,
        ST_CONNECTED,
        ST_PASSCODE_GENERATED,
        ST_PROV_BONDED,
        ST_BONDED
    }CONNECT_STATE;
    word getRemoteHandle(btCharacteristic *bt);
    word getLocalHandle(btCharacteristic *bt);
};

#endif // BLECONTROL_H
