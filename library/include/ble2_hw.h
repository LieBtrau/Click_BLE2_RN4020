/****************************************************************************
* Title                 :   BLE 2 Click HW layer
* Filename              :   ble2_hw.h
* Author                :   VM
* Origin Date           :   11/05/2016
* Notes                 :   None
*****************************************************************************/
/**************************CHANGE LIST **************************************
*
*    Date    Software Version    Initials       Description
*  11/05/16    1.0.0               VM       Interface Created.
*
*****************************************************************************/
/**
 * @file ble2_hw.h
 *
 * @brief HW layer for the Mikroelektronika's BLE 2 click board.
 */

/**
 * @mainpage
 * ### General Description ###
 * BLE2 click™ is a simple solution for adding Bluetooth 4.1 (also known as Bluetooth Low Energy, BLE, Bluetooth Smart) to your design.
 * It features the <a href="http://ww1.microchip.com/downloads/en/DeviceDoc/50002279A.pdf">RN4020</a> module from MicroChip that has the complete Bluetooth stack, and can act both as a client or a server.
 * Additionally, the RN4020 module supports 13 public profiles and 17 public services, including the MicroChip Low-energy Data Profile (MLDP).
 * BLE 2 communicates with the target board microcontroller through mikroBUS™ RX, TX and AN (here, CMD), PWM (con.), and RST (wake) lines.
 * The board is designed to use a 3.3V power supply only.
 * @par Example on STM :
 * @ref example_arm
 *
 * @par Example on PIC32 :
 * @ref example_pic32
 *
 * @par Example on FT90 :
 * @ref example_ft90
 */


#ifndef BLE2_HW_H
#define BLE2_HW_H
/******************************************************************************
* Includes
*******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include "ble2_hal.h"
/******************************************************************************
* Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Configuration Constants
*******************************************************************************/

/******************************************************************************
* Macros
*******************************************************************************/

/******************************************************************************
* Typedefs
*******************************************************************************/

typedef enum
{
    BR_2400 = 0,
    BR_9600,
    BR_19200,
    BR_38400,
    BR_115200,
    BR_230400,
    BR_460800,
    BR_921600
}baud_rate_t;

typedef enum
{
    PWR_MIN19_1dBm,
    PWR_MIN15_1dBm,
    PWR_MIN10_9dBm,
    PWR_MIN6_9dBm,
    PWR_MIN2_5dBm,
    PWR_1_6dBm,
    PWR_5_8dBm,
    PWR_7_5
}tx_pwr_t;

typedef enum
{
    RESET_SOME = 1,
    RESET_ALL
}reset_parameter_t;


typedef enum
{
    NOT_SAVED  = 0,
    SAVED,
}bond_saving_t;

typedef enum
{
    PUBLIC_ADDRESS = 0,
    RANDOM_ADDRESS,
}mac_address_t;

typedef enum
{
    NOTIFICATION_STOP = 0,
    NOTIFICATION_START = 1
}configuration_t;

typedef enum
{
    NO_SECURITY = 0,
    OTA_ENCRYPTED,
    OTA_AUTHENTICATED,
}security_t;

typedef enum
{
    PW_ON = 0,
    TIMER1_t,
    TIMER2_t,
    CONN,
    DISCON,
    PIOL,
    PIOH,
    ALERTH,
    ALERTL,
    ALERTO
}debug_event_t;

typedef enum
{
    FW_UART = 1,
    FW_OTA,
}fw_update_t;
/******************************************************************************
* Variables
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/
#ifdef __cplusplus
extern "C"{
#endif

/**
 * @brief
 * Sets the serialized Bluetooth-friendly name of the device, where the string is up to 15 alphanumeric characters.
 * @param device_name
 * Desired bluetooth name
 */
void ble2_set_device_bluetooth_name (const char* device_name);

/**
 * @brief
 * Sets the baud rate of the UART communication
 * @param baud_rate
 * desired baud rate, from 2400 to 921K
 * @return
 * 0 if succesful, -1 if invalid baud rate passed
 */
int8_t ble2_set_baud_rate (baud_rate_t baud_rate);

int8_t ble2_set_transmission_power(tx_pwr_t pwr);
/**
 * @brief
 * Sets the value of the firmware revision characteristic in the Device Information Service.
 * @param fw_value
 * Desired firmware revision value
 * @return
 * 0 if succesful, -1 if falied
 */
int8_t ble2_set_firmware_rev_value(char* fw_value);
/**
 * @brief
 * Sets the value of the hardware revision characteristics in the Device Information Service.
 * @param hw_value
 * Desired hw revision value
 * @return
 * 0 if successful, -1 if failed
 */
int8_t ble2_set_hw_rev_value(char* hw_value);
/**
 * @brief
 * Sets the value of the model characteristics in the Device Information Service
 * @param model_value
 * Desired model value
 * @return
 * 0 if sucessful, -1 if failed
 */
int8_t ble2_set_model_value(char* model_value);
/**
 * @brief
 * sets the value of the manufacturer name characteristics in the Device Information Service.
 * @param name_value
 * Desired name
 * @return
 * 0 if sucessful, -1 if failed
 */
int8_t ble2_set_manufacturer_name_value (char* name_value);
/**
 * @brief
 * Sets the value of the software revision characteristics in the Device Information Service.
 * @param sw_value
 * Desired sw revision value
 * @return
 * 0 if sucessful, -1 if failed
 */
int8_t ble2_set_sw_revision_value (char* sw_value);
/**
 * @brief
 * Sets the value of the serial number characteristics in the Device Information Service.
 * @param serial_value
 * Desired serial number value
 * @return
 * 0 if sucessful, -1 if failed
 */
int8_t ble2_set_serial_number_value (char* serial_value);
/**
 * @brief
 * This command resets the configurations to the factory default at the next reboot.
 * @param reset
 * When the input parameter is ‘1’, a majority of the settings will be restored to the
 * factory default, but some settings, such as device name, device info, script and
 * private services, stay the same. When the input parameter is ‘2’, all parameters are
 * restored to factory default.
 * @return
 * 0 if succesful, -1 if failed
 */
int8_t ble2_reset_to_factory_default(reset_parameter_t reset);
/**
 * @brief
 * sets the device name, where <string> is up to 20 alphanumeric characters.
 * @param name
 * Desired name
 * @return
 * 0 if successful, -1 if failed
 */
int8_t ble2_set_device_name(char* name);
/**
 * @brief
 * Sets the supported features of current RN4020 module.
 * @param features
 * 32-bit bitmap that indicates features to be supported. After changing the features, a reboot is necessary to make the changes effective.
 */
