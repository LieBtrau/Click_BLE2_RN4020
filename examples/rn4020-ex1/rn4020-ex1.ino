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

//Build instruction:

//Adjust build.path to suit your needs.  Don't make it a subfolder of the directory where your *.ino 's are located,
//because Arduino 1.6.9 will compile these also, which will result in linking errors.
//~/Programs/arduino-1.6.9/arduino --verify --board Arduino_STM32:STM32F1:nucleo_f103rb --pref target_package=Arduino_STM32 --pref build.path=/home/ctack/build --pref target_platform=STM32F1 --pref board=nucleo_f103rb ~/Arduino/blinky_nucleo/blinky_nucleo.ino



uint32_t ulStartTime;

bleControl ble;

void setup() {
    ulStartTime=millis();
    while (!(*sw)) ;
    sw->begin(9600);
    sw->println("I'm ready, folk!");

    if(!ble.begin(true))
    {
        sw->println("RN4020 not set up");
        return;
    }
    if(!ble.findUnboundPeripheral("001EC01D03EA"))
    {
        sw->println("Remote peer not found");
        return;
    }
    secureConnect();
    ble.writeServiceCharacteristic(bleControl::BLE_S_IMMEDIATE_ALERT_SERVICE, bleControl::BLE_CH_ALERT_LEVEL,1);
    delay(5000);
    ble.disconnect();
    delay(5000);
    secureConnect();
}

void loop() {
    ble.loop();
}

void secureConnect()
{
    sw->println("Trying secure connect");
    bleControl::CONNECT_STATE state=bleControl::ST_NOTCONNECTED;
    do
    {
        state=ble.secureConnect("001EC01D03EA", state);
        switch(state)
        {
        case bleControl::ST_PASS_GENERATED:
            sw->print("Peripheral must set PASS: ");
            sw->println(ble.getPasscode(), DEC);
            break;
        case bleControl::ST_BONDED:
            sw->println("Bonded!");
            break;
        default:
            break;
        }
    }while(state!=bleControl::ST_NOTCONNECTED && state!=bleControl::ST_BONDED);
}
