#include "rn4020.h"
#include "ble2_hw.h"

static HardwareSerial* sPort;
static SoftwareSerial* sPortDebug;
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

rn4020::rn4020(HardwareSerial &s, byte pinCts, byte pinWake_sw, byte pinWake_hw, byte pinEnPwr, byte pinBtActive):
    _pinCts_14(pinCts),
    _pinWake_sw_7(pinWake_sw),
    _pinWake_hw_15(pinWake_hw),
    _pinEnPwr(pinEnPwr),
    _pinActive_12(pinBtActive)
{
    sPort=&s;
    sPortDebug=&sw;
    ble2_hal_init();
}

bool rn4020::reset(unsigned long baudrate)
{
#if DEBUG_LEVEL >= DEBUG_ALL
    sPortDebug->print("Bytes available: ");
    sPortDebug->println(sPort->available());
#endif
    ble2_device_reboot();
    //ProTrinket 3V gets framing errors when trying to receive the "Reboot".
//    if(!waitForReply(2000,"Reboot"))
//    {
//#if DEBUG_LEVEL >= DEBUG_ALL
//        sPortDebug->println("Module doesn't reboot");
//#endif
//        return false;
//    }
    return waitForStartup(baudrate);
}

bool rn4020::begin(unsigned long baudrate)
{
    pinMode(_pinEnPwr, OUTPUT);
    pinMode(_pinWake_hw_15, OUTPUT);
    pinMode(_pinWake_sw_7, OUTPUT);
    pinMode(_pinCts_14, OUTPUT);
    pinMode(_pinActive_12, INPUT);

    digitalWrite(_pinCts_14,LOW);
    digitalWrite(_pinWake_hw_15, LOW);
    digitalWrite(_pinWake_sw_7, LOW);
    digitalWrite(_pinEnPwr, HIGH);//module OFF
    delay(100);
    digitalWrite(_pinEnPwr, LOW);//module ON
    digitalWrite(_pinWake_sw_7, HIGH);
    return waitForStartup(baudrate);
}


/* If the module is in an unknown state, e.g. unknown baudrate, then it can only be reset by toggling its
 * power.  Take this into account when designing a PCB with this module.  You should be able to toggle the power
 * of the RN4020 with a GPIO of your MCU.
 * At power up, the SW_WAKE pin is held high to force full factory reset when toggling the HW_WAKE lines.
 */
bool rn4020::doFactoryDefault()
{
    digitalWrite(_pinWake_sw_7, HIGH);
    if(!isModuleActive(2000))
    {
        return false;
    }
    delay(200);
#if DEBUG_LEVEL >= DEBUG_ALL
    sPortDebug->println("Forcing factory default.");
#endif
    for(byte i=0;i<8;i++)
    {
        digitalWrite(_pinWake_hw_15, HIGH);
        delay(200);
        digitalWrite(_pinWake_hw_15, LOW);
        delay(200);
    }
    //Give module time to reset.
    //It won't send any data to signal a succeeded factory reset.  Pin 12 will remain high
    delay(1500);
    return true;
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
            //#if DEBUG_LEVEL >= DEBUG_ALL
            //            sPortDebug->println("Unknown baud rate");
            //#endif
            return false;
        }
        if(!waitForReply(2000,"AOK"))
        {
#if DEBUG_LEVEL >= DEBUG_ALL
            sPortDebug->println("Baudrate change not accepted");
#endif
            return false;
        }
        return true;
    default:
#if DEBUG_LEVEL >= DEBUG_ALL
        sPortDebug->println("Undefined set command");
#endif
        return false;
    }
}

bool rn4020::waitForStartup(unsigned long baudrate)
{
    sPort->begin(baudrate);
    //After power up, it takes about 1.28s for the RN4020 to become active
    if(!isModuleActive(1500))
    {
        return false;
    }
    //25µs after pin12 came high, "CMD\r\n" will be sent out on the UART
    if(!waitForReply(2000,"CMD"))
    {
        return false;
    }
#if DEBUG_LEVEL >= DEBUG_ALL
    sPortDebug->println("RN4020 Module found");
#endif
    return true;
}

bool rn4020::isModuleActive(unsigned long uiTimeout)
{
    unsigned long ulStartTime=millis();
    if(!digitalRead(_pinActive_12))
    {
        //Clean serial port buffer
        while(sPort->available())
        {
            sPort->read();
        }
    }
    while(!digitalRead(_pinActive_12))
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

bool rn4020::waitForReply(unsigned long uiTimeout, const char* pattern)
{
    char readLine[10]={0};
    getLine(uiTimeout, readLine, 10);
    int result=strspn(readLine, pattern);
//#if DEBUG_LEVEL >= DEBUG_ALL
//    sPortDebug->println(result, DEC);
//#endif
    return result == strlen(pattern);
}

void rn4020::getLine(unsigned long uiTimeout, char* buf, byte buflength)
{
    byte index=0;
    char b=0,c=0;
    if(!buf)
    {
        return;
    }
    buf[0]='\0';
    unsigned long ulStartTime=millis();
    while(millis()<ulStartTime+uiTimeout && index<buflength)
    {
        if(sPort->available())
        {
            b=c;
            c=sPort->read();
            if((((byte)c)!=0xFF) && (((byte)c)!=0x00))
            {
                buf[index++]=c;
            }
            if(b=='\r' && c=='\n')
            {
                break;
            }
        }
    }
    buf[index]='\0';
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
        sPortDebug->print(buf[i]>27 ? buf[i]: '.');
    }
    sPortDebug->print("\t\t");
    //Print data in HEX format
    for(byte i=0;i<index;i++)
    {
        sPortDebug->print((byte)buf[i],HEX);
        sPortDebug->print(" ");
    }
    sPortDebug->println();
#endif
}
