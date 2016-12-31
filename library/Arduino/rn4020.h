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
        RL_UNDEFINED,
        RL_PERIPHERAL,
        RL_CENTRAL
    }ROLES;
    typedef enum
    {
        OM_NORMAL,
        OM_DEEP_SLEEP,
        OM_DORMANT
    }OPERATING_MODES;
    typedef struct
    {
        char btAddress[13];
        char privCharacteristic[33];
        char rssi;
    }ADVERTISEMENT;
    typedef enum
    {
        BD_PASSCODE_NEEDED,
        BD_ESTABLISHED
    }BONDING_MODES;
    rn4020(HardwareSerial &s, byte pinWake_sw, byte pinBtActive, byte pinWake_hw, byte pinEnPwr);
    bool addCharacteristic(btCharacteristic* bt);
    bool begin(unsigned long baudrate);
    bool doAdvertizing(bool bStartNotStop, unsigned int interval_ms);
    bool doConnecting(const char* remoteBtAddress);
    bool doDisconnect();
    bool doFindRemoteDevices(bool bEnabled);
    bool doReboot(unsigned long baudrate);
    bool doStopConnecting();
    bool doRemoveBond();
    bool getBluetoothDeviceName(char* btName);
    bool getMacAddress(byte* array, byte& length);
    void loop();
    bool removePrivateCharacteristics();
    void setAdvertisementListener(void(*ftAdvertisementReceived)(ADVERTISEMENT*));
    bool setBluetoothDeviceName(const char* btName);
    void setBondingListener(void (*ftBonding)(BONDING_MODES bd));
    void setBondingPasscodeListener(void (*ftPasscode)(unsigned long));
    void setBondingPasscode(const char* passcode);
    void setConnectionListener(void (*ftConnection)(bool));
    bool setFeatures(uint32_t features);
    bool setOperatingMode(OPERATING_MODES om);
    bool setServices(uint32_t services);
    bool setTxPower(byte pwr);
    bool startBonding();
private:
    word getNrOfOccurrence(char* buf, char findc);
    void cyclePower(OPERATING_MODES om);
    bool doFactoryDefault();
    bool gotLine();
    void hex2array(char* hexstringIn, byte *arrayOut, byte& lengthOut);
    bool isModuleActive(unsigned long uiTimeout);
    bool parseAdvertisement(char* buffer);
    bool setBaudrate(unsigned long baud);
    void updateHandles();
    bool waitForNrOfLines(unsigned long ulTimeout, byte nrOfEols);
    bool waitForReply(unsigned long uiTimeout, const char *pattern);
    bool waitForStartup(unsigned long baudrate);
    byte _pinWake_sw_7; //RN4020 pin 7
    byte _pinActive_12; //RN4020 pin 12
    byte _pinWake_hw_15;//RN4020 pin 15
    byte _pinEnPwr;
    void (*_ftConnectionStateChanged)(bool bUp);
    void (*_ftBondingEvent)(BONDING_MODES bd);
    void (*_ftAdvertisementReceived)(ADVERTISEMENT* adv);
    void (*_ftPasscodeGenerated)(unsigned long);
    btCharacteristic** _characteristicList;
    byte _characteristicCount;
    char _lastCreatedService[40];
};

#endif // RN4020_H
