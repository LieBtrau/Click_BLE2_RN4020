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
        OM_DEEP_SLEEP,  //BLE modem still works, UART works in 2400baud
        OM_DORMANT      //Pairing status remembered, all other functionality off
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
        BD_BONDED,
        BD_SECURED
    }BONDING_MODES;
    rn4020(HardwareSerial &s, byte pinWake_sw, byte pinBtActive, byte pinWake_hw, byte pinEnPwr, byte pinBtConnected);
    bool begin(unsigned long baudrate);
    bool doAddCharacteristic(btCharacteristic* bt);
    bool doAddService(btCharacteristic* bt);
    bool doAdvertizing(bool bStartNotStop, unsigned int interval_ms);
    bool startConnecting(const byte *remoteBtAddress);
    bool doDisconnect();
    bool doFindRemoteDevices(byte **&macList, byte &nrOfItems, unsigned long timeout);
    bool doReboot(unsigned long baudrate);
    bool doStopConnecting();
    bool doRemoveBond();
    bool doRemovePrivateCharacteristics();
    bool doReadLocalCharacteristic(word handle, byte* array, byte& length);
    bool doReadRemoteCharacteristic(word handle, byte* array, byte& length);
    void doUpdateHandles(btCharacteristic* characteristicList[], byte nrOfChrs);
    bool doWriteLocalCharacteristic(word handle, const byte *array, byte length);
    bool doWriteRemoteCharacteristic(word handle, const byte *array, byte length);
    bool getBluetoothDeviceName(char* btName);
    word getLocalHandle(btCharacteristic* bt);
    bool getMacAddress(byte* array, byte& length);
    word getRemoteHandle(btCharacteristic *bt);
    bool isBonded(bool &status, byte *macOut);
    bool isBonded(bool &status);
    bool isConnected();
    bool isConnectedTo(byte* macOut);
    void loop();
    void setAdvertisementListener(void(*ftAdvertisementReceived)(ADVERTISEMENT*));
    bool setBluetoothDeviceName(const char* btName);
    void setBondingListener(void (*ftBonding)(BONDING_MODES bd));
    void setBondingPasscodeListener(void (*ftPasscode)(unsigned long));
    void setBondingPasscode(unsigned long passcode);
    void setCharacteristicWrittenListener(void(*ftCharacteristicWritten)(word, byte*, byte value));
    void setConnectionListener(void (*ftConnection)(bool));
    bool setFeatures(uint32_t features);
    bool setOperatingMode(OPERATING_MODES om);
    bool setServices(uint32_t services);
    bool setTxPower(byte pwr);
    bool startBonding();
private:
    word getNrOfOccurrence(char* buf, char findc);
    void cyclePower(OPERATING_MODES om);
    bool doFactoryDefault(unsigned long baudrate);
    bool gotLine();
    void hex2array(char* hexstringIn, byte *arrayOut, byte& lengthOut);
    void array2hex(const byte* arrayIn, char* stringOut, byte length);
    bool isModuleActive(unsigned long uiTimeout);
    bool parseAdvertisement(char* buffer);
    word parseServicesList(btCharacteristic *bt);
    void resetBuffer();
    bool setBaudrate(unsigned long baud);
    word waitForNrOfLines(unsigned long ulTimeout, byte nrOfEols);
    bool waitForReply(unsigned long uiTimeout, const char *pattern);
    bool waitForStartup();
    void setLocalBaud(unsigned long baudrate);
    byte _pinWake_sw_7; //RN4020 pin 7
    byte _pinActive_12; //RN4020 pin 12
    byte _pinWake_hw_15;//RN4020 pin 15
    byte _pinEnPwr;
    byte _pinBtConnected;
    void (*_ftConnectionStateChanged)(bool bUp);
    void (*_ftBondingEvent)(BONDING_MODES bd);
    void (*_ftAdvertisementReceived)(ADVERTISEMENT* adv);
    void (*_ftPasscodeGenerated)(unsigned long);
    void (*_ftCharacteristicWritten)(word handle, byte* newValue, byte length);
};

#endif // RN4020_H