void ble2_set_supported_features(uint32_t features);
/**
 * @brief
 * Sets the services supported by the device in a server role
 * @param services
 * 32-bit bitmap that indicates the services supported as a server. Once the service bitmap is modified, the device must reboot to make the new services effective.
 */
void ble2_set_server_services(uint32_t services);
/**
 * @brief
 * Sets the initial connection parameters for future connections
 * @param interval
 * Desired connection interval
 * @param latency
 * Desired connection latency
 * @param timeout
 * Desired connection timeout
 * @return
 */
int8_t ble2_set_initial_connection_params (uint16_t interval, uint16_t latency, uint16_t timeout);
/**
 * @brief
 * Toggles the local echo on and off. If the “+” command is sent in Command mode, all typed characters are echoed to the output. Entering the “+”
 * command again will turn local echo off
 */
void ble2_trigger_echo();
/**
 * @brief
 * Starts advertisement. When the device acts in a
 * broadcaster role, the advertisement is an undirected, unconnectable, manufacturer-specific broadcast message.
 * When the device acts in a peripheral role and it is not bonded, the advertisement is
 * ndirected connectable, which means it is discoverable by all BTLE central devices.
 * When the device is bonded, the advertisement is directed if the no_direct_adv bit is
 * cleared using the "set supported features" command; Otherwise, the advertisement is undirected if the
 * no_direct_adv bit is set. When direct advertisement is used, it is directed to the
 * bonded device so that other BTLE devices are not heard.
 * @param interval
 * Desired advertising interval in miliseconds
 * @param window_time
 * Total advertisment window in miliseconds
 */
void ble2_start_advertisement(uint16_t interval, uint16_t window_time);
/**
 * @brief
 * Secures the connection and bond two connected devices.
 * @param parameter
 * If input parameter is ‘1’, the connection will be
 * secured and the peer device remembered. In this situation, the two devices are
 * considered bonded. If the input parameter is ‘0’, the connection is secured; however,
 * the peer device is not saved into NVM. In this situation, the connection is not bonded.
 */
void ble2_bond(bond_saving_t parameter);
/**
 * @brief
 * Displays critical information about the current device over the UART.
 */
void ble2_display_critical_info();
/**
 * @brief
 * Starts the process to establish a connection with a peer peripheral device.
 * @param mac_address_type
 *  The MAC address type is either ‘0’ for public address or ‘1’ for a random address.
 * @param mac_address
 * 6-byte MAC address
 * @return
 * 0 if sucessful, -1 if failed
 */
int8_t ble2_start_connection(mac_address_t mac_address_type,char* mac_address );
/**
 * @brief
 * Used to query the peripheral devices before establishing a connection.
 * @param scan_interval
 * Desired scanning interval in miliseconds
 * @param scan_window
 * Desired scanning window in miliseconds
 */
void ble2_query_peripheral_devices(uint16_t scan_interval, uint16_t scan_window);
/**
 * @brief
 * Used to receive broadcast messages.
 * @param scan_interval
 * Scanning interval for receiving broadcast messages
 * @param scan_window
 * Scanning window for receiving broadcast messages
 * @return
 * 0 if succesful, -1 if failed
 */
void ble2_receive_broadcast_messages(uint16_t scan_interval, uint16_t scan_window);
/**
 * @brief
 * Places the device into or out of an observer role.
 * @param enable
 * True - enables observer role, false - disables ovserver role
 */
void ble2_observer_role_enable(bool enable);
/**
 * @brief
 * Used to disconnect the active BTLE link.
 *
 */
void ble2_kill_active_connection();
/**
 * @brief
 * Used to obtain the signal strength of the last communication with the peer device.
 *
 */
void ble2_get_signal_strength();
/**
 * @brief
 * Places the RN4020 module into a broadcaster role and sets the advertisement content
 * @param advertisment_content
 * Desired avertisment content for broadcasting
 * @return
 * 0 if successful, -1 if failed
 */
int8_t ble2_init_broadcast(uint32_t advertisment_content);
/**
 * @brief
 * places the module into a Dormant mode that consumes very little power, and can be issued by either a central or peripheral device
 */
void ble2_dormant_mode_enable();
/**
 * @brief
 *  returns the Bluetooth connection status <BT address>,<0-1>. The first parameter is the Bluetooth address of the remote device.
 *  The second parameter indicates whether the address is private/random (1) or public (0).
 */
void ble2_get_connection_status();
/**
 * @brief
 *  returns the bonding state of the RN4020 module <BT
 * address>,<0-1>. The first parameter is the Bluetooth address of the remote device.
 * The second parameter indicates whether the address is private (1) or public (0). If not
 * bonded, the command returns “No Bonding”.
 */
void ble2_get_bonded_status();
/**
 * @brief
 * forces a complete device reboot (similar to a power cycle). After rebooting the RN4020 module, all prior change
 * settings take effect.
 */
void ble2_device_reboot();
/**
 * @brief
 * Changes the connection parameters: interval, latency, and time-out for the current connection.
 * Command is only effective if an active connection exists when the command is issued.
 * When the command with valid parameters is issued by a peripheral device, a
 * minimum time-out interval is required between the two connection parameter update requests.
 * @param interval
 * Desired connection interval
 * @param latency
 * Desired connection latency
 * @param timeout
 * Desired connection timeout
 */
uint8_t ble2_change_connection_parameters(uint16_t interval, uint16_t latency, uint16_t timeout);
/**
 * @brief
 * Removes the existing bonding, it also changes the advertisement method.
 */
void ble2_remove_bonding();
/**
 * @brief
 * Displays the firmware version
 */
void ble2_display_firmware_version();
/**
 * @brief
 * Stops the inquiry process
 *
 */
void ble2_stop_inquiry_process();
/**
 * @brief
 * Stops receiving broadcast messages
 */
void ble2_stop_receiving_broadcast();
/**
 * @brief
 * Stops advertising.
 */
void ble2_stop_advertising();
/**
 * @brief
 * Stops the connection process.
 */
void ble2_stop_connection_process();
/**
 * @brief
 * Lists the available client services and their characteristics
 */
void ble2_list_client_services();
/**
 * @brief
 * Displays the client service with the provided UUID along with all of its characteristics
 * @param uuid
 * UUID of the client service
 */
void  ble2_list_client_services_uuid_service(char* uuid);
/**
 * @brief
 * Displays the characteristic with the provided UUID in the client service with the given UUID.
 * @param uuid_service
 * UUID of the client service
 * @param uuid_characteristic
 * UUID of of the characteristic
 */
