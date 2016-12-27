# BLE2 Click
The original MikroE BLE2-click has some shortcomings.  The BLE2-click board described in this repository has some improvements over the original board:
* Connection to reset the board.  The RN4020 doesn't have a reset.  The only way to implement a reset was to do a power toggle.  By sending illegal characters, it's possible to bring the RN4020 in a non-responsive state.  The only way of recovery is a power cycle of the RN4020.
* Connection to the baseboard of the RN4020 activity status.
* WAKE_HW signal connected to the baseboard, so that the RN4020 can be brought back to factory default state.
* CMD/MLDP signal set fix low.  This might hinder full MLDP functionality.  Anyway, if you buy a bluetooth module to do MLDP, you had better spent your money on a NRF24L01+ module.

## Hardware
### BRD161224
* [Schematic](https://drive.google.com/open?id=0B5_mAlpV8IjvNDhhZ1VFa0pTalE)
* [Assembly Drawing](https://drive.google.com/open?id=0B5_mAlpV8IjvUFloZ0N6MFRrYnM)
* [Drill Plan](https://drive.google.com/open?id=0B5_mAlpV8IjvWHlsd1NfR2djTkU)
* [Films](https://drive.google.com/open?id=0B5_mAlpV8IjvTHRYaU1Lb0hmRk0)

---

## Software

State: In progress

The original MikroE library is far from complete.  It only holds functions to send data to the RN4020.  There's no functionality to parse the response from the RN4020.  
This library will include a central/peripheral application using private characteristics, implemented for Arduino.
