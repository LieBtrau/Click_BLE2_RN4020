#include "blecontrol.h"

#define DEBUG_LEVEL DEBUG_ALL

static void connectionEvent(bool bConnectionUp);
static void bondingEvent(rn4020::BONDING_MODES bd);
static void advertisementEvent(rn4020::ADVERTISEMENT* adv);
static void passcodeGeneratedEvent(unsigned long passcode);
static void characteristicWrittenEvent(word handle, byte* value, byte length);

namespace
{
unsigned long pass;
volatile bool bPassReady=false;
struct
{
    bool isConnected=false;
    bool isBonded=false;       //reflects RN4020 output of "Q,1"-command
    bool isSecured=false;
} status;
void (*generateEvent)(bleControl::EVENT);

volatile char* foundBtAddress=0;
btCharacteristic** _localCharacteristics=0;
byte _nrOfCharacteristics=0;
rn4020 *rn;
}

bleControl::bleControl(rn4020* prn)
{
    generateEvent=0;
    rn=prn;
}

//Set up the RN4020 module
bool bleControl::init()
{
    //Switch to 2400baud
    // + It's more reliable than 115200baud with the ProTrinket 3V.
    // + It also works when the module is in deep sleep mode.
    if(!rn->begin(2400) || !rn->isBonded(status.isBonded))
    {
        return false;
    }

    rn->setConnectionListener(connectionEvent);
    rn->setBondingListener(bondingEvent);
    return true;
}

bool bleControl::isBonded()
{
    return status.isBonded;
}

bool bleControl::isSecured()
{
    return status.isSecured;
}

bool bleControl::programPeripheral()
{
    //Enable authentication with Keyboard and display as IO-capabilities
    //Server only (services will only be served, no client functionalities)
    if(!rn->setFeatures(FR_AUTH_KEYB_DISP | FR_SERV_ONLY))
    {
        return false;
    }
    //Services: Device Information + Battery Level + user defined private services
    if(!rn->setServices(SRV_BATTERY | SRV_DEVICE_INFO | SRV_USR_PRIV_SERV))
    {
        return false;
    }
    if(!rn->setTxPower(0))
    {
        return false;
    }
    return true;
}

bool bleControl::programCentral()
{
    //Services: Device Information + Battery Level services
    if(!rn->setServices(SRV_BATTERY | SRV_DEVICE_INFO))
    {
        return false;
    }
    //Central role
    //Enable authentication with Keyboard and display as IO-capabilities
    if(!rn->setFeatures(FR_CENTRAL | FR_AUTH_KEYB_DISP))
    {
        return false;
    }
    return true;
}

bool bleControl::beginPeripheral(btCharacteristic **localCharacteristics, byte nrOfCharacteristics)
{
    //Reboot is needed to activate previously programmed settings.
    if(!rn->doReboot(2400))
    {
        return false;
    }
    _localCharacteristics=localCharacteristics;
    _nrOfCharacteristics=nrOfCharacteristics;
    rn->doUpdateHandles(_localCharacteristics, _nrOfCharacteristics);
    rn->setCharacteristicWrittenListener(characteristicWrittenEvent);
    //Start advertizing to make the RN4020 discoverable & connectable
    //Auto-advertizing is not used, because it doesn't allow for setting the advertisement interval
    if(!rn->doAdvertizing(true,5000))
    {
        return false;
    }
    return rn->setOperatingMode(rn4020::OM_DEEP_SLEEP);
}

bool bleControl::beginCentral()
{
    //Reboot is needed to activate previously programmed settings.
    if(!rn->doReboot(2400))
    {
        return false;
    }
    rn->setAdvertisementListener(advertisementEvent);
    rn->setBondingPasscodeListener(passcodeGeneratedEvent);
    return rn->setOperatingMode(rn4020::OM_NORMAL);
}

bool bleControl::unbond()
{
    if(!status.isBonded)
    {
        return true;
    }
    if(rn->doRemoveBond())
    {
        status.isBonded=false;
        return true;
    }
    return false;
}

bool bleControl::startAdvertizement(unsigned int interval_ms)
{
    //stop advertizing
    rn->doAdvertizing(false,0);
    //start new advertizement command
    return rn->doAdvertizing(true, interval_ms);
}


bool bleControl::addLocalCharacteristics(btCharacteristic **localCharacteristics, byte nrOfCharacteristics)
{
    _localCharacteristics=localCharacteristics;
    _nrOfCharacteristics=nrOfCharacteristics;

    if(!rn->doRemovePrivateCharacteristics())
    {
        return false;
    }
    //Power must be cycled after removing private characteristics
    if(!rn->begin(2400))
    {
        return false;
    }
    for(byte i=0;i<_nrOfCharacteristics;i++)
    {
        if((!rn->doAddService(_localCharacteristics[i])) || (!rn->doAddCharacteristic(_localCharacteristics[i])))
        {
            return false;
        }
    }
    //doUpdateHandles must be called after this function.
}


void bleControl::disconnect()
{
    rn->doDisconnect();
}