void  ble2_list_client_services_uuid_characteristic(char* uuid_service, char* uuid_characteristic);
/**
 * @brief
 * Lists the server services and their characteristics.
 *
 */
void ble2_list_server_services();
/**
 * @brief
 * Reads the content of the characteristic of the client service from a remote device by addressing its handle
 * @param handle
 * 16-bit hexadecimal value of the handle, which corresponds to a characteristic of the client service.
 * Users can find a match between the handle and its characteristic UUID using the by listing characteristics and services.
 */
void ble2_read_characteristic_content(uint16_t handle);
/**
 * @brief
 * Writes the contents of the characteristic in the client service from a remote device by addressing its handle.
 * @param handle
 * 16-bit hexadecimal value of the handle, which corresponds to a characteristic of the client service.
 * @param content
 * Content to be written
 */
void ble2_write_characteristic_content(uint16_t handle, char* content);
/**
 * @brief
 * Reads the configuration of a characteristic in the client service from a remote device by addressing its UUID.
 * @param UUID
 * The required UUID
 */
void ble2_read_characteristic_configuration ( char* UUID );
/**
 * @brief
 * Reads the value of a characteristic in the client service from a remote device by addressing its UUID.
 * @param UUID
 * 16-bit short UUID for a public characteristic, or a 128-bit long UUID for a private characteristic
 */
void ble2_read_characteristic_content_via_UUID(char* UUID);
/**
 * @brief
 * Writes the configuration of a characteristic in the client service to a remote device by addressing its UUID.
 * @param UUID
 * 16-bit short UUID or a 128- bit long UUID) of the characteristic
 * @param configuration
 * Parameter ‘1’ starts notification or indication, depending
 * on the property of the configuration handle. Parameter ‘0’ turns off notification or
 * indication. Only characteristics with a property of notification or indication have a
 * configuration and, therefore, are addressable by this command.
 */
void ble2_write_characteristic_configuration_via_UUID(char* UUID, configuration_t configuration);
/**
 * @brief
 * Writes the value of a characteristic in the client service to a remote device by addressing its UUID.
 * @param UUID
 * 16-bit short UUID or a 128-bit long UUID of the characteristic
 * @param content
 * hexadecimal value of the contents to be written.
 */
void ble2_write_characteristic_content_via_UUID(char* UUID, char* content);
/**
 * @brief
 * Reads the contents of the characteristic of the server service on a local device by addressing its handle.
 * @param handle
 * 16-bit hexadecimal value of the handle, which corresponds to a characteristic of the server service
 */
void ble2_read_server_characteristic_value_via_handle (uint16_t handle);
/**
 * @brief
 * Writes the contents of the characteristic in the server service to a local device by addressing its handle.
 * @param handle
 * 16-bit hexadecimal value of the handle, which corresponds to a characteristic of the server service
 * @param content
 * The second parameter is the content to be written to the characteristic
 */
void ble2_write_server_characteristic_value_via_handle (uint16_t handle, char* content);
/**
 * @brief
 * Reads the value of the characteristic in the server service on a local device by addressing its UUID.
 * @param UUID
 * Hexadecimal value of the UUID of a characteristic. The UUID can be either a 16-bit short UUID for a public characteristic,
 * or a 128-bit long UUID for a private characteristic.
 */
void ble2_read_server_characteristic_value_via_UUID (char* UUID);
/**
 * @brief
 * Writes the contents of the characteristic in the server service to a local device by addressing its UUID.
 * @param UUID
 * 16-bit short UUID for a public characteristic, or a 128-bit long UUID for a private characteristic
 * @param content
 * content to be written to the characteristic.
 */
void ble2_write_server_characteristic_value_via_UUID (char* UUID, char* content);
/**
 * @brief
 * Sets the private characteristic. This command must be called after the private service UUID has been set.
 * Calling this command adds one private characteristic to the private service at a time. Calling this
 * command later will not overwrite the previous settings, but instead will add another
 * private characteristic. This command is only effective if the private service bit is set
 * @param UUID
 * 128-bit UUID for the private characteristic
 * @param bitmap_characteristic
 * 8-bit property bitmap of the characteristic
 * @param max_data_len
 * 8-bit value that indicates the maximum data size in bytes
 * that the private characteristic holds. The real data size can be smaller. The maximum
 * data size of a characteristic cannot exceed 20 bytes.
 */
uint8_t ble2_set_private_characteristics(const char* UUID, uint8_t bitmap_characteristic, uint8_t max_data_len, uint8_t bitmap_security);
/**
 * @brief
 * Sets the primary UUID filter of the private service.
 * @param UUID
 * 128-bit private UUID
 */
void ble2_set_private_service_primary_filter_uuid(char* UUID);
/**
 * @brief
 * Clears the filter
 */
void ble2_erase_private_service_primary_filter_uuid();
/**
 * @brief
 * Sets the UUID of the private service
 * @param uuid
 * 128-bit UUID for the private service.
 */
void ble2_set_private_service_uuid(const char* uuid);
/**
 * @brief
 * Clears all settings of the private service and the private characteristics.
 */
void ble2_private_service_clear_all();
/**
 * @brief
 * Places the RN4020 module into MLDP simulation mode
 * The command is only effective if all of the following conditions are met:
 *  • Central and peripheral devices have been connected
 *  • MLDP mode is enabled using the “SR” command, which takes effect after a power
 * cycle on both of the RN4020 devices
 * Once the command is issued, the RN4020 module enters MLDP mode and all
 * data through the UART will be wirelessly transmitted to the peer device. The only way
 * to exit MLDP mode is to assert CMD/MLDP low
 */
void ble2_init_mldp_simulation_mode();
/**
 * @brief
 * sets the security mode for MLDP communications
 * @param additional_security
 * If the parameter is ‘0’, no additional security is required.
 * If the parameter is ‘1’, MLDP data over the air will be encrypted. Bonding is required
 * before the MLDP service starts.
 * If the parameter is ‘2’, MLDP data over the air will be authenticated.
 * @return
 *  if successful, -1 if failed
 */
int8_t ble2_mldp_set_security_mode(security_t additional_security);
/**
 * @brief
 * Lists the current script that is loaded in the RN4020 module. After all script lines are output, the string “END” will be output to the UART.
 */
void ble2_list_current_script();
/**
 * @brief
 * Clears the script, if any, that is loaded in the RN4020.
 */
void ble2_clear_script();
/**
 * @brief
 * Stops script execution.
 */
void ble2_stop_script_execution();
/**
 * @brief
 * Starts script execution.
 */
