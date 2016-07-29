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

rn4020::rn4020(HardwareSerial &s, byte pinWake_sw, byte pinWake_hw, byte pinEnPwr, byte pinBtActive):
    _pinWake_sw_7(pinWake_sw),
    _pinWake_hw_15(pinWake_hw),
    _pinEnPwr(pinEnPwr),
    _pinActive_12(pinBtActive)
{
    sPort=&s;
    sPortDebug=&sw;
    ble2_hal_init();
}

bool rn4020::begin(unsigned long baudrate)
{
    pinMode(_pinEnPwr, OUTPUT);
    pinMode(_pinWake_hw_15, OUTPUT);
    pinMode(_pinWake_sw_7, OUTPUT);
    pinMode(_pinActive_12, INPUT);

    digitalWrite(_pinWake_hw_15, LOW);
    digitalWrite(_pinWake_sw_7, LOW);
    digitalWrite(_pinEnPwr, HIGH);//module OFF
    delay(100);
    digitalWrite(_pinEnPwr, LOW);//module ON
    digitalWrite(_pinWake_sw_7, HIGH);
    return waitForStartup(baudrate);
}

bool rn4020::update(rn4020::STATUS_UPDATES& su)
{
    char* readLine;
    su=SU_UNKNOWN;
    if(!getLine(&readLine))
    {
        return false;
    }
    if(strstr(readLine, "Connected"))
    {
        su=SU_CONNECTED;
    }
    if(strstr(readLine, "Connection End"))
    {
        su=SU_DISCONNECTED;
    }
    if(su==SU_UNKNOWN)
    {
        //If no matching string found
#if DEBUG_LEVEL >= DEBUG_ALL
        sPortDebug->print("Unknown input: ");
        sPortDebug->println(readLine);
#endif
    }
    return true;
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

bool rn4020::doReboot(unsigned long baudrate)
{
    pinMode(A0, OUTPUT);
    digitalWrite(A0, HIGH);
    delay(1);
    digitalWrite(A0, LOW);
    ble2_device_reboot();
    //ProTrinket 3V gets framing errors when trying to receive the "Reboot".
    //waitForReply(2000,"Reboot");
    return waitForStartup(baudrate);
}


bool rn4020::set(rn4020::SETGET st, unsigned long ulValue)
{

    switch(st)
    {
    case SG_SRV_SERVICES:
        ble2_set_server_services(ulValue);
        break;
    case SG_SUP_FEATURES:
        ble2_set_supported_features(ulValue);
        break;
    case SG_BAUDRATE:
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
            return false;
        }
        break;
    case SG_TX_POWER:
        if(ulValue>7)
        {
            return false;
        }
        ble2_set_transmission_power((tx_pwr_t)ulValue);
        break;
    default:
#if DEBUG_LEVEL >= DEBUG_ALL
        sPortDebug->println("Undefined set command");
#endif
        return false;
    }
    if(!waitForReply(2000,"AOK"))
    {
#if DEBUG_LEVEL >= DEBUG_ALL
        sPortDebug->println("No response for set");
#endif
        return false;
    }
    return true;
}

bool rn4020::createService(rn4020::SERVICES srv)
{
    //Create service: https://www.bluetooth.com/specifications/gatt/services
    switch(srv)
    {
    case SRV_IAS:
        ble2_set_private_service_uuid("1802");
        break;
    default:
        return false;
    }
    if(!waitForReply(2000,"AOK"))
    {
        return false;
    }
    //Set characteristics of the created service: https://www.bluetooth.com/specifications/gatt/characteristics
    switch(srv)
    {
    case SRV_IAS:
        ble2_set_private_characteristics("2A06","04",1);//"write without response", 1 byte maximum
        break;
    default:
        return false;
    }
    return waitForReply(2000,"AOK");
}

bool rn4020::doAdvertizing(bool bStartNotStop, unsigned int interval_ms)
{
    if(bStartNotStop)
    {
        //Start
        ble2_start_advertisment(interval_ms,0);
    }else
    {
        //Stop
        ble2_stop_advertising();
    }
    return waitForReply(2000,"AOK");
}