bool bleControl::loop()
{
    rn->loop();
}

bool bleControl::findUnboundPeripheral(const byte* remoteBtAddress)
{
    bool bFound=false;

    //Unbound first, otherwise the bonded module can't be found by a scan
    if(!unbond())
    {
        return false;
    }
    byte** macList;
    byte nrOfItems;
    //Start search
    if(!rn->doFindRemoteDevices(macList, nrOfItems, 1000))
    {
        return false;
    }
    for(byte i=0;i<nrOfItems;i++)
    {
        if(!memcmp(remoteBtAddress,macList[i],6))
        {
            bFound=true;
        }
        free(macList[i]);
    }
    free(macList);
    return bFound;
}


bool bleControl::secureConnect(const byte* remoteBtAddress)
{
    unsigned long ulStartTime;
    CONNECT_STATE state=ST_NOTCONNECTED;
    do
    {
        loop();
        switch(state)
        {
        case ST_NOTCONNECTED:
            if(!rn->startConnecting(remoteBtAddress))
            {
                return false;
            }
            ulStartTime=millis();
            state=ST_WAITING_FOR_CONNECTION;
            break;
        case ST_WAITING_FOR_CONNECTION:
            if(millis()>ulStartTime+10000)
            {
                //stop connecting process
                rn->doStopConnecting();
                return false;
            }
            if(status.isConnected)
            {
                state=ST_CONNECTED;
            }
            break;
        case ST_CONNECTED:
            //delay(1000);
            bPassReady=false;
            if(!rn->startBonding())
            {
                rn->doDisconnect();
                return false;
            }
            ulStartTime=millis();
            state=ST_START_BONDING;
            break;
        case ST_START_BONDING:
            if(millis()>ulStartTime+1000)
            {
                disconnect();
                return false;
            }
            if(bPassReady)  //establishing bond for the 1st time
            {
                generateEvent(EV_PASSCODE_GENERATED);
                state=ST_PASSCODE_GENERATED;
                ulStartTime=millis();
            }
            if(status.isBonded && status.isSecured) //re-establishing bond
            {
                state=ST_BONDED;
            }
            break;
        case ST_PASSCODE_GENERATED:
            if(millis()>ulStartTime+1000)
            {
                disconnect();
                return false;
            }
            loop();
            if(status.isBonded)
            {
                state=ST_BONDED;
            }
            break;
        case ST_BONDED:
            if(!rn->startBonding())
            {
                rn->doDisconnect();
                return false;
            }
            state=ST_SECURED;
            break;
        }
    }while(state!=ST_SECURED);

}

bool bleControl::getBluetoothDeviceName(char* btName)
{
    return rn->getBluetoothDeviceName(btName);
}

bool bleControl::setBluetoothDeviceName(const char* btName)
{
    return rn->setBluetoothDeviceName(btName);
}


unsigned long bleControl::getPasscode()
{
    return pass;
}

void bleControl::setPasscode(unsigned long pass)
{
    rn->setBondingPasscode(pass);
}

bool bleControl::writeLocalCharacteristic(btCharacteristic *bt, byte value)
{
    word handle=getLocalHandle(bt);
    if(!handle)
    {
        return false;
    }
    return rn->doWriteLocalCharacteristic(handle,&value,1);
}

bool bleControl::writeRemoteCharacteristic(btCharacteristic *bt, byte value)
{
    word handle=getRemoteHandle(bt);
    if(!handle)
    {
        return false;
    }
    return rn->doWriteRemoteCharacteristic(handle,&value,1);
}

bool bleControl::readRemoteCharacteristic(btCharacteristic *bt, byte* value, byte& length)
{
    word handle=getRemoteHandle(bt);
    if(!handle)
    {
        return false;
    }
    return rn->doReadRemoteCharacteristic(handle, value, length);
}

bool bleControl::readLocalCharacteristic(btCharacteristic *bt, byte* value, byte& length)
{
    word handle=getLocalHandle(bt);
    if(!handle)
    {
        return false;
    }
    return rn->doReadLocalCharacteristic(handle, value, length);
}


bool bleControl::getLocalMacAddress(byte* address, byte& length)
{
    return rn->getMacAddress(address, length);
}

word bleControl::getRemoteHandle(btCharacteristic* bt)
{
    word handle=bt->getHandle();
    if(handle)
    {
        return handle;
    }
    handle=rn->getRemoteHandle(bt);
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
    handle=rn->getLocalHandle(bt);
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
    case rn4020::BD_BONDED:
        status.isBonded=true;
        if(generateEvent)
        {
            generateEvent(bleControl::EV_BONDING_BONDED);
        }
        break;
    case rn4020::BD_SECURED:
        status.isSecured=true;
        if(generateEvent)
        {
            generateEvent(bleControl::EV_BONDING_SECURED);
        }
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
    status.isConnected=bConnectionUp;
    status.isSecured=false;
    if(!generateEvent)
    {
        return;
    }
    generateEvent(bConnectionUp ? bleControl::EV_CONNECTION_UP : bleControl::EV_CONNECTION_DOWN);
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
