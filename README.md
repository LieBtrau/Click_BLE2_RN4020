# RN4020
The RN4020 is a Bluetooth low energy module from Microchip.  The big advantage of this module to most of the other modules on the market which only implement the peripheral role is that the RN4020 also implements the central role.  This opens up a lot of possibilities: You can use the RN4020 in the central role to talk to ble trackers, sensor beacons, ble tags, ...  You could also do RN4020 module to RN4020 module communication, having the advantage that you're using a standard protocol.  This means you can easily add standard bluetooth low energy hardware to your network.  You can use the RN4020 in peripheral role to communicate with your smartphone, tablet, etc. and only using very little power to do that.

# BLE2 Click
MikroElektronika has made a Click-board for this module.  Unfortunately, their MikroE BLE2-click has some shortcomings.  The BLE2-click board described in this repository has improvements over the original board:
* Connection to reset the board.  The RN4020 doesn't have a reset.  The only way to implement a reset was to do a power toggle.  By sending illegal characters, it's possible to bring the RN4020 in a non-responsive state.  The only way of recovery is a power cycle of the RN4020.
* Connection to the baseboard of the RN4020 activity status.
* WAKE_HW signal connected to the baseboard, so that the RN4020 can be brought back to factory default state.
* CMD/MLDP signal set fix low.  This might hinder full MLDP functionality.  Anyway, if you buy a bluetooth module to do MLDP, you had better spent your money on a NRF24L01+ module.

## Hardware
### BRD161224
* [Schematic](https://github.com/LieBtrau/Click_BLE2_RN4020/raw/master/Hardware/BRD161224/doc/BRD161224_R000x_Schematic.pdf)
* [Assembly Drawing](https://github.com/LieBtrau/Click_BLE2_RN4020/raw/master/Hardware/BRD161224/doc/BRD161224_R00_AssemblyDrawing_top.pdf)
* [Drill Plan](https://github.com/LieBtrau/Click_BLE2_RN4020/raw/master/Hardware/BRD161224/doc/BRD161224_R00_DrillPlan.pdf)
* [Film top](https://github.com/LieBtrau/Click_BLE2_RN4020/raw/master/Hardware/BRD161224/doc/BRD161224_R00_Film_top.pdf)
* [Film bottom](https://github.com/LieBtrau/Click_BLE2_RN4020/raw/master/Hardware/BRD161224/doc/BRD161224_R00_Film_bot.pdf)

---

## Software

* Central & peripheral functionality implemented (and tested on Nucleo STM32F103).
* Use of private characteristics

This library is based on the MikroE library which is far from complete or usable.  It only holds functions to send data to the RN4020.  There's no functionality to parse the response from the RN4020.  

