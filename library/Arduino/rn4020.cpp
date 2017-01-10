#include "rn4020.h"
#include "ble2_hw.h"

#define DEBUG_LEVEL DEBUG_ALL

#if defined(ARDUINO_AVR_PROTRINKET3FTDI) || defined(ARDUINO_AVR_PROTRINKET3)
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
        sPortDebug->print("\r\nTX: ");
        sPortDebug->println(_data);
    }
#endif
}

rn4020::rn4020(HardwareSerial &s, byte pinWake_sw, byte pinBtActive, byte pinWake_hw, byte pinEnPwr):
    _pinWake_sw_7(pinWake_sw),
    _pinWake_hw_15(pinWake_hw),
    _pinEnPwr(pinEnPwr),
    _pinActive_12(pinBtActive),
    _ftConnectionStateChanged(0),
    _ftAdvertisementReceived(0),
    _ftBondingEvent(0),
    _ftCharacteristicWritten(0),
    _ftPasscodeGenerated(0)
{
    sPort=&s;
    sPortDebug=sw;
    ble2_hal_init();
}

bool rn4020::doAddCharacteristic(btCharacteristic* bt)
{
    //Set characteristics of the created service:
    ble2_set_private_characteristics(bt->getUuidCharacteristic(),bt->getProperty(),bt->getValueLength(), bt->getSecurityBmp());
    return waitForReply(2000,"AOK");
}

bool rn4020::doAddService(btCharacteristic* bt)
{
    ble2_set_private_service_uuid(bt->getUuidService());
    return waitForReply(2000,"AOK");
}

bool rn4020::begin(unsigned long baudrate)
{
    pinMode(_pinEnPwr, OUTPUT);
    pinMode(_pinWake_hw_15, OUTPUT);
    pinMode(_pinWake_sw_7, OUTPUT);
    pinMode(_pinActive_12, INPUT);
    //Establish UART communication with module
    cyclePower(OM_NORMAL);
    if(!waitForStartup(baudrate))
    {
        //Maybe the module is blocked or set to an unknown baudrate?
        if(!doFactoryDefault())
        {
            return false;
        }
        if(!setBaudrate(2400))
        {
            return false;
        }
        //Baudrate only becomes active after resetting the module.
        if(!doReboot(2400))
        {
            return false;
        }
    }
    return true;
}

word rn4020::getNrOfOccurrence(char* buf, char findc)
{
    char* pch=buf;
    word ctr=0;
    while(*pch)
    {
        if(*pch==findc)
        {
            ctr++;
        }
        pch++;
    }
    return ctr;
}

void rn4020::cyclePower(OPERATING_MODES om)
{
    digitalWrite(_pinWake_hw_15, LOW);
    digitalWrite(_pinWake_sw_7, LOW);
    digitalWrite(_pinEnPwr, HIGH);//module OFF
    delay(100);
    digitalWrite(_pinEnPwr, LOW);//module ON
    if(om==OM_NORMAL)
    {
        digitalWrite(_pinWake_sw_7, HIGH);
    }
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

bool rn4020::doConnecting(const char* remoteBtAddress)
{
    if(ble2_start_connection(PUBLIC_ADDRESS, remoteBtAddress)!=0)
    {
        return false;
    }
    if(!waitForReply(10000,"AOK"))
    {
        return false;
    }
    ble2_get_connection_status();
    return waitForReply(2000,"Connected");
}

bool rn4020::doDisconnect()
{
    ble2_kill_active_connection();
    return waitForReply(2000,"AOK");
}

/* If the module is in an unknown state, e.g. unknown baudrate, then it can only be reset by toggling its
 * power.  Take this into account when designing a PCB with this module.  You should be able to toggle the power
 * of the RN4020 with a GPIO of your MCU.
 * At power up, the SW_WAKE pin is held high to force full factory reset when toggling the HW_WAKE lines.
 */
bool rn4020::doFactoryDefault()
{
    setOperatingMode(OM_NORMAL);
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
    //Force reboot.  It allows us to check if the module communication is OK again.
    cyclePower(OM_NORMAL);
    //Factory default baud=115200
    return waitForStartup(115200);
}

/* Scan for undirected advertisements.
 * Bonded (but unconnected) devices will send directed advertisements (unless configured otherwise with "SR")
 * Devices sending directed advertisements will not be listed.
 */
bool rn4020::doFindRemoteDevices(bool bEnabled)
{
    if(bEnabled)
    {
        ble2_query_peripheral_devices(0,0);
    }
    else
    {
        ble2_stop_inquiry_process();
    }
    return waitForReply(2000,"AOK");
}

bool rn4020::doReboot(unsigned long baudrate)
{
    ble2_device_reboot();
    //ProTrinket 3V gets framing errors when trying to receive the "Reboot" at 115200baud.
    //waitForReply(2000,"Reboot");
    return waitForStartup(baudrate);
}

bool rn4020::doRemoveBond()
{
    ble2_remove_bonding();
    return waitForReply(2000,"AOK");
}

bool rn4020::doStopConnecting()
{
    ble2_stop_connection_process();
    return waitForReply(2000,"AOK");
}

bool rn4020::doReadRemoteCharacteristic(word handle, byte* array, byte& length)
{
    char hexarray[41];//maximum data length=20 bytes
    ble2_read_characteristic_content(handle);
    if(!waitForNrOfLines(2000,1))
    {
        return false;
    }
    if(sscanf(rxbuf, "R,%40s.", hexarray)!=1)
    {
        return false;
    }
    hex2array(hexarray, array, length);
    return true;
}


/* When adding services and characteristics to the RN4020, the handles of the existing services and characteristics
 * change.  This function updates those handles.
 */
void rn4020::doUpdateHandles(btCharacteristic** characteristicList, byte count)
{
    char* pch;
    byte state=0;
    byte ctr=0;

    //Get list of services
    ble2_list_server_services();
    if(!waitForReply(2000, "END"))
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
            for(ctr=0;ctr<count;ctr++)
            {
                //Nothing found yet, split string in lines;
                if(strstr(pch,characteristicList[ctr]->getUuidService()))
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
            if(strstr(pch, characteristicList[ctr]->getUuidCharacteristic()))
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
                    characteristicList[ctr]->setHandle(handle);

                }
                state=0;
                break;
            }
        }
        pch = strtok (NULL, "\r\n");
    }while (pch != NULL);
}