void ble2_start_script_execution();
/**
 * @brief
 * The script starts running the corresponding event in Debugging mode.
 * When the script is running Debugging mode, all variables assigned and any ASCII
 * commands executed would be output to the UART for debugging purposes by the
 * developer.
 * @param debug_event
 * Debug events as listed in the <A HREF=http://ww1.microchip.com/downloads/en/DeviceDoc/70005191B.pdf > datasheet </A> page 72, table 2-16
 * @return
 * 0 if successful, -1 if failed
 */
int8_t ble2_start_script_debug(debug_event_t debug_event);
/**
 * @brief
 * Enters Script Input mode. When in Script Input mode, the script
 * can be input through the UART line by line terminated by either a Carriage Return (\r)
 * or a Line Feed (\n). Once all script lines are input, press the “ESC” (\x1b) key to exit
 * Script Input. The “END” status message is returned to indicated Script Input is
 * completed and Command mode is resumed.
 */
void ble2_enter_script_input_mode();
/**
 * @brief
 * Enables users to execute commands on connected
 * peer devices. The command is sent to the connected remote device, executed at the
 * remote device, and the result is sent back to the local device. Since the UART output
 * rate is usually much higher than the BLE transmission rate, if the output data exceeds the buffer size (128 bytes), the local
 * device may only receive whatever is stored in the buffer.
 * The remote command capability provides a mechanism for another Bluetooth device
 * running in Central mode to send commands to a remote RN4020 module in
 * Peripheral mode. A host device can use a remote command to gain access to the
 * remote device and access and control all of its analog or digital I/O ports. All
 * application logic is performed on the host device. Therefore, no programming or
 * application logic needs to be run on the remote device. To summarize, the remote
 * command function allows the central host to connect to any RN4020 peripheral
 * device and invoke commands.
 * @param enable
 * true - enables, false - disables
 */
void ble2_remote_command_enable(bool enable);
/**
 * @brief
 * Places the device into Device Firmware Service mode.
 * @param interface
 * If the input parameter is ‘1’, DFU mode is set for the upgrade to be handled through
 * the UART. The message “DFU” will be output and the RN4020 module waits for the
 * DFU image to be sent through the UART. The user must then stream the signed
 * Microchip RN4020 image to the UART. If a terminal emulator is used, it is
 * recommended to use a feature such as “send file” or something similar.
 * If the input parameter is ‘2’, DFU mode is set for the upgrade to occur OTA. A valid
 * BLE connection must be established before the command can be issued from
 * the device that will send the DFU image. Both ends of the connection must support
 * MLDP, and ENABLE OTA must be set.
 * @return
 * 0 if successful, -1 if failed
 */
int8_t ble2_update_firmware(fw_update_t interface);



#ifdef __cplusplus
} // extern "C"
#endif

#endif
/*** End of File **************************************************************/

