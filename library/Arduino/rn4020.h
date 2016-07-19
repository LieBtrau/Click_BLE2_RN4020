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
    rn4020(SoftwareSerial &s, byte pinCts, byte pinWake_sw, byte pinWake_hw, byte pinEnPwr, byte pinBtActive);
    bool begin(unsigned long baudrate);
    bool set(SETGET st, unsigned long ulValue);
    bool doFactoryDefault();
private:
    void getResponse(unsigned int uiTimeout, char *readData, byte buflength);
    bool waitForReply(unsigned int uiTimeout, const char *pattern);
    bool isModuleActive(unsigned int uiTimeout);
    byte _pinWake_sw;
    byte _pinActive;
    byte _pinCts;
    byte _pinWake_hw;
    byte _pinEnPwr;
};

#endif // RN4020_H