bool rn4020::doWriteRemoteCharacteristic(word handle, const byte* array, byte length)
{
    char* hexString=(char*)malloc((length<<1)+1);
    if(!hexString)
    {
        return false;
    }
    for(byte i=0;i<length;i++)
    {
        sprintf(hexString+(i<<1),"%02X", array[i]);
    }
    hexString[length<<1]='\0';
    ble2_write_characteristic_content(handle, hexString);
    return waitForReply(2000,"AOK");
}


bool rn4020::getBluetoothDeviceName(char* btName)
{
    if(!btName)
    {
        return false;
    }
    ble2_display_critical_info();
    if(!waitForNrOfLines(2000, 8))
    {
        return false;
    }
    //Parse response line by line
    char* pch = strtok (rxbuf,"\r\n");
    while (pch != NULL)
    {
        if(sscanf(pch, "Name=%20s", btName)==1)
        {
            return true;
        }
        pch = strtok (NULL, "\r\n");
    }
    return false;
}

bool rn4020::getMacAddress(byte* array, byte& length)
{
    if(!array)
    {
        return false;
    }
    char hexarray[12];
    ble2_display_critical_info();
    if(!waitForNrOfLines(2000, 8))
    {
        return false;
    }
    //Parse response line by line
    char* pch = strtok (rxbuf,"\r\n");
    while (pch != NULL)
    {
        if(sscanf(pch, "BTA=%12s", hexarray)==1)
        {
            //MAC address contains 6 bytes
            length=6;
            hex2array(hexarray, array, length);
            return true;
        }
        pch = strtok (NULL, "\r\n");
    }
}

word rn4020::getRemoteHandle(const char* service, const char* characteristic)
{
    char* pch;
    byte state=0;
    byte ctr=0;

    //Get list of services
    ble2_list_client_services();
    if(!waitForReply(5000, "END"))
    {
        return 0;
    }
    //Parse response line by line
    pch = strtok (rxbuf,"\r\n");
    do
    {
        switch(state)
        {
        case 0:
            //Check if the line contains a service
            if(strstr(pch,service))
            {
                //Service found, now looking for line with characteristic
                state=1;
                break;
            }
            break;
        case 1:
            if(strncmp(pch,"  ",2))
            {
                //String doesn't start with two spaces, so this is not a characteristic.  This is an error.
                return 0;
            }
            if(strstr(pch, characteristic))
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
                    return handle;

                }
                state=0;
                break;
            }
        }
        pch = strtok (NULL, "\r\n");
    }while (pch != NULL);
    return 0;
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
            //Reset buffer when there's an overflow
            indexRxBuf=0;
        }
    }
#if DEBUG_LEVEL >= DEBUG_ALL
    static char b=0,d=0;
    if(b==0 && d==0)
    {
        sPortDebug->print("\r\nRX: ");
    }
    if(c>27)
    {
        sPortDebug->print(c);
    }
    b=d;
    d=c;
    if(b=='\r' && d=='\n')
    {
        sPortDebug->print("\r\nRX: ");
    }
#endif
    return strstr(rxbuf,"\r\n");
}

