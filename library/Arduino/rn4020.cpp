#include "rn4020.h"
#include "ble2_hw.h"

static SoftwareSerial* sPort;
static HardwareSerial* sPortDebug;
extern SoftwareSerial sw;

#ifdef __cplusplus
extern "C"{
#endif
void UART_Wr_Ptr(unsigned char _data)
{
    sPort->write(_data);
#if DEBUG_LEVEL >= DEBUG_ALL
    sPortDebug->println(_data);
#endif
}

void UART_Write_Text(unsigned char *_data)
{
    sPort->print((const char*)_data);
#if DEBUG_LEVEL >= DEBUG_ALL
    sPortDebug->print("TX: ");
    sPortDebug->println((const char*)_data);
#endif
}
#ifdef __cplusplus
} // extern "C"
#endif

rn4020::rn4020(SoftwareSerial &s, byte pinCts, byte pinWake_sw, byte pinWake_hw, byte pinEnPwr, byte pinBtActive):
    _pinCts(pinCts),
    _pinWake_sw(pinWake_sw),
    _pinWake_hw(pinWake_hw),
    _pinEnPwr(pinEnPwr),
    _pinActive(pinBtActive)
{
    sPort=&s;
    sPortDebug=&Serial;
    ble2_hal_init();
}

bool rn4020::begin(unsigned long baudrate)
{
    pinMode(_pinEnPwr, OUTPUT);
    digitalWrite(_pinEnPwr, HIGH);//module OFF
    pinMode(_pinWake_hw, OUTPUT);
    digitalWrite(_pinWake_hw, LOW);
    pinMode(_pinWake_sw, OUTPUT);
    pinMode(_pinCts, OUTPUT);
    digitalWrite(_pinCts,LOW);
    pinMode(_pinActive, INPUT);
    delay(100);
    digitalWrite(_pinEnPwr, LOW);//module ON
    digitalWrite(_pinWake_sw, HIGH);
    sPort->begin(baudrate);
    if(!isModuleActive(1500))
    {
        return false;
    }
    //25µs after pin12 came high, "CMD\r\n" will be sent out on the UART
    if(!waitForReply(1000,"CMD"))
    {
        return false;
    }
#if DEBUG_LEVEL >= DEBUG_ALL
    sPortDebug->println("RN4020 Module found");
#endif
    return true;
}

void rn4020::getResponse(unsigned int uiTimeout, char* readData, byte buflength)
{
    byte index=0;
    if(!readData)
    {
        return;
    }
    readData[0]='\0';
    unsigned long ulStartTime=millis();
    while(millis()<ulStartTime+uiTimeout && index<buflength)
    {
        if(sPort->available())
        {
            char c=sPort->read();
            readData[index++]=c;
        }
    }
#if DEBUG_LEVEL >= DEBUG_ALL
    if(!index)
    {
        sPortDebug->println("Nothing received");
        return;
    }
    sPortDebug->print("RX: ");
    //Print data in plain text format
    for(byte i=0;i<index;i++)
    {
        sPortDebug->print(readData[i]>27 ? readData[i]: '.');
    }
    sPortDebug->print("\t\t");
    //Print data in HEX format
    for(byte i=0;i<index;i++)
    {
        sPortDebug->print((byte)readData[i],HEX);
        sPortDebug->print(" ");
    }
    sPortDebug->println();
#endif
}

bool rn4020::waitForReply(unsigned int uiTimeout, const char* pattern)
{
    char readData[100]={0};
    getResponse(uiTimeout, readData,100);
    return (strncmp(readData, pattern, strlen(pattern))==0);
}

/* If the module is in an unknown state, e.g. unknown baudrate, then it can only be reset by toggling its
 * power.  Take this into account when designing a PCB with this module.  You should be able to toggle the power
 * of the RN4020 with a GPIO of your MCU.
 * At power up, the SW_WAKE pin is held high to force full factory reset when toggling the HW_WAKE lines.
 * By watching the UART, this function verifies that the module has reset correctly.
 */
bool rn4020::doFactoryDefault()
{

    //    //Factory reset the module
    //    for(byte i=0;i<3;i++)
    //    {
    //	digitalWrite(_pinWake_hw, HIGH);
    //	delay(200);
    //	digitalWrite(_pinWake_hw, LOW);
    //	delay(200);
    //    }
}

bool rn4020::set(rn4020::SETGET st, unsigned long ulValue)
{

    switch(st)
    {
    case SRV_SERVICES:
        ble2_set_server_services(ulValue);
        return waitForReply(2000,"AOK");
    case SUP_FEATURES:
        ble2_set_supported_features(ulValue);
        return waitForReply(2000,"AOK");
    case BAUDRATE:
        switch(ulValue)
        {
        case 2400:
            ble2_set_baud_rate(BR_2400);
            break;
        case 9600:
            ble2_set_baud_rate(BR_9600);
            break;
        case 19200:
            ble2_set_baud_rate(BR_19200);
            break;
        case 38400:
            ble2_set_baud_rate(BR_38400);
            break;
        case 115200:
            ble2_set_baud_rate(BR_115200);
            break;
        case 230400:
            ble2_set_baud_rate(BR_230400);
            break;
        case 460800:
            ble2_set_baud_rate(BR_460800);
            break;
        case 921600:
            ble2_set_baud_rate(BR_921600);
            break;
        default:
#if DEBUG_LEVEL >= DEBUG_ALL
            sPortDebug->println("Unknown baud rate");
#endif
        }
        if(!waitForReply(2000,"AOK"))
        {
            return false;
        }
        ble2_device_reboot();
        sPort->begin(ulValue);
        if(!waitForReply(1000,"Reboot"))
        {
            return false;
        }
        return waitForReply(2000,"CMD");
    default:
#if DEBUG_LEVEL >= DEBUG_ALL
        sPortDebug->println("Undefined set command");
#endif
        return false;
    }
}

bool rn4020::isModuleActive(unsigned int uiTimeout)
{
    unsigned long ulStartTime=millis();
    while(!digitalRead(_pinActive))
    {
        //Typically pin12 comes high 1.28s after powerup.
        if(millis()>ulStartTime+uiTimeout)
        {
#if DEBUG_LEVEL >= DEBUG_ALL
            sPortDebug->println("Module doesn't become active");
#endif
            return false;
        }
    }
    return true;
}