/**
 * @page example_arm Example on STM32
 * @par Example created using the Easymx Pro for STM32
 * @par STM32F107
 * @code
 *
 * // Development sistem: EasyMX PRO v7
 * // MCU: STM32F107VC 72MHz
 * // BLE2 click on mikrobus socket 2
 *
 * #include "ble2_hw.h"
 * #define INIT_BLE
 * #define MAX_BUFFER_LEN        0x40
 * #define LF  0x0A
 *
 *
 * // TFT module connections
 * unsigned int TFT_DataPort at GPIOE_ODR;
 * sbit TFT_RST at GPIOE_ODR.B8;
 * sbit TFT_RS at GPIOE_ODR.B12;
 * sbit TFT_CS at GPIOE_ODR.B15;
 * sbit TFT_RD at GPIOE_ODR.B10;
 * sbit TFT_WR at GPIOE_ODR.B11;
 * sbit TFT_BLED at GPIOE_ODR.B9;
 * // End TFT module connections
 *
 * sbit RN_WAKE at GPIOC_ODR.B3;
 * sbit RN_CMD at GPIOD_ODR.B12;
 * sbit RN_CONN at GPIOA_IDR.B5;
 * sbit LED at GPIOD_ODR.B0;
 *
 *
 * uint8_t rx_buff[MAX_BUFFER_LEN];
 * char data_len,data_ready;
 * char tmr_cnt,tmr_flg;
 * char batt_level;
 * char batt_level_txt[10];
 *
 * #define TIME_OUT       5    //5 seconds
 * #define INIT_BLE
 *
 * void UART2_Interrupt() iv IVT_INT_USART2 ics ICS_AUTO {
 * char   tmp = UART2_Read();
 *    if(tmp == LF) {
 *      rx_buff[data_len] = 0;
 *      data_len++;
 *      data_ready = 1;
 *    }else{
 *      rx_buff[data_len] = tmp;
 *      data_len++;
 *    }
 * }
 *
 * //Timer2 Prescaler :1874; Preload = 63999; Actual Interrupt Time = 1sec
 *
 * void InitTimer2(){
 *   RCC_APB1ENR.TIM2EN = 1;
 *   TIM2_CR1.CEN = 0;
 *   TIM2_PSC = 1874;
 *   TIM2_ARR = 63999;
 *   NVIC_IntEnable(IVT_INT_TIM2);
 *   TIM2_DIER.UIE = 1;
 *   TIM2_CR1.CEN = 1;
 *
 * }
 *
 * void Timer2_interrupt() iv IVT_INT_TIM2 {
 *   TIM2_SR.UIF = 0;
 *   tmr_cnt++;
 *   if (tmr_cnt >= TIME_OUT) {
 *      tmr_cnt = 0;
 *      tmr_flg = 1;
 *   }
 * }
 *
 * void InitGPIO()
 * {
 *   GPIO_Digital_Output(&GPIOC_ODR, _GPIO_PINMASK_3); //PC3 - RN_WAKE
 *   GPIO_Digital_Output(&GPIOD_ODR, _GPIO_PINMASK_12 | _GPIO_PINMASK_0); //PD12 - RN_CMD
 *   GPIO_Digital_Input(&GPIOA_IDR, _GPIO_PINMASK_5);  //PA5 - RN_CONN
 *   RN_WAKE = 0;
 *   RN_CMD = 0;
 *
 * }
 *
 *
 * void MCU_Init()
 * {
 *   InitGPIO();
 *   delay_ms(500);
 *   UART2_Init_Advanced(115200, _UART_8_BIT_DATA, _UART_NOPARITY, _UART_ONE_STOPBIT, &_GPIO_MODULE_USART2_PD56);
 *   Delay_ms(100);
 *   ble2_hal_init();
 *   data_len = 0;
 *   data_ready = 0;
 *   RXNEIE_USART2_CR1_bit = 1;
 *   NVIC_IntEnable(IVT_INT_USART2);
 *   EnableInterrupts();
 * }
 *
 * void Display_Init()
 * {
 *   TFT_Init_ILI9341_8bit(320, 240);
 *   TFT_BLED = 1;
 * }
 *
 * void DrawFrame()
 * {
 *   TFT_Fill_Screen(CL_WHITE);
 *   TFT_Set_Pen(CL_BLACK, 1);
 *   TFT_Line(20, 220, 300, 220);
 *   TFT_Line(20,  46, 300,  46);
 *   TFT_Set_Font(&TFT_defaultFont, CL_RED, FO_HORIZONTAL);
 *   TFT_Write_Text("BLE2  Click  Board  Demo", 25, 14);
 *   TFT_Set_Font(&TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
 *   TFT_Write_Text("EasyMx PRO v7", 19, 223);
 *   TFT_Set_Font(&TFT_defaultFont, CL_RED, FO_HORIZONTAL);
 *   TFT_Write_Text("www.mikroe.com", 200, 223);
 *   TFT_Set_Font(&TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
 *   TFT_Write_Text("BATTERY SERVICE", 40, 60);
 *   TFT_Write_Text("BATTERY LEVEL:", 40, 100);
 *   TFT_Write_Text("RECEIVE:", 40, 120);
 * }
 *
 * void reset_buff()
 * {
 *    memset(rx_buff,0,data_len);
 *    data_len = 0;
 *    data_ready = 0;
 * }
 *
 * void Display_BatteryLevel()
 * {
 *    char txt[3];
 *    ByteToStr(batt_level,txt);
 *    TFT_Set_Pen(CL_WHITE, 10);
 *    TFT_Set_Brush(1, CL_WHITE, 0, 0, 0, 0);
 *    TFT_Rectangle(150,100,230,115);
 *    TFT_Write_Text(txt,150,100);
 *    TFT_Write_Char(0x25,180,100);
 * }
 *
 * void Display_Message()
 * {
 *    TFT_Set_Pen(CL_WHITE, 10);
 *    TFT_Set_Brush(1, CL_WHITE, 0, 0, 0, 0);
 *    TFT_Rectangle(150,120,300,140);
 *    TFT_Write_Text(rx_buff,150,120);
 * }
 *
 * char wait_response(char *value) {
 *    char result = 0;
 *    while(!data_ready);
 *    if(strstr(rx_buff,value)) {
 *      result = 1;
 *    }
 *    memset(rx_buff,0,data_len);
 *    data_len = 0;
 *    data_ready = 0;
 *    return result;
 * }
 *
 * void BLE_Init()
 * {
 *   unsigned int line_no;
 *    line_no = 10;
 *   TFT_Write_Text("Start initialization",10,line_no);
 *   RN_WAKE = 1;
 *   while (!wait_response("CMD"));                                                //Wait to response CMD
 *   line_no += 20;
 *   Delay_ms(100);
 *   ble2_reset_to_factory_default(1);                                             //Reset RN4020 to factory settings
 *   while (!wait_response("AOK"));                                                //Wait to response AOK
 *   TFT_Write_Text("Factory reset",10,line_no);
 *   line_no += 20;
 *   Delay_ms(100);
 *   TFT_Write_Text("Set module name: BLE2_Click",10,line_no);
 *   ble2_set_device_name("BLE2_Click");                                           //Set name BLE2_Click
 *   while (!wait_response("AOK"));                                                //Wait to response AOK
 *   line_no += 20;
 *   Delay_ms(100);
 *   TFT_Write_Text("Configure services",10,line_no);
 *
 *
 * //  Example:
 * //  Table: 2-6 in Datasheet
 * //  Health Thermometer + Heart Rate + Battery Services
 * //  UART_WRite_Line("SS,F0000000");
 *
 *
 *   ble2_set_server_services(40000000);                                           //Battery
 *   while (!wait_response("AOK"));                                                //Wait to response AOK
 *   line_no += 20;
 *   Delay_ms(100);
 *   TFT_Write_Text("Configure features",10,line_no);
 *   ble2_set_supported_features(20000000);                                        //Auto Advertise (Table: 2-5 in Datasheet)
 *   while (!wait_response("AOK"));                                                //wait to response AOK
 *   line_no += 20;
 *   Delay_ms(100);
 *   TFT_Write_Text("Reboot module",10,line_no);
 *   ble2_device_reboot();                                                       //reboot
 *   while (!wait_response("Reboot"));                                             //wait to response Reboot
 * }
 *
 * void main()
 * {
 *   Display_Init();
 *   MCU_Init();
 *   #ifdef INIT_BLE
 *   BLE_Init();
 *   delay_ms(2000);
 *   #else
 *   RN_WAKE = 1;
 *   wait_response("CMD");
 *   #endif
 *   DrawFrame();
 *   InitTimer2();
 *   while(1) {
 *     if(data_ready)
 *     {
 *       //If characteristic is configured as write
 *       //received messages come here
 *       Display_Message();
 *       reset_buff();
 *     }
 *     else
 *     {
 *        //Test: every 5sec increase baterry level (0 to 100%)
 *        //and send value via Bluetooth Low Energy
 *        if (tmr_flg)
 *        {
 *           batt_level++;
 *           if(batt_level > 100)
 *           {
 *             batt_level = 0;
 *           }
 *           Display_BatteryLevel();
 *           if(RN_CONN)
 *           { //send battery level value if BLE connected
 *             shorttohex(batt_level, batt_level_txt);
 *             ltrim(batt_level_txt);
 *             ble2_write_server_characteristic_value_via_UUID("2A19",batt_level_txt);
 *           }
 *           tmr_flg = 0;
 *        }
 *     }
 *   }
 * }
 *
 * @endcode
 */

