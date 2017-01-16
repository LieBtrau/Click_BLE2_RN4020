#ifndef NUCLEO
#define NUCLEO
//declaration of needed libraries must be done in the ino-file of the project.
#include "Arduino.h"
#if defined(ARDUINO_AVR_PROTRINKET3FTDI) || defined(ARDUINO_AVR_PROTRINKET3)
/* Preferably use the USB-connection to program the Protrinket3V.  Connect the Trinket directly to your PC.  Don't use a
 * USB-hub.
 * FTDI programming uses the same UART as the RN4020 is connected on.
 * FTDI connection is easier for debugging, though.  There's no need to push that tiny reset button all the time.
 */
#include <SoftwareSerial.h>
SoftwareSerial swPort(A5,A4);//RX, TX
SoftwareSerial* sw=&swPort;
#elif defined(ARDUINO_STM_NUCLEO_F103RB)
/* git clone git@github.com:LieBtrau/Arduino_STM32.git ~/git/Arduino_STM32
 * ln -s ~/git/Arduino_STM32/ ~/Programs/arduino-1.6.9/hardware/
 *
 * Build instruction:
 * Adjust build.path to suit your needs.  Don't make it a subfolder of the directory where your *.ino 's are located,
 * because Arduino 1.6.9 will compile these also, which will result in linking errors.
 * ~/Programs/arduino-1.6.9/arduino --verify --board Arduino_STM32:STM32F1:nucleo_f103rb --pref target_package=Arduino_STM32 --pref build.path=/home/ctack/build --pref target_platform=STM32F1 --pref board=nucleo_f103rb ~/Arduino/blinky_nucleo/blinky_nucleo.ino
 */
HardwareSerial* sw=&Serial;
#endif
#include "blecontrol.h"
#include "btcharacteristic.h"



#endif // NUCLEO

