//declaration of needed libraries must be done in the ino-file of the project.
#include "Arduino.h"
#if defined(ARDUINO_AVR_PROTRINKET3FTDI) || defined(ARDUINO_AVR_PROTRINKET3)
#include <SoftwareSerial.h>
SoftwareSerial swPort(A3,A2);//RX, TX
SoftwareSerial* sw=&swPort;
#elif defined(ARDUINO_STM_NUCLEO_F103RB)
HardwareSerial* sw=&Serial;
#endif

#include "rn4020-ex1.h"

//git clone git@github.com:LieBtrau/Arduino_STM32.git ~/git/Arduino_STM32
//ln -s ~/git/Arduino_STM32/ ~/Programs/arduino-1.6.9/hardware/

#include "blecontrol.h"
#include "btcharacteristic.h"

//Build instruction:

//Adjust build.path to suit your needs.  Don't make it a subfolder of the directory where your *.ino 's are located,
//because Arduino 1.6.9 will compile these also, which will result in linking errors.
//~/Programs/arduino-1.6.9/arduino --verify --board Arduino_STM32:STM32F1:nucleo_f103rb --pref target_package=Arduino_STM32 --pref build.path=/home/ctack/build --pref target_platform=STM32F1 --pref board=nucleo_f103rb ~/Arduino/blinky_nucleo/blinky_nucleo.ino

static void alertLevelEvent(byte *value, byte& length);

//Private UUIDs have been generated by: https://www.uuidgenerator.net/version4
static btCharacteristic rfid_key("f1a87912-5950-479c-a5e5-b6cc81cd0502",        //private service
                                 "855b1938-83e2-4889-80b7-ae58fcd0e6ca",        //private characteristic
                                 btCharacteristic::WRITE_WOUT_RESP,5,           //properties+length
                                 btCharacteristic::ENCR_W);                     //security
//https://www.bluetooth.com/specifications/gatt/services
//https://www.bluetooth.com/specifications/gatt/characteristics
static btCharacteristic ias_alertLevel("1802",                                  //IAS Alert Service
                                       "2A06",                                  //Alert Level characteristic
                                       btCharacteristic::WRITE_WOUT_RESP, 1,    //properties+length
                                       btCharacteristic::NOTHING,                //security
                                       alertLevelEvent);

uint32_t ulStartTime;
bool bConnected;
btCharacteristic* _localCharacteristics[2]={&rfid_key, &ias_alertLevel};
bleControl ble(_localCharacteristics,2);

void setup() {
    ulStartTime=millis();
    while (!(*sw)) ;
    sw->begin(9600);
    sw->println("I'm ready, folk!");
    bool modeIsCentral=false;
    char peripheralMac[]="001EC01D03EA";
    ble.setEventListener(bleEvent);

    if(!ble.begin(modeIsCentral))
    {
        sw->println("RN4020 not set up");
        return;
    }

    if(modeIsCentral)
    {
        if(!ble.findUnboundPeripheral(peripheralMac))
        {
            sw->println("Remote peer not found");
            return;
        }
        if(!ble.secureConnect(peripheralMac))
        {
            return;
        }
        //Example of writing a characterictic
        if(!ble.writeRemoteCharacteristic(&ias_alertLevel,1))
        {
            return;
        }
        //Example of reading a characteristic
        byte value[20];
        byte length;
        btCharacteristic serial_number("180A",                      //Device Information Service
                                       "2A25",                      //Serial Number String
                                       btCharacteristic::READ, 20,  //properties+length
                                       btCharacteristic::NOTHING);  //security
        if(ble.readRemoteCharacteristic(&serial_number, value, length))
        {
            sw->print("Serial number of peripheral is: ");
            sw->println((char*)value);
        }
        delay(5000);
        ble.disconnect();
        delay(5000);
        ble.secureConnect(peripheralMac);
    }
    else
    {

    }
}

void loop() {
    ble.loop();
}


void bleEvent(bleControl::EVENT ev)
{
    switch(ev)
    {
    case bleControl::EV_PASSCODE_WANTED:
        sw->println("Let's guess that the passcode is 123456");
        ble.setPasscode(123456);
        break;
    case bleControl::EV_PASSCODE_GENERATED:
        sw->print("Peripheral must set PASS: ");
        sw->println(ble.getPasscode(), DEC);
        break;
    case bleControl::EV_CONNECTION_DOWN:
        sw->println("Connection down");
        bConnected=false;
        break;
    case bleControl::EV_CONNECTION_UP:
        sw->println("Connection up");
        bConnected=true;
        break;
    case bleControl::EV_CHARACTERISTIC_VALUE_CHANGED:
        sw->println("Value of some characteristic changed");
        break;
    default:
        sw->print("Unknown event: ");
        sw->println(ev, DEC);
        break;
    }
}

void alertLevelEvent(byte* value, byte &length)
{
    sw->print("Characteristic changed to: ");
    for(byte i=0;i<length;i++)
    {
        sw->print(value[i], HEX);
        sw->print(" ");
    }
    sw->println();
}
