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

volatile char* foundBtAddress;
btCharacteristic** _localCharacteristics;
byte _nrOfCharacteristics;
rn4020 *rn;
const int CONNECTION_TIMEOUT=15000;
}

bleControl::bleControl(rn4020* prn)
{
    generateEvent=0;
    rn=prn;
    initialized=false;
}

//Set up the RN4020 module
//2400baud:
// + It's more reliable than 115200baud with the ProTrinket 3V.
// + It also works when the module is in deep sleep mode.
bool bleControl::init(unsigned long baud)
{
    baudrate=baud;
    if(!rn->begin(baudrate))
    {
        return false;
    }
    //For some unknown reason, the command only succeeds the 2nd time.
    if(!rn->isBonded(status.isBonded) && !rn->isBonded(status.isBonded))
    {
        return false;
    }
    rn->setConnectionListener(connectionEvent);
    rn->setBondingListener(bondingEvent);
    initialized=true;
    return true;
}

bool bleControl::isBondedTo(byte* mac)
{
    bool isBonded;
    return !rn->isBonded(isBonded, mac) || !isBonded ? false : true;
}

bool bleControl::isBonded()
{
    return status.isBonded;
}

bool bleControl::isConnected()
{
    status.isConnected=rn->isConnected();
    return status.isConnected;
}

bool bleControl::isInitialized()
{
    return initialized;
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
    //weakest transmission power in order to limit communication range
    return rn->setTxPower(0);
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
    //Actually this can be left to 0x0000 0000, authentication will work the same way.
    if(!rn->setFeatures(FR_CENTRAL | FR_AUTH_KEYB_DISP))
    {
        return false;
    }
    return true;
}

bool bleControl::beginPeripheral(btCharacteristic *localCharacteristics[], byte nrOfChrs)
{
    _localCharacteristics=localCharacteristics;
    _nrOfCharacteristics=nrOfChrs;
    rn->setCharacteristicWrittenListener(characteristicWrittenEvent);
    return true;
}

bool bleControl::beginCentral()
{
    //Reboot is needed to activate previously programmed settings.
    if(!rn->doReboot(baudrate))
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

//set interval to 0 to disable advertizements
bool bleControl::setAdvertizement(unsigned int interval_ms)
{
    //stop advertizing
    if(!rn->setAdvertizement(0))
    {
        return false;
    }
    //start new advertizement command
    return (interval_ms ? rn->setAdvertizement(interval_ms) : true);
}


bool bleControl::addLocalCharacteristics(btCharacteristic *localCharacteristics[], byte nrOfChrs)
{
    _localCharacteristics=localCharacteristics;
    _nrOfCharacteristics=nrOfChrs;

    if(!rn->doRemovePrivateCharacteristics())
    {
        return false;
    }
    //Module must be reset after removing characteristics
    if(!rn->doReboot(baudrate))
    {
        return false;
    }
    for(byte i=0;i<nrOfChrs;i++)
    {
        if((!rn->doAddService(_localCharacteristics[i])) || (!rn->doAddCharacteristic(_localCharacteristics[i])))
        {
            return false;
        }
    }
    rn->doUpdateHandles(_localCharacteristics, nrOfChrs);
    return true;
}


bool bleControl::disconnect(unsigned long timeout)
{
    rn->doDisconnect();
    unsigned long startTime=millis();
    while(status.isConnected && millis() < startTime + timeout)
    {
        loop();
    }
    return !isConnected();
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
    byte mac[6];
    CONNECT_STATE state;

    if(rn->isConnectedTo(mac))
    {
        if(memcmp(mac,remoteBtAddress, sizeof(mac)))
        {
            rn->doDisconnect();
            state=ST_NOTCONNECTED;
        }
        else
        {
            state=ST_CONNECTED;
        }
    }else
    {
        state=ST_NOTCONNECTED;
    }
    do
    {
        loop();
        switch(state)
        {
        case ST_NOTCONNECTED:
            status.isConnected=false;
            if(!rn->startConnecting(remoteBtAddress))
            {
                return false;
            }
            ulStartTime=millis();
            state=ST_WAITING_FOR_CONNECTION;
            break;
        case ST_WAITING_FOR_CONNECTION:
            if(millis()>ulStartTime+CONNECTION_TIMEOUT)
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
                debug_println("Bonding failed.");
		rn->doDisconnect();
                return false;
            }
            ulStartTime=millis();
            state=ST_START_BONDING;
            break;
        case ST_START_BONDING:
            if(millis()>ulStartTime+2000)
            {
                debug_println("Too late");
		disconnect(3000);
                return false;
            }
            if(status.isBonded && status.isSecured) //re-establishing bond
            {
                state=ST_BONDED;
            }
            if(bPassReady)  //establishing bond for the 1st time
            {
                if(status.isBonded)
                {
                    //Generating passcode while bonded already?  Central must have lost power.
                    //Repairing needed.
                    debug_println("Repairing needed.");
                    //The disconnect will only be executed after the 30s timeout of the password input.
		    disconnect(3000);
                    return false;
                }
                generateEvent(EV_PASSCODE_GENERATED);
                state=ST_PASSCODE_GENERATED;
                ulStartTime=millis();
            }
            break;
        case ST_PASSCODE_GENERATED:
            if(millis()>ulStartTime+1000)
            {
		disconnect(3000);
                return false;
            }
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

bool bleControl::writeLocalCharacteristic(btCharacteristic *bt, byte *value, byte length)
{
    word handle=getLocalHandle(bt);
    if(!handle)
    {
        return false;
    }
    return rn->doWriteLocalCharacteristic(handle, value, length);
}

bool bleControl::writeRemoteCharacteristic(btCharacteristic *bt, byte *value, byte length)
{
    if( bt->getValueLength()<length ||
            !(bt->getProperty() & (btCharacteristic::WRITE | btCharacteristic::WRITE_WOUT_RESP)) )
    {
        return false;
    }
    word handle=getRemoteHandle(bt);
    if(!handle)
    {
        return false;
    }
    return rn->doWriteRemoteCharacteristic(handle, value, length);
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

bool bleControl::sleep()
{
    return rn->setOperatingMode(rn4020::OM_DEEP_SLEEP);
}

bool bleControl::reboot()
{
    return rn->doReboot(baudrate);
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
        debug_println("Bonded event");
        status.isBonded=true;
        if(generateEvent)
        {
            generateEvent(bleControl::EV_BONDING_BONDED);
        }
        break;
    case rn4020::BD_SECURED:
        debug_println("Secured event");
        status.isSecured=true;
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
    debug_println("pass generated");
}
