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
        PERIPHERAL
    }ROLES;
    typedef enum
    {
        NORMAL,
        DEEP_SLEEP,
        DORMANT
    }OPERATING_MODES;
    rn4020(HardwareSerial &s, byte pinWake_sw, byte pinBtActive, byte pinWake_hw, byte pinEnPwr);
    bool begin(unsigned long baudrate);
    void loop();
    bool doFactoryDefault();
    bool doReboot(unsigned long baudrate);
    bool doAdvertizing(bool bStartNotStop, unsigned int interval_ms);
    bool dummy(void(*function)());
    bool getMacAddress(byte* array, byte& length);
    bool setRole(ROLES rl);
    bool setTxPower(byte pwr);
    bool setBaudrate(unsigned long baud);
    bool setOperatingMode(OPERATING_MODES om);
    void setConnectionListener(void (*ftConnection)(bool));
    bool addCharacteristic(btCharacteristic* bt);
    bool removePrivateCharacteristics();
private:
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
    void (*_ftConnection)(bool bUp);
    btCharacteristic** _characteristicList;
    byte _characteristicCount;
    char _lastCreatedService[40];
};

#endif // RN4020_H