/**
 * @page example_pic32 Example on PIC32
 * @par Example created using the EasyPIC Fusion
 * @par P32MX795F512L
 * @code
 *
 * #include "ble2_hw.h"
 * #define INIT_BLE
 * #define MAX_BUFFER_LEN        0x40
 * #define CR  0x0D
 * #define LF  0x0A
 *
 * // TFT module connections
 * char TFT_DataPort at LATE;
 * sbit TFT_RST at LATD7_bit;
 * sbit TFT_BLED at LATD2_bit;
 * sbit TFT_RS at LATD9_bit;
 * sbit TFT_CS at LATD10_bit;
 * sbit TFT_RD at LATD5_bit;
 * sbit TFT_WR at LATD4_bit;
 * char TFT_DataPort_Direction at TRISE;
 * sbit TFT_RST_Direction at TRISD7_bit;
 * sbit TFT_BLED_Direction at TRISD2_bit;
 * sbit TFT_RS_Direction at TRISD9_bit;
 * sbit TFT_CS_Direction at TRISD10_bit;
 * sbit TFT_RD_Direction at TRISD5_bit;
 * sbit TFT_WR_Direction at TRISD4_bit;
 * // End TFT module connections
 *
 * sbit RN_WAKE at LATC1_BIT;
 * sbit RN_WAKE_DIR at TRISC1_BIT;
 * sbit RN_CMD at LATD0_BIT;
 * sbit RN_CMD_DIR at TRISD0_BIT;
 * sbit RN_CONN at RB8_BIT;
 * sbit RN_CONN_DIR at TRISB8_BIT;
 * //sbit LED at GPIOD_ODR.B0;
 *
 * char rx_buff[MAX_BUFFER_LEN];
 * char data_len,data_ready;
 * char tmr_cnt,tmr_flg;
 * char batt_level;
 * char batt_level_txt[10];
 *
 * #define TIME_OUT       5*10    //5 seconds
 * #define INIT_BLE
 *
 * void UART2interrupt() iv IVT_UART_2 ilevel 6 ics ICS_AUTO
 * {
 *    char tmprd;
 *    tmprd = UART2_Read();
 *    if(tmprd == LF) {
 *      rx_buff[data_len] = 0;
 *      data_len++;
 *      data_ready = 1;
 *    }else{
 *      rx_buff[data_len] = tmprd;
 *      data_len++;
 *    }
 *    U2RXIF_bit = 0;
 * }
 *
 * //Timer2 Prescaler :1874; Preload = 63999; Actual Interrupt Time = 100 ms
 * void InitTimer2(){
 *   T2CON         = 0x8070;
 *   T2IP0_bit         = 1;
 *   T2IP1_bit         = 1;
 *   T2IP2_bit         = 1;
 *   T2IF_bit         = 0;
 *   T2IE_bit         = 1;
 *   PR2                 = 39063;
 *   TMR2                 = 0;
 * }
 *
 * void Timer2Interrupt() iv IVT_TIMER_2 ilevel 7 ics ICS_SRS
 * {
 *    T2IF_bit = 0;
 *   tmr_cnt++;
 *   if (tmr_cnt >= TIME_OUT) {
 *      tmr_cnt = 0;
 *      tmr_flg = 1;
 *   }
 * }
 *
 * void InitGPIO() {
 *     AD1PCFG = 0XFFFF;
 *     RN_WAKE_DIR = 0;
 *     RN_CMD_DIR = 0;
 *     RN_CONN_DIR = 1;
 *     RN_WAKE = 0;
 *     RN_CMD = 0;
 * }
 *
 *
 * void MCU_Init() {
 *   InitGPIO();
 *   UART2_Init(115200);
 *   Delay_ms(100);
 *   ble2_hal_init();
 *   data_len = 0;
 *   data_ready = 0;
 *   U2IP0_bit = 0;
 *   U2IP1_bit = 1;
 *   U2IP2_bit = 1;
 *   U2RXIE_bit = 1;
 *   EnableInterrupts();
 * }
 *
 * void Display_Init(){
 *   TFT_Init_ILI9341_8bit(320, 240);
 *   TFT_BLED = 1;
 * }
 *
 * void DrawFrame() {
 *   TFT_Fill_Screen(CL_WHITE);
 *   TFT_Set_Pen(CL_BLACK, 1);
 *   TFT_Line(20, 220, 300, 220);
 *   TFT_Line(20,  46, 300,  46);
 *   TFT_Set_Font(&TFT_defaultFont, CL_RED, FO_HORIZONTAL);
 *   TFT_Write_Text("BLE2  Click  Board  Demo", 25, 14);
 *   TFT_Set_Font(&TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
 *   TFT_Write_Text("EasyMx PRO v7", 19, 223);
 *   TFT_Set_Font(&TFT_defaultFont, CL_RED, FO_HORIZONTAL);
 *   TFT_Write_Text("www.mikroe.com", 200, 223);
 *   TFT_Set_Font(&TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
 *   TFT_Write_Text("BATTERY SERVICE", 40, 60);
 *   TFT_Write_Text("BATTERY LEVEL:", 40, 100);
 *   TFT_Write_Text("RECEIVE:", 40, 120);
 * }
 *
 * void reset_buff()
 * {
 *    memset(rx_buff,0,data_len);
 *    data_len = 0;
 *    data_ready = 0;
 * }
 *
 * void Display_BatteryLevel()
 * {
 *    char txt[3];
 *    ByteToStr(batt_level,txt);
 *    TFT_Set_Pen(CL_WHITE, 10);
 *    TFT_Set_Brush(1, CL_WHITE, 0, 0, 0, 0);
 *    TFT_Rectangle(150,100,230,115);
 *    TFT_Write_Text(txt,150,100);
 *    TFT_Write_Char(0x25,180,100);
 * }
 *
 * void Display_Message()
 * {
 *    TFT_Set_Pen(CL_WHITE, 10);
 *    TFT_Set_Brush(1, CL_WHITE, 0, 0, 0, 0);
 *    TFT_Rectangle(150,120,300,140);
 *    TFT_Write_Text(rx_buff,150,120);
 * }
 *
 * char wait_response(char *value)
 * {
 *    char result = 0;
 *    while(!data_ready);
 *    if(strstr(rx_buff,value))
 *    {
 *      result = 1;
 *    }
 *    memset(rx_buff,0,data_len);
 *    data_len = 0;
 *    data_ready = 0;
 *    return result;
 * }
 *
 * void BLE_Init()
 * {
 *   unsigned int line_no;
 *    line_no = 10;
 *   TFT_Write_Text("Start initialization",10,line_no);
 *   RN_WAKE = 1;
 *   while (!wait_response("CMD"));                                                //Wait to response CMD
 *   line_no += 20;
 *   Delay_ms(100);
 *   ble2_reset_to_factory_default(1);                                             //Reset RN4020 to factory settings
 *   while (!wait_response("AOK"));                                                //Wait to response AOK
 *   TFT_Write_Text("Factory reset",10,line_no);
 *   line_no += 20;
 *   Delay_ms(100);
 *   TFT_Write_Text("Set module name: BLE2_Click",10,line_no);
 *   ble2_set_device_name("BLE2_Click");                                           //Set name BLE2_Click
 *   while (!wait_response("AOK"));                                                //Wait to response AOK
 *   line_no += 20;
 *   Delay_ms(100);
 *   TFT_Write_Text("Configure services",10,line_no);
 *
 *
 *  // Example:
 *  // Table: 2-6 in Datasheet
 *  // Health Thermometer + Heart Rate + Battery Services
 *  // UART_WRite_Line("SS,F0000000");
 *
 *   ble2_set_server_services(40000000);                                           //Battery
 *   while (!wait_response("AOK"));                                                //Wait to response AOK
 *   line_no += 20;
 *   Delay_ms(100);
 *   TFT_Write_Text("Configure features",10,line_no);
 *   ble2_set_supported_features(20000000);                                        //Auto Advertise (Table: 2-5 in Datasheet)
 *   while (!wait_response("AOK"));                                                //wait to response AOK
 *   line_no += 20;
 *   Delay_ms(100);
 *   TFT_Write_Text("Reboot module",10,line_no);
 *   ble2_device_reboot();                                                       //reboot
 *   while (!wait_response("Reboot"));                                             //wait to response Reboot
 *
 *
 * }
 *
 * void main() {
 *   Display_Init();
 *   MCU_Init();
 *   #ifdef INIT_BLE
 *   BLE_Init();
 *   delay_ms(2000);
 *   #else
 *   RN_WAKE = 1;
 *   wait_response("CMD");
 *   #endif
 *   DrawFrame();
 *   InitTimer2();
 *   while(1)
 *   {
 *     if(data_ready)
 *     {
 *       //If characteristic is configured as write
 *       //received messages come here
 *       Display_Message();
 *       reset_buff();
 *     }
 *     else
 *     {
 *        //Test: every 5sec increase baterry level (0 to 100%)
 *        //and send value via Bluetooth Low Energy
 *        if (tmr_flg)
 *        {
 *           batt_level++;
 *           if(batt_level > 100)
 *           {
 *             batt_level = 0;
 *           }
 *           Display_BatteryLevel();
 *           if(RN_CONN)
 *           { //send battery level value if BLE connected
 *             shorttohex(batt_level, batt_level_txt);
 *             ltrim(batt_level_txt);
 *             ble2_write_server_characteristic_value_via_UUID("2A19",batt_level_txt);
 *           }
 *           tmr_flg = 0;
 *        }
 *     }
 *   }
 * }
 *
 *
 * @endcode
 */

