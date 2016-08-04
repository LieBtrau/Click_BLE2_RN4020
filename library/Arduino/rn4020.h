#ifndef RN4020_H
#define RN4020_H
#include "Arduino.h"
#include <SoftwareSerial.h>
#include "btcharacteristic.h"

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
    rn4020(HardwareSerial &s, byte pinWake_sw, byte pinWake_hw, byte pinEnPwr, byte pinBtActive);
    bool begin(unsigned long baudrate);
    void loop();
    bool doFactoryDefault();
    bool doReboot(unsigned long baudrate);
    bool doAdvertizing(bool bStartNotStop, unsigned int interval_ms);
    bool dummy(void(*function)());
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
    bool getHandle(btCharacteristic* pbt);
    void cyclePower(OPERATING_MODES om);
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
