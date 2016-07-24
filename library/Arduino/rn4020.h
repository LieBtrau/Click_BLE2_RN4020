#ifndef RN4020_H
#define RN4020_H
#include "Arduino.h"
#include <SoftwareSerial.h>

class rn4020
{
public:
    typedef enum
    {
	SRV_SERVICES,
	SUP_FEATURES,
	BAUDRATE
    }SETGET;
    rn4020(HardwareSerial &s, byte pinCts, byte pinWake_sw, byte pinWake_hw, byte pinEnPwr, byte pinBtActive);
    bool begin(unsigned long baudrate);
    bool set(SETGET st, unsigned long ulValue);
    bool doFactoryDefault();
    bool reset(unsigned long baudrate);
private:
    void getLine(unsigned long uiTimeout, char *buf, byte buflength);
    bool waitForReply(unsigned long uiTimeout, const char *pattern);
    bool isModuleActive(unsigned long uiTimeout);
    bool waitForStartup(unsigned long baudrate);
    byte _pinWake_sw_7; //RN4020 pin 7
    byte _pinActive_12; //RN4020 pin 12
    byte _pinCts_14;    //RN4020 pin 14
    byte _pinWake_hw_15;//RN4020 pin 15
    byte _pinEnPwr;
};

#endif // RN4020_H
