#include "blecontrol.h"
#include "rn4020.h"
#include "btcharacteristic.h"

#if defined(ARDUINO_AVR_PROTRINKET3FTDI) || defined(ARDUINO_AVR_PROTRINKET3)
#include <SoftwareSerial.h>
extern SoftwareSerial* sw;
rn4020 rn(Serial,3,4,5,A3);
SoftwareSerial* sPortDebug;
/*Connections between ProTrinket3V and RN4020
 * RN4020.1 -> GND
 * RN4020.5 -> RX
 * RN4020.6 -> TX
 * RN4020.7 -> 3
 * RN4020.12 -> 4
 * RN4020.15 -> 5
 * RN4020.PWREN -> A3
 * RN4020.3V3 -> 3V
 */
#elif defined(ARDUINO_STM_NUCLEO_F103RB)
/*Connections between Nucleo and RN4020
 * RN4020.1 -> GND
 * RN4020.5 -> D2
 * RN4020.6 -> D8
 * RN4020.7 -> D3
 * RN4020.12 -> D4
 * RN4020.15 -> D5
 * RN4020.PWREN -> D6
 * RN4020.3V3 -> 3V3
 */
rn4020 rn(Serial1, 3, 4, 5, 6);
extern HardwareSerial* sw;
#endif

static void connectionEvent(bool bConnectionUp);
static void alertLevelEvent(char* value, byte& length);
static void bondingEvent(rn4020::BONDING_MODES bd);
static void advertisementEvent(rn4020::ADVERTISEMENT* adv);
static void passcodeGeneratedEvent(unsigned long passcode);
static unsigned long pass;
static volatile bool bPassReady=false;
static volatile bool bIsBonded;
static bool bIsCentral;

//https://www.bluetooth.com/specifications/gatt/services
//https://www.bluetooth.com/specifications/gatt/characteristics
static btCharacteristic ias_alertLevel("1802",                                  //IAS Alert Service
                                       "2A06",                                  //Alert Level characteristic
                                       btCharacteristic::WRITE_WOUT_RESP, 1,    //properties+length
                                       btCharacteristic::NOTHING);              //security
//Private UUIDs have been generated by: https://www.uuidgenerator.net/version4
static btCharacteristic rfid_key("f1a87912-5950-479c-a5e5-b6cc81cd0502",        //private service
                                 "855b1938-83e2-4889-80b7-ae58fcd0e6ca",        //private characteristic
                                 btCharacteristic::WRITE_WOUT_RESP,5,           //properties+length
                                 btCharacteristic::ENCR_W);                     //security
static volatile char* foundBtAddress=0;

bleControl::bleControl()
{
    bIsBonded=false;
}

//Set up the RN4020 module
bool bleControl::begin(bool bCentral)
{
    char dataname[20];
    const char BT_NAME_KEYFOB[]="AiakosKeyFob";
    const char BT_NAME_BIKE[]="AiakosBike";

    bIsCentral=bCentral;
    //Switch to 2400baud
    // + It's more reliable than 115200baud with the ProTrinket 3V.
    // + It also works when the module is in deep sleep mode.
    if(!rn.begin(2400, bCentral ? rn4020::RL_CENTRAL : rn4020::RL_PERIPHERAL))
    {
        return false;
    }
    rn.setConnectionListener(connectionEvent);
    rn.setBondingListener(bondingEvent);
    //Check if settings have already been done.  If yes, we don't have to set them again.
    //This is check is performed by checking if the last setting command has finished successfully:
    if(!rn.getBluetoothDeviceName(dataname))
    {
        return false;
    }
    if(bCentral)
    {
        //Central
        if(strncmp(dataname,BT_NAME_BIKE, strlen(BT_NAME_BIKE)))
        {
            if(!rn.setBluetoothDeviceName(BT_NAME_BIKE))
            {
                return false;
            }
            //Settings only becomes active after resetting the module.
            if(!rn.doReboot(2400))
            {
                return false;
            }
        }
        rn.setAdvertisementListener(advertisementEvent);
        rn.setBondingPasscodeListener(passcodeGeneratedEvent);
        return rn.setOperatingMode(rn4020::OM_NORMAL);
    }else
    {
        //Peripheral
        if(strncmp(dataname,BT_NAME_KEYFOB, strlen(BT_NAME_KEYFOB)))
        {
            if(!rn.setTxPower(0))
            {
                return false;
            }
            if(!rn.removePrivateCharacteristics())
            {
                return false;
            }
            //Power must be cycled after removing private characteristics
            if(!rn.begin(2400, rn4020::RL_PERIPHERAL))
            {
                return false;
            }
            if(!rn.addCharacteristic(&rfid_key))
            {
                return false;
            }
            ias_alertLevel.setListener(alertLevelEvent);
            if(!rn.addCharacteristic(&ias_alertLevel))
            {
                return false;
            }
            if(!rn.setBluetoothDeviceName(BT_NAME_KEYFOB))
            {
                return false;
            }
            //Normally reboot needed to make changes take effect, but that has already been done by adding characteristics.
        }
        //Start advertizing to make the RN4020 discoverable & connectable
        //Auto-advertizing is not used, because it doesn't allow for setting the advertisement interval
        if(!rn.doAdvertizing(true,5000))
        {
            return false;
        }
        //        byte mac[6];
        //        byte macLength;
        //        if(!rn.getMacAddress(mac, macLength))
        //        {
        //            return false;
        //        }
        //        for(byte i=0;i<macLength;i++)
        //        {
        //            Serial.println(mac[i], HEX);
        //        }
        return rn.setOperatingMode(rn4020::OM_DEEP_SLEEP);
    }

}

