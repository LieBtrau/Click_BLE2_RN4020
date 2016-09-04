#ifndef RN4020_H
#define RN4020_H
#include "Arduino.h"
#include "btcharacteristic.h"

#ifdef __cplusplus
extern "C"{
#endif
void UART_Wr_Ptr(char _data);
void UART_Write_Text(const char *_data);
#ifdef __cplusplus
} // extern "C"
#endif

class rn4020
{
public:
    typedef enum
    {
        RL_PERIPHERAL,
        RL_CENTRAL
    }ROLES;
    typedef enum
    {
        OM_NORMAL,
        OM_DEEP_SLEEP,
        OM_DORMANT
    }OPERATING_MODES;
    rn4020(HardwareSerial &s, byte pinWake_sw, byte pinBtActive, byte pinWake_hw, byte pinEnPwr);
    bool begin(unsigned long baudrate, ROLES role);
    void loop();
    bool doReboot(unsigned long baudrate);
    bool doAdvertizing(bool bStartNotStop, unsigned int interval_ms);
    bool getMacAddress(byte* array, byte& length);
    bool getBluetoothDeviceName(char* btName);
    bool setBluetoothDeviceName(const char* btName);
    bool setTxPower(byte pwr);
    bool setOperatingMode(OPERATING_MODES om);
    void setBondingPasscode(const char* passcode);
    void setConnectionListener(void (*ftConnection)(bool));
    void setBondingListener(void (*ftBonding)(void));
    bool addCharacteristic(btCharacteristic* bt);
    bool removePrivateCharacteristics();
    bool doFindRemoteDevices(bool bEnabled);
private:
    bool doFactoryDefault();
    bool setBaudrate(unsigned long baud);
    bool waitForLines(unsigned long ulTimeout, byte nrOfEols);
    bool gotLine();
    bool waitForReply(unsigned long uiTimeout, const char *pattern);
    bool isModuleActive(unsigned long uiTimeout);
    bool waitForStartup(unsigned long baudrate);
    void updateHandles();
    void cyclePower(OPERATING_MODES om);
    void hex2array(char* hexstringIn, byte *arrayOut, byte& lengthOut);
    word countChars(char* buf, char findc);
    byte _pinWake_sw_7; //RN4020 pin 7
    byte _pinActive_12; //RN4020 pin 12
    byte _pinWake_hw_15;//RN4020 pin 15
    byte _pinEnPwr;
    void (*_ftConnectionStateChanged)(bool bUp);
    void (*_ftBondingRequested)(void);
    btCharacteristic** _characteristicList;
    byte _characteristicCount;
    char _lastCreatedService[40];
    ROLES _role;
};

#endif // RN4020_H
