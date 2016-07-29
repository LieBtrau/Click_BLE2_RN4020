#ifndef RN4020_H
#define RN4020_H
#include "Arduino.h"
#include <SoftwareSerial.h>

class rn4020
{
public:
    typedef enum
    {
        SG_SRV_SERVICES,
        SG_SUP_FEATURES,
        SG_BAUDRATE,
        SG_TX_POWER
    }SETGET;
    typedef enum
    {
        SRV_IAS //Immediate Alert Service
    }SERVICES;
    typedef enum
    {
        SU_CONNECTED,
        SU_DISCONNECTED,
        SU_UNKNOWN
    }STATUS_UPDATES;
    rn4020(HardwareSerial &s, byte pinWake_sw, byte pinWake_hw, byte pinEnPwr, byte pinBtActive);
    bool begin(unsigned long baudrate);
    bool update(STATUS_UPDATES& su);
    bool set(SETGET st, unsigned long ulValue);
    bool doFactoryDefault();
    bool doReboot(unsigned long baudrate);
    bool doAdvertizing(bool bStartNotStop, unsigned int interval_ms);
    bool createService(SERVICES srv);
    bool dummy();
private:
    bool getLine(char **pReadLine);
    bool waitForReply(unsigned long uiTimeout, const char *pattern);
    bool waitForReply(unsigned long uiTimeout, const char *pattern, char buf[], byte buffsize);
    bool isModuleActive(unsigned long uiTimeout);
    bool waitForStartup(unsigned long baudrate);
    bool getHandle(const char* service, const char* characteristic, char handle[]);
    byte _pinWake_sw_7; //RN4020 pin 7
    byte _pinActive_12; //RN4020 pin 12
    byte _pinWake_hw_15;//RN4020 pin 15
    byte _pinEnPwr;
};

#endif // RN4020_H
