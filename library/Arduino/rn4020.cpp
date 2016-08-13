#include "rn4020.h"
#include "ble2_hw.h"

#define DEBUG_LEVEL DEBUG_ALL

#ifdef ARDUINO_AVR_PROTRINKET3FTDI
#include <SoftwareSerial.h>
static SoftwareSerial* sPortDebug;
extern SoftwareSerial* sw;
#elif defined(ARDUINO_STM_NUCLEO_F103RB)
static HardwareSerial* sPortDebug;
extern HardwareSerial* sw;
#endif

static HardwareSerial* sPort;
const byte BUFFSIZE=250;
static char rxbuf[BUFFSIZE];
static byte indexRxBuf=0;

void UART_Wr_Ptr(char _data)
{
    sPort->write(_data);
#if DEBUG_LEVEL >= DEBUG_ALL
    sPortDebug->println(_data);
#endif
}

void UART_Write_Text(const char *_data)
{
    sPort->print(_data);
#if DEBUG_LEVEL >= DEBUG_ALL
    if(!strlen(_data))
    {
        sPortDebug->print("Empty tx msg");
    }else
    {
        sPortDebug->print("TX: ");
        sPortDebug->println(_data);
    }
#endif
}

rn4020::rn4020(HardwareSerial &s, byte pinWake_sw, byte pinBtActive, byte pinWake_hw, byte pinEnPwr):
    _pinWake_sw_7(pinWake_sw),
    _pinWake_hw_15(pinWake_hw),
    _pinEnPwr(pinEnPwr),
    _pinActive_12(pinBtActive),
    _ftConnection(0),
    _characteristicCount(0)
{
    sPort=&s;
    sPortDebug=sw;
    memset(_lastCreatedService,'\0',sizeof(_lastCreatedService));
    ble2_hal_init();
}

bool rn4020::begin(unsigned long baudrate)
{
    pinMode(_pinEnPwr, OUTPUT);
    pinMode(_pinWake_hw_15, OUTPUT);
    pinMode(_pinWake_sw_7, OUTPUT);
    pinMode(_pinActive_12, INPUT);
    cyclePower(NORMAL);
    return waitForStartup(baudrate);
}

void rn4020::cyclePower(OPERATING_MODES om)
{
    digitalWrite(_pinWake_hw_15, LOW);
    digitalWrite(_pinWake_sw_7, LOW);
    digitalWrite(_pinEnPwr, HIGH);//module OFF
    delay(100);
    digitalWrite(_pinEnPwr, LOW);//module ON
    if(om==NORMAL)
    {
        digitalWrite(_pinWake_sw_7, HIGH);
    }
}

void rn4020::setConnectionListener(void (*ftConnection)(bool))
{
    _ftConnection=ftConnection;
}