void rn4020::hex2array(char* hexstringIn, byte* arrayOut, byte& lengthOut)
{
    if((!arrayOut) || (!hexstringIn))
    {
        return;
    }
    lengthOut=strlen(hexstringIn)>>1;
    for(byte i=0;i<lengthOut;i++)
    {
        sscanf(hexstringIn+(i<<1),"%2x", arrayOut+i);
    }
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

void rn4020::loop()
{
    word handle;
    byte length;
    char value[42];
    unsigned long passcode;
    if(!strncmp(rxbuf, "Passcode:", 9) && _ftBondingEvent)
    {
        _ftBondingEvent(BD_PASSCODE_NEEDED);
        *rxbuf='\0';    //avoid future calls
        indexRxBuf=0;
        return;
    }
    if(!gotLine())
    {
        return;
    }
    indexRxBuf=0;
    if(strstr(rxbuf, "Secured") && _ftBondingEvent)
    {
        _ftBondingEvent(BD_ESTABLISHED);
        return;
    }
    if(strstr(rxbuf, "Connected") && _ftConnectionStateChanged)
    {
        _ftConnectionStateChanged(true);
        return;
    }
    if(strstr(rxbuf, "Connection End") && _ftConnectionStateChanged)
    {
        _ftConnectionStateChanged(false);
        return;
    }
    if(sscanf(rxbuf, "WV,%04x,%32s ", &handle,value)==2 && _ftCharacteristicWritten)
    {
        byte array[17];
        hex2array(value, array, length);
        _ftCharacteristicWritten(handle, array, length);
        return;
    }
    if(sscanf(rxbuf, "Peer Passcode:%d",&passcode)==1 && _ftPasscodeGenerated)
    {
        _ftPasscodeGenerated(passcode);
        return;
    }
    //If no matching string found
#if DEBUG_LEVEL >= DEBUG_ALL
    sPortDebug->print("Unknown input: ");
    sPortDebug->println(rxbuf);
    byte i=0;
    while(rxbuf[i])
    {
        sPortDebug->print(rxbuf[i++], HEX);
        sPortDebug->print(" ");
    }
#endif
}

//The format of the scan result is:
//<BTADDR>    ,<PRIVATE>,<BTName>      ,<UUID>                          ,<RSSI>
//-----------------------------------------------------------------------------
//001EC01CFD32,0        ,              ,F1A879125950479CA5E5B6CC81CD0502,-41
//001EC01D03EA,0        ,akaikqsdfj03EA,                                ,-3A
bool rn4020::parseAdvertisement(char* buffer)
{
    return false;
}

bool rn4020::doRemovePrivateCharacteristics()
{
    ble2_private_service_clear_all();
    return waitForReply(2000,"AOK");
}

void rn4020::setAdvertisementListener(void(*ftAdvertisementReceived)(ADVERTISEMENT*))
{
    _ftAdvertisementReceived=ftAdvertisementReceived;
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

bool rn4020::setBluetoothDeviceName(const char* btName)
{
    ble2_set_device_bluetooth_name(btName);
    return waitForReply(2000,"AOK");
}

void rn4020::setBondingListener(void (*ftBonding)(BONDING_MODES bd))
{
    _ftBondingEvent=ftBonding;
}

void rn4020::setBondingPasscodeListener(void (*ftPasscode)(unsigned long))
{
    _ftPasscodeGenerated=ftPasscode;
}

void rn4020::setBondingPasscode(unsigned long passcode)
{
    ble2_set_passcode(passcode);
}

void rn4020::setCharacteristicWrittenListener(void(*ftCharacteristicWritten)(word, byte*, byte))
{
    _ftCharacteristicWritten=ftCharacteristicWritten;
}

void rn4020::setConnectionListener(void (*ftConnection)(bool))
{
    _ftConnectionStateChanged=ftConnection;
}

bool rn4020::setFeatures(uint32_t features)
{
    ble2_set_supported_features(features);
    return waitForReply(2000,"AOK");
}


//http://microchip.wikidot.com/ble:rn4020-power-states
bool rn4020::setOperatingMode(OPERATING_MODES om)
{
    bool bSuccess=false;
    bool bInNormalMode=false;
    switch (om) {
    case OM_NORMAL:
        digitalWrite(_pinWake_sw_7, HIGH);
        digitalWrite(_pinWake_hw_15, HIGH);
        bSuccess=isModuleActive(2000);
        digitalWrite(_pinWake_hw_15, LOW);
        return bSuccess;
    case OM_DEEP_SLEEP:
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
    case OM_DORMANT:
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

bool rn4020::setServices(uint32_t services)
{
    ble2_set_server_services(services);
    return waitForReply(2000,"AOK");
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

bool rn4020::startBonding()
{
    ble2_bond(SAVED);
    return waitForReply(2000,"AOK");
}


word rn4020::waitForNrOfLines(unsigned long ulTimeout, byte nrOfEols)
{
    unsigned long ulStartTime=millis();
    while(millis()<ulStartTime+ulTimeout && getNrOfOccurrence(rxbuf,'\n')<nrOfEols)
    {
        gotLine();
    }
    return getNrOfOccurrence(rxbuf,'\n');
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
        if(gotLine() && strstr(rxbuf, pattern))
        {
            indexRxBuf=0;
            return true;
        }
    }while(millis()<ulStartTime+uiTimeout);

    return false;
}

bool rn4020::waitForStartup(unsigned long baudrate)
{
    delay(100);//give serial port time to send its last characters
    sPort->flush();//on Nucleo, flush() simply clears buffers, it doesn't wait for TX-ing to finish.
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