void bleControl::disconnect()
{
    rn.doDisconnect();
}

bool bleControl::loop()
{
    rn.loop();
}

bool bleControl::findUnboundPeripheral(const char* remoteBtAddress)
{
    bool bFound=false;
    //Start search
    if(!rn.doFindRemoteDevices(true))
    {
        return false;
    }
    //Polling loop
    unsigned long ulStartTime=millis();
    while(millis()<ulStartTime+6000)
    {
        loop();
        if(!strcmp(remoteBtAddress, (char*)foundBtAddress))
        {
            bFound=true;
            break;
        }
    }
    //Stop searching
    if(!rn.doFindRemoteDevices(false))
    {
        return false;
    }
    return bFound;
}

bleControl::CONNECT_STATE bleControl::secureConnect(const char* remoteBtAddress, CONNECT_STATE state)
{
    unsigned long ulStartTime;
    switch(state)
    {
    case ST_NOTCONNECTED:
        if(!rn.doConnecting(remoteBtAddress))
        {
            return ST_NOTCONNECTED;
        }
        delay(1000);
        if(!rn.startBonding())
        {
            rn.doDisconnect();
            return ST_NOTCONNECTED;
        }
        ulStartTime=millis();
        while(millis()<ulStartTime+10000)
        {
            loop();
            if(bPassReady)
            {
                bPassReady=false;
                return ST_PASS_GENERATED;
            }
            if(bIsBonded)
            {
                return ST_BONDED;
            }
        }
        disconnect();
        return ST_NOTCONNECTED;
    case ST_PASS_GENERATED:
        ulStartTime=millis();
        while(millis()<ulStartTime+10000)
        {
            loop();
            if(bIsBonded)
            {
                return ST_BONDED;
            }
        }
        disconnect();
        return ST_NOTCONNECTED;
    case ST_BONDED:
        return ST_BONDED;
    }
}

unsigned long bleControl::getPasscode()
{
    return pass;
}


bool bleControl::getLocalMacAddress(byte* address, byte& length)
{
    return rn.getMacAddress(address, length);
}

void advertisementEvent(rn4020::ADVERTISEMENT* adv)
{
    foundBtAddress=(char*)malloc(strlen(adv->btAddress)+1);
    if(!foundBtAddress)
    {
        return;
    }
    strcpy((char*)foundBtAddress, adv->btAddress);
}

void alertLevelEvent(char* value, byte &length)
{
    for(byte i=0;i<length;i++)
    {
        sw->print(value[i], HEX);
        sw->print(" ");
    }
    sw->println();
}

void bondingEvent(rn4020::BONDING_MODES bd)
{
    switch(bd)
    {
    case rn4020::BD_ESTABLISHED:
        bIsBonded=true;
        break;
    case rn4020::BD_PASSCODE_NEEDED:
        rn.setBondingPasscode("123456");
        break;
    default:
        break;
    }
}

void connectionEvent(bool bConnectionUp)
{
    if(bConnectionUp)
    {
        sw->println("up");
    }else
    {
        sw->println("down");
        if(!bIsCentral)
        {
            //After connection went down, advertizing must be restarted or the module will no longer be connectable.
            if(!rn.doAdvertizing(true,5000))
            {
                return;
            }
        }
    }
}

void passcodeGeneratedEvent(unsigned long passcode)
{
    pass=passcode;
    bPassReady=true;
}