/**
 * @page example_ft90 Example on FT90
 * @par Example created using the EasyFT90x
 * @par FT900
 * @code
 *
 *
 *
 * #include <built_in.h>
 * #include "ble2_hw.h"
 * #define INIT_BLE
 * #define MAX_BUFFER_LEN        0x40
 * #define LF  0x0A
 *
 * void Auxiliary_Code(void);
 * void Set_Index(unsigned short index);
 * void Write_Command(unsigned short cmd);
 * void Write_Data(unsigned int _data);
 *
 * // TFT module connections
 * char TFT_DataPort at GPIO_PORT_32_39;
 * sbit TFT_RST at GPIO_PIN42_bit;
 * sbit TFT_RS at GPIO_PIN40_bit;
 * sbit TFT_CS at GPIO_PIN41_bit;
 * sbit TFT_RD at GPIO_PIN46_bit;
 * sbit TFT_WR at GPIO_PIN47_bit;
 * sbit TFT_BLED at GPIO_PIN58_bit;
 * // End TFT module connections
 *
 * void Auxiliary_Code() {
 *   TFT_WR = 0;
 *   asm nop;
 *   TFT_WR = 1;
 * }
 *
 * void Set_Index(unsigned short index) {
 *   TFT_RS = 0;
 *   asm nop;
 *   TFT_DataPort = index;
 *   asm nop; asm nop;
 *   Auxiliary_Code();
 * }
 *
 * void Write_Command(unsigned short cmd) {
 *   TFT_RS = 1;
 *   asm nop;
 *   TFT_DataPort = cmd;
 *   asm nop; asm nop;
 *   Auxiliary_Code();
 * }
 *
 * void Write_Data(unsigned int _data) {
 *   TFT_RS = 1;
 *   asm nop;
 *   TFT_DataPort = Hi(_data);
 *   asm nop; asm nop;
 *   Auxiliary_Code();
 *   asm nop;
 *   TFT_DataPort = Lo(_data);
 *   asm nop; asm nop;
 *   Auxiliary_Code();
 * }
 *
 * sbit RN_WAKE at GPIO_PIN1_BIT;
 * sbit RN_CMD at GPIO_PIN56_BIT;
 * sbit RN_CONN at GPIO_PIN9_BIT;
 *
 * char rx_buff[MAX_BUFFER_LEN];
 * char data_len,data_ready;
 * char tmr_cnt,tmr_flg;
 * char batt_level;
 * char batt_level_txt[10];
 *
 * #define TIME_OUT       5    //5 seconds
 * #define INIT_BLE
 *
 * void UART_RX_ISR() iv IVT_UART2_IRQ ics ICS_AUTO
 * {
 *   char tmp_read;
 *   tmp_read = UART2_Read();
 *   if(tmp_read == LF) {
 *    rx_buff[data_len] = 0;
 *    data_len++;
 *    data_ready = 1;
 *  }else{
 *    rx_buff[data_len] = tmp_read;
 *    data_len++;
 *  }
 * }
 *
 * //Place/Copy this part in declaration section
 * void InitTimer2()
 * {
 *   TIMER_CONTROL_0 = 2;
 *   TIMER_SELECT = 1;
 *   TIMER_PRESC_LS = 36;
 *   TIMER_PRESC_MS = 244;
 *   TIMER_WRITE_LS = 63;
 *   TIMER_WRITE_MS = 6;
 *   TIMER_CONTROL_3 = 0;
 *   TIMER_CONTROL_4 |= 18;
 *   TIMER_CONTROL_2 |= 32;
 *   TIMER_INT |= 8;
 *   TIMER_CONTROL_1 |= 2;
 * }
 *
 *
 * void Timer2_interrupt() iv IVT_TIMERS_IRQ *
 * {
 *   tmr_cnt++;
 *   if (tmr_cnt >= TIME_OUT) {
 *      tmr_cnt = 0;
 *      tmr_flg = 1;
 *   }
 *     if (TIMER_INT_B_bit){
 *     TIMER_INT = (TIMER_INT & 0xAA) | (1 << 2);
 *   }
 * }
 *
 * void InitGPIO()
 * {
 *   GPIO_Digital_Output( &GPIO_PORT_00_07, _GPIO_PINMASK_1 );
 *   GPIO_Digital_Output( &GPIO_PORT_48_55, _GPIO_PINMASK_1 );
 *   GPIO_Digital_Input( &GPIO_PORT_08_15, _GPIO_PINMASK_1 );
 *   RN_WAKE = 0;
 *   RN_CMD = 0;
 * }
 *
 *
 * void MCU_Init() {
 *   InitGPIO();
 *   UART2_Init_Advanced( 115200, _UART_DATA_BITS_8,
 *                                  _UART_PARITY_NONE,
 *                                  _UART_STOP_BITS_1 );
 *   Delay_ms(100);
 *   ble2_hal_init();
 *   data_len = 0;
 *   data_ready = 0;
 *
 *   UART2_FCR |= ( 1 << FIFO_EN ) | ( 1 << RCVR_RST );
 *   UART2_INT_ENABLE |= ( 1 << RX_AVL_EN );
 *
 *   IRQ_CTRL &= ~( 1 << GLOBAL_INTERRUPT_MASK );
 * }
 *
 * void Display_Init(){
 *   TFT_Set_Active(Set_Index, Write_Command, Write_Data);
 *   TFT_Init_ILI9341_8bit(320, 240);
 *   TFT_BLED = 1;
 * }
 *
 * void DrawFrame() {
 *   TFT_Fill_Screen(CL_WHITE);
 *   TFT_Set_Pen(CL_BLACK, 1);
 *   TFT_Line(20, 220, 300, 220);
 *   TFT_Line(20,  46, 300,  46);
 *   TFT_Set_Font(&TFT_defaultFont, CL_RED, FO_HORIZONTAL);
 *   TFT_Write_Text("BLE2  Click  Board  Demo", 25, 14);
 *   TFT_Set_Font(&TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
 *   TFT_Write_Text("EasyMx PRO v7", 19, 223);
 *   TFT_Set_Font(&TFT_defaultFont, CL_RED, FO_HORIZONTAL);
 *   TFT_Write_Text("www.mikroe.com", 200, 223);
 *   TFT_Set_Font(&TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
 *   TFT_Write_Text("BATTERY SERVICE", 40, 60);
 *   TFT_Write_Text("BATTERY LEVEL:", 40, 100);
 *   TFT_Write_Text("RECEIVE:", 40, 120);
 * }
 *
 * void reset_buff() {
 *    memset(rx_buff,0,data_len);
 *    data_len = 0;
 *    data_ready = 0;
 * }
 *
 * char send_succcess() {
 *    char success = 0;
 *    while (!data_ready);
 *    if(strstr(rx_buff,"AOK")) {
 *      success = 1;
 *    }
 *    reset_buff();
 *    return success;
 * }
 *
 * void Display_BatteryLevel() {
 *    char txt[3];
 *    ByteToStr(batt_level,txt);
 *    TFT_Set_Pen(CL_WHITE, 10);
 *    TFT_Set_Brush(1, CL_WHITE, 0, 0, 0, 0);
 *    TFT_Rectangle(150,100,230,115);
 *    TFT_Write_Text(txt,150,100);
 *    TFT_Write_Char(0x25,180,100);
 * }
 *
 * void Display_Message() {
 *    TFT_Set_Pen(CL_WHITE, 10);
 *    TFT_Set_Brush(1, CL_WHITE, 0, 0, 0, 0);
 *    TFT_Rectangle(150,120,300,140);
 *    TFT_Write_Text(rx_buff,150,120);
 * }
 *
 * char wait_response(char *value) {
 *    char result = 0;
 *    while(!data_ready);
 *    if(strstr(rx_buff,value)) {
 *      result = 1;
 *    }
 *    memset(rx_buff,0,data_len);
 *    data_len = 0;
 *    data_ready = 0;
 *    return result;
 * }
 *
 * void BLE_Init()
 * {
 *   unsigned int line_no;
 *    line_no = 10;
 *   TFT_Write_Text("Start initialization",10,line_no);
 *   RN_WAKE = 1;
 *   while (!wait_response("CMD"));                                                //Wait to response CMD
 *   line_no += 20;
 *   Delay_ms(100);
 *   ble2_reset_to_factory_default(1);                                             //Reset RN4020 to factory settings
 *   while (!wait_response("AOK"));                                                //Wait to response AOK
 *   TFT_Write_Text("Factory reset",10,line_no);
 *   line_no += 20;
 *   Delay_ms(100);
 *   TFT_Write_Text("Set module name: BLE2_Click",10,line_no);
 *   ble2_set_device_name("BLE2_Click");                                           //Set name BLE2_Click
 *   while (!wait_response("AOK"));                                                //Wait to response AOK
 *   line_no += 20;
 *   Delay_ms(100);
 *   TFT_Write_Text("Configure services",10,line_no);
 *
 *
 *  // Example:
 *  // Table: 2-6 in Datasheet
 *  // Health Thermometer + Heart Rate + Battery Services
 *  // UART_WRite_Line("SS,F0000000");
 *
 *
 *   ble2_set_server_services(40000000);                                           //Battery
 *   while (!wait_response("AOK"));                                                //Wait to response AOK
 *   line_no += 20;
 *   Delay_ms(100);
 *   TFT_Write_Text("Configure features",10,line_no);
 *   ble2_set_supported_features(20000000);                                        //Auto Advertise (Table: 2-5 in Datasheet)
 *   while (!wait_response("AOK"));                                                //wait to response AOK
 *   line_no += 20;
 *   Delay_ms(100);
 *   TFT_Write_Text("Reboot module",10,line_no);
 *   ble2_device_reboot();                                                       //reboot
 *   while (!wait_response("Reboot"));                                             //wait to response Reboot
 * }
 *
 * void main()
 * {
 *   Display_Init();
 *   MCU_Init();
 *   #ifdef INIT_BLE
 *   BLE_Init();
 *   delay_ms(2000);
 *   #else
 *   RN_WAKE = 1;
 *   wait_response("CMD");
 *   #endif
 *   DrawFrame();
 *   InitTimer2();
 *   while(1) {
 *     if(data_ready)
 *     {
 *       //If characteristic is configured as write
 *       //received messages come here
 *       Display_Message();
 *       reset_buff();
 *     }
 *     else
 *     {
 *        //Test: every 5sec increase baterry level (0 to 100%)
 *        //and send value via Bluetooth Low Energy
 *        if (tmr_flg)
 *        {
 *           batt_level++;
 *           if(batt_level > 100)
 *           {
 *             batt_level = 0;
 *           }
 *           Display_BatteryLevel();
 *           if(RN_CONN)
 *           { //send battery level value if BLE connected
 *             shorttohex(batt_level, batt_level_txt);
 *             ltrim(batt_level_txt);
 *             ble2_write_server_characteristic_value_via_UUID("2A19",batt_level_txt);
 *           }
 *           tmr_flg = 0;
 *        }
 *     }
 *   }
 * }
 *
 *
 *
 * @endcode
 * /
 */



