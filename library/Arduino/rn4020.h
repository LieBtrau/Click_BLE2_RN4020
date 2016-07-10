#ifndef RN4020_H
#define RN4020_H
#include "Arduino.h"
#include <SoftwareSerial.h>

class rn4020
{
public:
    rn4020(SoftwareSerial &s, byte pinCts, byte pinWake_sw, byte pinWake_hw, byte pinEnPwr, byte pinBtConnected);
    bool begin(bool bCentral);
private:
    void getResponse(unsigned int uiTimeout, String &readData);
    void reset();
    byte _pinWake_sw;
    byte _pinConnection;
    byte _pinCts;
    byte _pinWake_hw;
    byte _pinEnPwr;
};

#endif // RN4020_H