bool rn4020::dummy()
{
    char handle[10];
    if(!getHandle("1802","2A06", handle))
    {
#if DEBUG_LEVEL >= DEBUG_ALL
        sPortDebug->println("GetHandle Failed!!");
#endif
    }else
    {
#if DEBUG_LEVEL >= DEBUG_ALL
        sPortDebug->println((char*)handle);
#endif
    }
}

bool rn4020::waitForStartup(unsigned long baudrate)
{
    sPort->end();
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

bool rn4020::getHandle(const char* service, const char* characteristic, char handle[])
{
    ble2_list_server_services();
    const byte BUFFSIZE=200;
    char buf[BUFFSIZE];
    if(!handle)
    {
        return false;
    }
    if(!waitForReply(2000, "END\r\n", buf, BUFFSIZE))
    {
        return false;
    }
#if DEBUG_LEVEL >= DEBUG_ALL
    sPortDebug->println(buf);
#endif
    char* pch= strtok (buf,"\r\n");
    byte state=0;
    do
    {
        switch(state)
        {
        case 0:
            //Nothing found yet, split string in lines;
            if(strstr(pch,service))
            {
                state=1;
#if DEBUG_LEVEL >= DEBUG_ALL
                sPortDebug->println("Service found");
#endif
            }
            pch = strtok (NULL, "\r\n");
            break;
        case 1:
            //Service found, now looking for line with characteristic
            if(strncmp(pch,"  ",2))
            {
#if DEBUG_LEVEL >= DEBUG_ALL
                sPortDebug->println("Characteristic not found in service");
#endif
                return false;
            }
            if(strstr(pch, characteristic))
            {
                state=2;
            }
            else
            {
                pch = strtok (NULL, "\r\n");
            }
            break;
        case 2:
            //Characteristic found, now looking for handle
            pch=strtok(pch,",");
            pch=strtok(NULL,",");
            if(pch)
            {
                strcpy(handle, pch);
                return true;
            }
        }
    }while (pch != NULL);
    return false;
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

//Read multiple lines and search for pattern
bool rn4020::waitForReply(unsigned long uiTimeout, const char *pattern, char buf[], byte buffsize)
{
    if(!buf)
    {
        return false;
    }
    buf[0]='\0';
    unsigned long ulStartTime=millis();
    byte index=0;
    do{
        if(!sPort->available())
        {
            continue;
        }
        char c=sPort->read();
        if((((byte)c)!=0xFF) && (((byte)c)!=0x00))
        {
            buf[index<buffsize-1 ? index++ : index=0]=c;
        }
        buf[index]='\0';
    }while(millis()<ulStartTime+uiTimeout && !strstr(buf, pattern));
    return strstr(buf, pattern);
}

bool rn4020::waitForReply(unsigned long uiTimeout, const char* pattern)
{
    unsigned long ulStartTime=millis();
    char* readLine=0;
    do
    {
        if(getLine(&readLine) && strstr(readLine, pattern))
        {
            return true;
        }
    }while(millis()<uiTimeout+ulStartTime);
    return false;
}

bool rn4020::getLine(char** pReadLine)
{
    const byte BUFFSIZE=40;
    static char buf[BUFFSIZE];
    static byte index=0;
    static char b=0,c=0;
    if(!sPort->available())
    {
        return false;
    }
    b=c;
    c=sPort->read();
    if((((byte)c)!=0xFF) && (((byte)c)!=0x00))
    {
        buf[index<BUFFSIZE-1 ? index++ : index=0]=c;
#if DEBUG_LEVEL >= DEBUG_ALL
        char charbuf[20];
        sprintf(charbuf,"RX: %c \t\t%02x",c>27?c:'.',(byte)c);
        sPortDebug->println(charbuf);
#endif
    }
    if(b=='\r' && c=='\n')
    {
        *pReadLine=buf;
        buf[index]='\0';
        index=0;
        return true;
    }
    return false;
}
