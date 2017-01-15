#include "blecontrol.h"
#include "rn4020.h"
#include "btcharacteristic.h"

#define DEBUG_LEVEL DEBUG_ALL

#if defined(ARDUINO_AVR_PROTRINKET3FTDI) || defined(ARDUINO_AVR_PROTRINKET3)
#include <SoftwareSerial.h>
extern SoftwareSerial* sw;
rn4020 rn(Serial,3,4,5,A3);
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
static void bondingEvent(rn4020::BONDING_MODES bd);
static void advertisementEvent(rn4020::ADVERTISEMENT* adv);
static void passcodeGeneratedEvent(unsigned long passcode);
static void characteristicWrittenEvent(word handle, byte* value, byte length);

static unsigned long pass;
static volatile bool bPassReady=false;
static volatile bool bIsBonded;
static bool bIsCentral;
static void (*generateEvent)(bleControl::EVENT);

static volatile char* foundBtAddress=0;
static btCharacteristic** _localCharacteristics;
static byte _nrOfCharacteristics;


bleControl::bleControl(btCharacteristic** localCharacteristics, byte nrOfCharacteristics)
{
    bIsBonded=false;
    generateEvent=0;
    _localCharacteristics=localCharacteristics;
    _nrOfCharacteristics=nrOfCharacteristics;

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
    if(!rn.begin(2400))
    {
        return false;
    }
    rn.setConnectionListener(connectionEvent);
    rn.setBondingListener(bondingEvent);
    //Check if settings have already been done.  If yes, we don't have to set them again.
    //This is check is performed by verifying if the last setting command has finished successfully:
    if(!rn.getBluetoothDeviceName(dataname))
    {
        return false;
    }
    if(bCentral)
    {
        //Central
        if(strncmp(dataname,BT_NAME_BIKE, strlen(BT_NAME_BIKE)))
        {
            //Module not yet correctly configured

            //Services: Device Information + Battery Level services
            if(!rn.setServices(SRV_BATTERY | SRV_DEVICE_INFO))
            {
                return false;
            }
            //Central role
            //Enable authentication with Keyboard and display as IO-capabilities
            if(!rn.setFeatures(FR_CENTRAL | FR_AUTH_KEYB_DISP))
            {
                return false;
            }
            if(!rn.setBluetoothDeviceName(BT_NAME_BIKE))
            {
                return false;
            }
            //Settings only become active after resetting the module.
            rn.doReboot(2400);
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
            //Module not yet correctly configured

            //Enable authentication with Keyboard and display as IO-capabilities
            //Server only (services will only be served, no client functionalities)
            if(!rn.setFeatures(FR_AUTH_KEYB_DISP | FR_SERV_ONLY))
            {
                return false;
            }
            //Services: Device Information + Battery Level + user defined private services
            if(!rn.setServices(SRV_BATTERY | SRV_DEVICE_INFO | SRV_USR_PRIV_SERV))
            {
                return false;
            }
            if(!rn.setTxPower(0))
            {
                return false;
            }
            if(!rn.doRemovePrivateCharacteristics())
            {
                return false;
            }
            //Power must be cycled after removing private characteristics
            if(!rn.begin(2400))
            {
                return false;
            }
            for(byte i=0;i<_nrOfCharacteristics;i++)
            {
                if((!rn.doAddService(_localCharacteristics[i])) || (!rn.doAddCharacteristic(_localCharacteristics[i])))
                {
                    return false;
                }
            }
            if(!rn.setBluetoothDeviceName(BT_NAME_KEYFOB))
            {
                return false;
            }
            //Settings only become active after resetting the module.
            //Created characteristics only become available after reboot.
            if(!rn.doReboot(2400))
            {
                return false;
            }
        }
        rn.doUpdateHandles(_localCharacteristics, _nrOfCharacteristics);
        rn.setCharacteristicWrittenListener(characteristicWrittenEvent);
        //Start advertizing to make the RN4020 discoverable & connectable
        //Auto-advertizing is not used, because it doesn't allow for setting the advertisement interval
        if(!rn.doAdvertizing(true,5000))
        {
            return false;
        }
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
    bool bonded;

    //Unbound first, otherwise the bonded module can't be found by a scan
    if(rn.isBonded(bonded) && bonded)
    {
        rn.doRemoveBond();
    }
    char** macList;
    byte nrOfItems;
    //Start search
    if(!rn.doFindRemoteDevices(macList, nrOfItems, 6000))
    {
        return false;
    }
    for(byte i=0;i<nrOfItems;i++)
    {
        if(!strcmp(remoteBtAddress,macList[i]))
        {
            bFound=true;
        }
        free(macList[i]);
    }
    free(macList);
    return bFound;
}


bool bleControl::secureConnect(const char* remoteBtAddress)
{
    unsigned long ulStartTime;
    CONNECT_STATE state=ST_NOTCONNECTED;
    do
    {
        switch(state)
        {
        case ST_NOTCONNECTED:
            if(!rn.doConnecting(remoteBtAddress))
            {
                //stop connecting process
                rn.doStopConnecting();
                return false;
            }
            delay(1000);
            bPassReady=false;
            if(!rn.startBonding())
            {
                rn.doDisconnect();
                return false;
            }
            ulStartTime=millis();
            state=ST_CONNECTED;
            break;
        case ST_CONNECTED:
            if(millis()>ulStartTime+10000)
            {
                disconnect();
                return false;
            }
            loop();
            if(bPassReady)
            {
                bIsBonded=false;
                generateEvent(EV_PASSCODE_GENERATED);
                state=ST_PASSCODE_GENERATED;
                ulStartTime=millis();
            }
            break;
        case ST_PASSCODE_GENERATED:
            if(millis()>ulStartTime+10000)
            {
                disconnect();
                return false;
            }
            loop();
            if(bIsBonded)
            {
                state=ST_PROV_BONDED;
            }
            break;
        case ST_PROV_BONDED:
            if(!rn.startBonding())
            {
                rn.doDisconnect();
                return false;
            }
            state=ST_BONDED;
        }
    }while(state!=ST_BONDED);

}

unsigned long bleControl::getPasscode()
{
    return pass;
}

void bleControl::setPasscode(unsigned long pass)
{
    rn.setBondingPasscode(pass);
}

bool bleControl::writeLocalCharacteristic(btCharacteristic *bt, byte value)
{
    word handle=getLocalHandle(bt);
    if(!handle)
    {
        return false;
    }
    return rn.doWriteLocalCharacteristic(handle,&value,1);
}

bool bleControl::writeRemoteCharacteristic(btCharacteristic *bt, byte value)
{
    word handle=getRemoteHandle(bt);
    if(!handle)
    {
        return false;
    }
    return rn.doWriteRemoteCharacteristic(handle,&value,1);
}

bool bleControl::readRemoteCharacteristic(btCharacteristic *bt, byte* value, byte& length)
{
    word handle=getRemoteHandle(bt);
    if(!handle)
    {
        return false;
    }
    return rn.doReadRemoteCharacteristic(handle, value, length);
}

bool bleControl::readLocalCharacteristic(btCharacteristic *bt, byte* value, byte& length)
{
    word handle=getLocalHandle(bt);
    if(!handle)
    {
        return false;
    }
    return rn.doReadLocalCharacteristic(handle, value, length);
}


bool bleControl::getLocalMacAddress(byte* address, byte& length)
{
    return rn.getMacAddress(address, length);
}

word bleControl::getRemoteHandle(btCharacteristic* bt)
{
    word handle=bt->getHandle();
    if(handle)
    {
        return handle;
    }
    handle=rn.getRemoteHandle(bt);
    bt->setHandle(handle);
    return handle;
}

word bleControl::getLocalHandle(btCharacteristic* bt)
{
    word handle=bt->getHandle();
    if(handle)
    {
        return handle;
    }
    handle=rn.getLocalHandle(bt);
    bt->setHandle(handle);
    return handle;
}



void bleControl::setEventListener(void(*ftEventReceived)(EVENT))
{
    generateEvent=ftEventReceived;
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


void bondingEvent(rn4020::BONDING_MODES bd)
{
    switch(bd)
    {
    case rn4020::BD_ESTABLISHED:
        bIsBonded=true;
        break;
    case rn4020::BD_PASSCODE_NEEDED:
        if(generateEvent)
        {
            generateEvent(bleControl::EV_PASSCODE_WANTED);
        }
        break;
    default:
        break;
    }
}

void connectionEvent(bool bConnectionUp)
{
    if(bConnectionUp)
    {
        if(generateEvent)
        {
            generateEvent(bleControl::EV_CONNECTION_UP);
        }
    }else
    {
        if(generateEvent)
        {
            generateEvent(bleControl::EV_CONNECTION_DOWN);
        }
        if(!bIsCentral)
        {
            //After connection goes down, advertizing must be restarted or the module will no longer be connectable.
            if(!rn.doAdvertizing(true,5000))
            {
                return;
            }
        }
    }
}

void characteristicWrittenEvent(word handle, byte* value, byte length)
{
    for(byte i=0;i<_nrOfCharacteristics;i++)
    {
        if(_localCharacteristics[i]->getHandle()==handle)
        {
            _localCharacteristics[i]->callListener(value, length);
        }
    }
}


void passcodeGeneratedEvent(unsigned long passcode)
{
    pass=passcode;
    bPassReady=true;
}