void rn4020::loop()
{
    bool bEventHandled=false;
    word handle;
    byte length;
    char value[42];
    if(!gotLine())
    {
        return;
    }
    indexRxBuf=0;
    if(strstr(rxbuf, "Connected") && _ftConnection)
    {
        _ftConnection(true);
        bEventHandled=true;
    }
    if(strstr(rxbuf, "Connection End") && _ftConnection)
    {
        _ftConnection(false);
        bEventHandled=true;
    }
    if(sscanf(rxbuf, "WV,%04x,%s ", &handle,value)==2)
    {
        //look up the matching characteristic in our local list
        for(byte i=0;i<_characteristicCount;i++)
        {
            if(_characteristicList[i]->getHandle()==handle)
            {
                //Local server characteristic has been written by remote client
                length=_characteristicList[i]->getValueLength();
                hex2array(value,length);
                //Call the event handler attached to this characteristic
                _characteristicList[i]->callListener(value, length);
            }
        }
        bEventHandled=true;
    }
    if(!bEventHandled)
    {
        //If no matching string found
#if DEBUG_LEVEL >= DEBUG_ALL
        sPortDebug->print("Unknown input: ");
        sPortDebug->println(rxbuf);
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

bool rn4020::removePrivateCharacteristics()
{
    ble2_private_service_clear_all();
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
    //ProTrinket 3V gets framing errors when trying to receive the "Reboot" at 115200baud.
    //waitForReply(2000,"Reboot");
    return waitForStartup(baudrate);
}

bool rn4020::addCharacteristic(btCharacteristic* bt)
{
    if(bt->getHandle())
    {
        //handle already exists
        return true;
    }
    if(strcmp(_lastCreatedService, bt->getUuidService()))
    {
        //Service not created in previous call.  Create it now.
        strcpy(_lastCreatedService, bt->getUuidService());
        ble2_set_private_service_uuid(_lastCreatedService);
        if(!waitForReply(2000,"AOK"))
        {
            return false;
        }
    }
    //Set characteristics of the created service:
    ble2_set_private_characteristics(bt->getUuidCharacteristic(),bt->getProperty(),bt->getValueLength(), bt->getSecurityBmp());
    if(!waitForReply(2000,"AOK"))
    {
        return false;
    }
    //Created characteristics only become available after reboot.
    if(!doReboot(2400))
    {
        return false;
    }
    _characteristicList = (btCharacteristic**) realloc(_characteristicList, (_characteristicCount + 1) * sizeof(btCharacteristic*));
    if(!_characteristicList)
    {
        return false;
    }
    _characteristicList[_characteristicCount++]=bt;
    //Get the handle of the characteristic
    updateHandles();
    return true;
}

bool rn4020::doAdvertizing(bool bStartNotStop, unsigned int interval_ms)
{
    if(bStartNotStop)
    {
        //Start
        ble2_start_advertisement(interval_ms,0);
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
    return waitForReply(2000,"CMD");
}

/* When adding services and characteristics to the RN4020, the handles of the existing services and characteristics
 * change.  This function updates those handles.
 */
void rn4020::updateHandles()
{
    char* pch;
    byte state=0;
    byte ctr=0;

    //Get list of services
    ble2_list_server_services();
    if(!waitForReply(2000, "END\r\n"))
    {
        return;
    }
    //Parse response line by line
    pch = strtok (rxbuf,"\r\n");
    do
    {
        switch(state)
        {
        case 0:
            //Check if the line contains a service
            for(ctr=0;ctr<_characteristicCount;ctr++)
            {
                //Nothing found yet, split string in lines;
                if(strstr(pch,_characteristicList[ctr]->getUuidService()))
                {
                    //Service found, now looking for line with characteristic
                    state=1;
                    break;
                }
            }
            break;
        case 1:
            if(strncmp(pch,"  ",2))
            {
                //String doesn't start with two spaces, so this is not a characteristic.  This is an error.
                return;
            }
            if(strstr(pch, _characteristicList[ctr]->getUuidCharacteristic()))
            {
                //Characteristic found, now looking for handle
                char* pch2=strchr(pch,',')+1;
                word handle;
                if(sscanf(pch2, "%x,", &handle)==1)
                {
//#if DEBUG_LEVEL >= DEBUG_ALL
//                    sPortDebug->println("Setting handle");
//                    sPortDebug->println(handle, HEX);
//#endif
                    _characteristicList[ctr]->setHandle(handle);

                }
                state=0;
                break;
            }
        }
        pch = strtok (NULL, "\r\n");
    }while (pch != NULL);
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
bool rn4020::waitForReply(unsigned long uiTimeout, const char *pattern)
{
    rxbuf[0]='\0';
    indexRxBuf=0;
    if(!pattern || !strlen(pattern))
    {
#if DEBUG_LEVEL >= DEBUG_ALL
        sPortDebug->println("No pattern defined");
#endif
        return false;
    }
    unsigned long ulStartTime=millis();
    do{
        gotLine();
    }while(millis()<ulStartTime+uiTimeout && (!strstr(rxbuf, pattern)));
    indexRxBuf=0;
#if DEBUG_LEVEL >= DEBUG_ALL
    //    sPortDebug->print("Pattern: ");
    //    sPortDebug->println(pattern);
    //    sPortDebug->println((unsigned long)strstr(rxbuf, pattern), DEC);
    char* pch=rxbuf;
    char b=0,c=0;
    sPortDebug->print("RX: ");
    while(*pch)
    {
        if(*pch>27)
        {
            sPortDebug->print(*pch);
        }
        b=c;
        c=*pch;
        if(b=='\r' && c=='\n')
        {
            sPortDebug->print("\r\nRX: ");
        }
        pch++;
    }
    sPortDebug->println();
#endif
    return strstr(rxbuf, pattern);
}

bool rn4020::gotLine()
{
    if(!sPort->available())
    {
        return false;
    }
    char c=sPort->read();
    if(c!='\xFF' && c!='\0')
    {
        rxbuf[indexRxBuf]=c;
        rxbuf[++indexRxBuf]='\0';
        if(indexRxBuf==BUFFSIZE-1)
        {
            indexRxBuf=0;
        }
    }
    return strstr(rxbuf,"\r\n");
}

void rn4020::hex2array(char* hexstring, byte& length)
{
    byte hexlength=strlen(hexstring)>>1;
    length=hexlength>length ? length : hexlength;
    for(byte i=0;i<length;i++)
    {
        sscanf(hexstring+(i<<1),"%2x", hexstring+i);
    }
    hexstring[length]='\0';
}
