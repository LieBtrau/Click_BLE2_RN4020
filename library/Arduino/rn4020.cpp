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
    _pinActive_12(pinBtActive),
    _ftConnection(0),
    _characteristicCount(0)
{
    sPort=&s;
    sPortDebug=&sw;
    memset(_lastCreatedService,'\0',sizeof(_lastCreatedService));
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

void rn4020::setConnectionListener(void (*ftConnection)(bool))
{
    _ftConnection=ftConnection;
}


void rn4020::loop()
{
    char* readLine;
    bool bEventHandled=false;
    if(!getLine(&readLine))
    {
        return;
    }
    if(strstr(readLine, "Connected") && _ftConnection)
    {
        _ftConnection(true);
        bEventHandled=true;
    }
    if(strstr(readLine, "Connection End") && _ftConnection)
    {
        _ftConnection(false);
        bEventHandled=true;
    }
    if(!strncmp(readLine, "WV,",3))
    {
        //Local server characteristic has been written by remote client
        char* pch= strtok(readLine,",");
        if(!pch)
        {
            return;
        }
        pch= strtok(NULL, ",");
        if(!pch)
        {
            return;
        }
        //Get the handle of the changed characteristic
        word handle;
        if(sscanf(pch, "%x", &handle)!=1)
        {
            return;
        }
        //look up the matching characteristic in our local list
        for(byte i=0;i<_characteristicCount;i++)
        {
            if(_characteristicList[i]->getHandle()==handle)
            {
                pch= strtok(NULL, ",");
                if(!pch)
                {
                    return;
                }
                //Call the event handler attached to this characteristic
                _characteristicList[i]->callListener(pch);
            }
        }
        bEventHandled=true;
    }
    if(!bEventHandled)
    {
        //If no matching string found
#if DEBUG_LEVEL >= DEBUG_ALL
        sPortDebug->print("Unknown input: ");
        sPortDebug->println(readLine);
#endif
    }
}

bool rn4020::setRole(rn4020::ROLES rl)
{
    switch(rl)
    {
    case PERIPHERAL:
        //Services: Device Information + Battery Level
        ble2_set_server_services(0xC0000001);
        if(!waitForReply(2000,"AOK"))
        {
            return false;
        }
        //Enable authentication with Keyboard and display as IO-capabilities
        ble2_set_supported_features(0x00480000);
        return waitForReply(2000,"AOK");
    default:
        return false;
    }
}

bool rn4020::setTxPower(byte pwr)
{
    if(pwr>7)
    {
        return false;
    }
    ble2_set_transmission_power((tx_pwr_t)pwr);
    return waitForReply(2000,"AOK");
}

bool rn4020::setBaudrate(unsigned long baud)
{
    switch(baud)
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
    return waitForReply(2000,"AOK");
}

//http://microchip.wikidot.com/ble:rn4020-power-states
bool rn4020::setOperatingMode(OPERATING_MODES om)
{
    bool bSuccess=false;
    bool bInNormalMode=false;
    switch (om) {
    case NORMAL:
        digitalWrite(_pinWake_sw_7, HIGH);
        digitalWrite(_pinWake_hw_15, HIGH);
        bSuccess=isModuleActive(2000);
        digitalWrite(_pinWake_hw_15, LOW);
        return bSuccess;
    case DEEP_SLEEP:
        bInNormalMode=isModuleActive(2000);
        digitalWrite(_pinWake_sw_7, LOW);
        if(bInNormalMode)
        {
            bSuccess=waitForReply(1000,"END\r\n");
        }
        digitalWrite(_pinWake_hw_15, HIGH);
        delay(10);
        digitalWrite(_pinWake_hw_15, LOW);
        return (!bInNormalMode) || bSuccess;
    case DORMANT:
        digitalWrite(_pinWake_sw_7, HIGH);
        bInNormalMode=isModuleActive(2000);
        ble2_dormant_mode_enable();
        //no reply on UART
        delay(200);
        digitalWrite(_pinWake_sw_7, LOW);
        return bInNormalMode;
    default:
        break;
    }
}

/* If the module is in an unknown state, e.g. unknown baudrate, then it can only be reset by toggling its
 * power.  Take this into account when designing a PCB with this module.  You should be able to toggle the power
 * of the RN4020 with a GPIO of your MCU.
 * At power up, the SW_WAKE pin is held high to force full factory reset when toggling the HW_WAKE lines.
 */
bool rn4020::doFactoryDefault()
{
    setOperatingMode(NORMAL);
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

bool rn4020::addCharacteristic(btCharacteristic* bt)
{
    char buf[50];
    if(getHandle(bt))
    {
        //Characteristic already exists
        return true;
    }
    bt->getUuidService(buf);
    if(strcmp(_lastCreatedService, buf))
    {
        //Service not created in previous call.  Create it now.
        strcpy(_lastCreatedService, buf);
        ble2_set_private_service_uuid(_lastCreatedService);
        if(!waitForReply(2000,"AOK"))
        {
            return false;
        }
    }
    //Set characteristics of the created service:
    bt->getUuidCharacteristic(buf);
    ble2_set_private_characteristics(buf,bt->getProperty(),bt->getValueLength(), bt->getSecurityBmp());
    if(!waitForReply(2000,"AOK"))
    {
        return false;
    }
    //Get handle of the created characteristic, so the callback functionality can work
    if(!getHandle(bt))
    {
        return false;
    }
    _characteristicList = (btCharacteristic**) realloc(_characteristicList, (_characteristicCount + 1) * sizeof(btCharacteristic*));
    if(!_characteristicList)
    {
        return false;
    }
    _characteristicList[_characteristicCount++]=bt;
    return true;
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

bool rn4020::dummy(void (*function)())
{
    //    char handle[10];
    //    if(!getHandle("1802","2A06", handle))
    //    {
    //#if DEBUG_LEVEL >= DEBUG_ALL
    //        sPortDebug->println("GetHandle Failed!!");
    //#endif
    //    }else
    //    {
    //#if DEBUG_LEVEL >= DEBUG_ALL
    //        sPortDebug->println((char*)handle);
    //#endif
    //    }
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

bool rn4020::getHandle(btCharacteristic* bt)
{
    const byte BUFFSIZE=250;
    char buf[BUFFSIZE];
    char buf2[50];
    bt->getUuidService(buf2);
    ble2_list_server_services();
    if(!waitForReply(2000, "END\r\n", buf, BUFFSIZE))
    {
        return false;
    }
    char* pch= strtok (buf,"\r\n");
    byte state=0;
    do
    {
        switch(state)
        {
        case 0:
            //Nothing found yet, split string in lines;
            if(strstr(pch,buf2))
            {
                state=1;
                bt->getUuidCharacteristic(buf2);
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
            if(strstr(pch, buf2))
            {
                state=2;
            }
            else
            {
                //get next characteristic in this service
                pch = strtok (NULL, "\r\n");
            }
            break;
        case 2:
            //Characteristic found, now looking for handle
            pch=strtok(pch,",");
            pch=strtok(NULL,",");
            if(pch)
            {
                word handle;
                if(sscanf(pch, "%x", &handle)==1)
                {
                    bt->setHandle(handle);
                    return true;
                }
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
//#if DEBUG_LEVEL >= DEBUG_ALL
//        sPortDebug->println(buf);
//#endif
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
