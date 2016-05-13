/*******************************************************************************
* Title                 :   BLE2 HWL
* Filename              :   ble2_hw.c
* Author                :   VM
* Origin Date           :   11/05/2016
* Notes                 :   None
*******************************************************************************/
/*************** MODULE REVISION LOG ******************************************
*
*    Date    Software Version    Initials   Description
*  11/05/16    1.0.0               VM      Module Created.
*
*******************************************************************************/

/******************************************************************************
* Includes
*******************************************************************************/
#include "ble2_hw.h"
#include <string.h>
#include <stdlib.h>

/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/

/******************************************************************************
* Module Typedefs
*******************************************************************************/

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/

/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/

//***** function implementations ***** //

void ble2_set_device_bluetooth_name (char* device_name)
{
    char tmp[16] = {0};
    char *tmp_device_name = device_name;
    strcpy ( tmp, "S-," );
    strcat ( tmp, tmp_device_name );
    ble2_hal_send(tmp);
}

int8_t ble2_set_baud_rate (baud_rate_t baud_rate)
{
    char tmp[16] = {0};
    char tmpb[2] = "0";
    int8_t status = 0;
    switch(baud_rate)
    {
    case BR_2400:
        break;
    case BR_9600:
        tmpb[0] ='1';
        break;
    case BR_19200:
        tmpb[0] = '2';
        break;
    case BR_38400:
        tmpb[0] = '3';
        break;
    case BR_115200:
        tmpb[0] = '4';
        break;
    case BR_230400:
        tmpb[0] = '5';
        break;
    case BR_460800:
        tmpb[0] = '6';
        break;
    case BR_921600:
        tmpb[0] = '7';
        break;
    default:
        status = -1;
        return status;
    }

    strcpy ( tmp, "SB,");
    strcat (tmp, tmpb);

    ble2_hal_send(tmp);

    return status;
}

int8_t ble2_set_firmware_rev_value(char* fw_value)
{
    char tmp[23] = {0};
    char *tmp_fw_value = fw_value;
    int8_t status = 0;
    if (strlen(tmp_fw_value) > 20) return (status - 1);

    strcpy( tmp, "SDF,");
    strcat( tmp, tmp_fw_value);

    ble2_hal_send(tmp);

    return status;
}

int8_t ble2_set_hw_rev_value(char* hw_value)
{
    char tmp[23] = {0};
    char* tmp_hw_value = hw_value;
    int8_t status = 0;
    if (strlen (hw_value) > 20) return (status - 1);

    strcpy( tmp, "SDH,");
    strcat( tmp, tmp_hw_value);

    ble2_hal_send(tmp);

    return status;
}

int8_t ble2_set_model_value(char* model_value)
{
    char tmp[23] = {0};
    int8_t status = 0;

    if (sizeof(model_value) > 20) return (status - 1);

    strcpy( tmp, "SDM,");
    strcat( tmp, model_value);

    ble2_hal_send(tmp);

    return status;
}

int8_t ble2_set_manufacturer_name_value (char* name_value)
{
    char tmp[23] = {0};
    int8_t status = 0;
    if (sizeof(name_value) > 20) return (status - 1);

    strcpy( tmp, "SDN,");
    strcat( tmp, name_value);

    ble2_hal_send(tmp);

    return status;
}

int8_t ble2_set_sw_revision_value (char* sw_value)
{
    char tmp[23] = {0};
    int8_t status = 0;
    if (sizeof(sw_value) > 20) return (status - 1);

    strcpy( tmp, "SDR,");
    strcat( tmp, sw_value);

    ble2_hal_send(tmp);

    return status;
}

int8_t ble2_set_serial_number_value (char* serial_value)
{
    char tmp[23] = {0};
    int8_t status = 0;
    if (sizeof(serial_value) > 20) return (status - 1);

    strcpy( tmp, "SDS,");
    strcat( tmp, serial_value);

    ble2_hal_send(tmp);

    return status;
}

int8_t ble2_reset_to_factory_default(reset_parameter_t reset)
{
    char tmp[10] = {0};
    char rst[2] = "1";
    int8_t status = 0;

    switch(reset)
    {
    case RESET_SOME:
        break;
    case RESET_ALL:
        rst[0] = '2';
        break;
    default:
        return (status - 1);
    }

    strcpy( tmp, "SF,");
    strcat( tmp, rst);

    ble2_hal_send(tmp);
    return status;
}

int8_t ble2_set_device_name(char* name)
{
    char *tmp_ptr = name;
    char tmp[21];
    int8_t status = 0;

    if ( sizeof(name) > 20)
        return (status - 1);

    strcpy( tmp, "SN,");
    strcat( tmp, tmp_ptr);

    ble2_hal_send(tmp);
    return status;
}

void ble2_set_supported_features(uint32_t features)
{
    char tmp_f [20];
    char tmp[15] = {0};

    LongToStr(features, tmp_f);
    ltrim(tmp_f);
    strcpy ( tmp, "SR," );
    strcat ( tmp, tmp_f);

    ble2_hal_send(tmp);
}

void ble2_set_server_services(uint32_t services)
{
    char tmp [ 100 ] = { 0 };
    char tmp_s[50];
    LongToStr(services, tmp_s);
    ltrim(tmp_s);
    strcpy ( tmp, "SS,");
    strcat ( tmp, tmp_s);
    ble2_hal_send(tmp);
}

int8_t ble2_set_initial_connection_params (uint16_t interval, uint16_t latency, uint16_t timeout)
{
    char tmp[43];
    char tmp_interval[10];
    char tmp_lat[10];
    char tmp_timeout[10];

    int8_t status = 0;
    if ( (interval < 0x0006) || (interval > 0x0C80) )
        return status - 1;
    if ( (latency > 0x01F3) || (latency < (timeout * 10/(interval * 1.25 - 1))) )
        return status - 1;
    if ( ( timeout < 0x000A ) || ( timeout > 0x0C80 ) )
        return status - 1;

    WordToHex(interval, tmp_interval);
    ltrim(tmp_interval);
    WordToHex(latency,tmp_lat);
    ltrim(tmp_interval);
    WordToHex(timeout,tmp_timeout);
    ltrim(tmp_interval);

    strcpy(tmp, "ST,");
    strcat(tmp, tmp_interval);
    strcat(tmp, ",");
    strcat(tmp, tmp_lat);
    strcat(tmp, ",");
    strcat(tmp, tmp_timeout);

    ble2_hal_send(tmp);

    return status;
}

void ble2_trigger_echo()
{
    char tmp[5] = "+";
    ble2_hal_send(tmp);
}

void ble2_start_advertisment (uint16_t interval, uint16_t window_time)
{
    char tmp[25] = {0};
    char tmp_interval[5] = {0};
    char tmp_window_time[5] = {0};

    WordToHex(interval, tmp_interval);
    ltrim(tmp_interval);
    WordToHex(window_time,tmp_window_time);
    ltrim(tmp_window_time);

    strcpy (tmp,"A");
    strcat (tmp, ",");
    strcat (tmp, tmp_interval);
    strcat (tmp, ",");
    strcat (tmp, tmp_window_time);

    ble2_hal_send(tmp);
}

void ble2_bond(bond_saving_t parameter)
{
    char tmp[10] = {0};
    strcpy(tmp, "B,");
    if (parameter == 0)
        strcat(tmp, "0");
    else
        strcat(tmp, "1");

    ble2_hal_send(tmp);
}

void ble2_display_critical_info()
{
    char tmp[5] = "D";
    ble2_hal_send(tmp);
}

int8_t ble2_start_connection(mac_address_t mac_address_type,char* mac_address )
{
    char tmp[20] = {0};
    char tmp_mac_address_type[2] = "0";
    char tmp_mac_address[7] = {0};
    int8_t status = 0;

    if (mac_address_type == 1)
        tmp_mac_address_type[0] = '1';

    else if (mac_address_type != 0)
        return (status-1);

    if (strlen (mac_address) > 6)
        return (status-1);

    strcpy(tmp_mac_address,mac_address);
    strcpy(tmp, "E,");
    strcat(tmp, tmp_mac_address_type);
    strcat(tmp,",");
    strcat(tmp, tmp_mac_address);

    ble2_hal_send(tmp);

    return status;
}

void ble2_query_peripheral_devices(uint16_t scan_interval, uint16_t scan_window)
{
    char tmp[33] = {0};
    char tmp_scan_interval[10] = {0};
    char tmp_scan_windows[10] = {0};

    LongWordToHex(scan_interval,tmp_scan_interval);
    ltrim(tmp_scan_interval);
    LongWordToHex(scan_window,tmp_scan_windows );
    ltrim(tmp_scan_windows);

    strcpy(tmp, "F,");
    strcat(tmp,tmp_scan_interval);
    strcat(tmp,",");
    strcat(tmp,tmp_scan_windows);

    ble2_hal_send(tmp);
}

void ble2_receive_broadcast_messages(uint16_t scan_interval, uint16_t scan_window)
{
    char tmp[33] = {0};
    char tmp_scan_interval[10] = {0};
    char tmp_scan_windows[10] = {0};

    LongWordToHex(scan_interval,tmp_scan_interval);
    ltrim(tmp_scan_interval);
    LongWordToHex(scan_window,tmp_scan_windows );
    ltrim(tmp_scan_windows);

    strcpy(tmp, "F,");
    strcat(tmp,tmp_scan_interval);
    strcat(tmp,",");
    strcat(tmp,tmp_scan_windows);

    ble2_hal_send(tmp);
}

void ble2_observer_role_enable(bool enable)
{
    char tmp[10] = {0};
    strcpy(tmp, "J,");
    if (enable)
        strcat(tmp, "1");
    else
        strcat(tmp,"0");

    ble2_hal_send(tmp);
}

void ble2_kill_active_connection()
{
    char tmp[5] = "K";
    ble2_hal_send(tmp);
}

void ble2_get_signal_strength()
{
    char tmp[5] = "M";
    ble2_hal_send(tmp);
}

int8_t ble2_init_broadcast(uint32_t advertisment_content)
{
    char tmp[33] = {0};
    char tmp_adv [26] = {0};
    LongLongUnsignedToHex(advertisment_content,tmp_adv);
    ltrim(tmp_adv);

    if (strlen(tmp_adv) > 25)
        return -1;

    strcpy(tmp,"N,");
    strcat(tmp,tmp_adv);
    ble2_hal_send(tmp);
    return 0;
}

void ble2_dormant_mode_enable()
{
    char tmp[8] = "O";
    ble2_hal_send(tmp);
}

void ble2_get_connection_status()
{
    char tmp[8] = "Q,0";
    ble2_hal_send(tmp);
}

void ble2_get_bonded_status()
{
    char tmp[8] = "Q,1";
    ble2_hal_send(tmp);
}

void ble2_device_reboot()
{
     char tmp[8] = "R,1";
     ble2_hal_send(tmp);
}

void ble2_change_connection_parameters(uint16_t interval, uint16_t latency, uint16_t timeout)
{
    char tmp[33] = {0};
    char temp_interval [9] = {0};
    char temp_latency  [9] = {0};
    char temp_timeout  [9] = {0};

    WordToHex(interval, temp_interval);
    ltrim(temp_interval);
    WordToHex(latency,temp_latency);
    ltrim(temp_latency);
    WordToHex(timeout,temp_timeout);
    ltrim(temp_timeout);

    strcpy(tmp, "T,");
    strcat(tmp, temp_interval);
    strcat(tmp,",");
    strcat(tmp, temp_latency);
    strcat(tmp, ",");
    strcat(tmp, temp_timeout);
    ble2_hal_send(tmp);
}

void ble2_remove_bonding()
{
    char tmp[5] = "U";
    ble2_hal_send(tmp);
}

void ble2_display_firmware_version()
{
    char tmp[5] = "V";
    ble2_hal_send(tmp);
}

void ble2_stop_inquiry_process()
{
    char tmp[5] = "X";
    ble2_hal_send(tmp);
}

void ble2_stop_receiving_broadcast()
{
    char tmp[5] = "X";
    ble2_hal_send(tmp);
}

void ble2_stop_advertising()
{
    char tmp[5] = "Y";
    ble2_hal_send(tmp);
}

void ble2_stop_connection_process()
{
    char tmp[5] = "Z";
    ble2_hal_send(tmp);
}

void ble2_list_client_services()
{
    char tmp[8] = "LC";
    ble2_hal_send(tmp);
}

void  ble2_list_client_services_uuid_service(char* uuid)
{
    char *tmp_uuid = uuid;
    char tmp [15] = {0};

    strcpy(tmp, "LC,");
    strcat(tmp, tmp_uuid);
    ble2_hal_send(tmp);
}

void  ble2_list_client_services_uuid_characteristic(char* uuid_service, char* uuid_characteristic)
{
    char *tmp_uuid_service = uuid_service;
    char *tmp_uuid_characteristic = uuid_characteristic;
    char tmp [15] = {0};

    strcpy(tmp, "LC,");
    strcat(tmp, tmp_uuid_service);
    strcat(tmp, ",");
    strcat(tmp, tmp_uuid_characteristic);
    ble2_hal_send(tmp);
}

void ble2_list_server_services()
{
    char tmp[8] = "LS";
    ble2_hal_send(tmp);
}

void ble2_read_characteristic_content(uint16_t handle)
{
    char tmp [15] = {0};
    char tmp_handle [5] = {0};

    WordToHex(handle, tmp_handle);
    strcpy(tmp, "CHR,");
    strcat(tmp, tmp_handle);
    ble2_hal_send(tmp);
}

void  ble2_write_characteristic_content(uint16_t handle, char* content)
{
    char tmp[20] = {0};
    char tmp_handle [5] = {0};
    char *tmp_content = content;

    WordToHex(handle,tmp_handle);
    ltrim(tmp_handle);
    strcpy(tmp, "CHW,");
    strcat(tmp, tmp_handle);
    strcat(tmp,",");
    strcat(tmp, tmp_content);
    ble2_hal_send(tmp);
}

void ble2_read_characteristic_configuration ( char* UUID )
{
    char tmp[48] = {0};
    char *tmp_uuid = UUID;

    strcpy(tmp, "CURC,");
    strcat(tmp, tmp_uuid);
    ble2_hal_send(tmp);
}

void ble2_read_characteristic_content_via_UUID(char* UUID)
{
    char tmp[48] = {0};
    char *tmp_uuid = UUID;

    strcpy(tmp, "CURV,");
    strcat(tmp, tmp_uuid);
    ble2_hal_send(tmp);
}

void ble2_write_characteristic_configuration_via_UUID(char* UUID, configuration_t configuration)
{
    char tmp[48] = {0};
    char *tmp_uuid = UUID;
    char tmp_configuration [2] = {0};

    sprintf(tmp_configuration,"%d", configuration);

    strcpy(tmp, "CUWC,");
    strcat(tmp, tmp_uuid);
    strcat(tmp,",");
    strcat(tmp, tmp_configuration);
    ble2_hal_send(tmp);
}

void  ble2_write_characteristic_content_via_UUID(char* UUID, char* content)
{
    char tmp[53] = {0};
    char *tmp_uuid = UUID;
    char *tmp_content = content;

    strcpy(tmp, "CUWV,");
    strcat(tmp,tmp_uuid);
    strcat(tmp, ",");
    strcat(tmp,tmp_content);
    ble2_hal_send(tmp);
}

void ble2_read_server_characteristic_value_via_handle (uint16_t handle)
{
    char tmp[13] = {0};
    char tmp_handle [5];
    WordToHex(handle, tmp_handle);
    ltrim(tmp_handle);

    strcpy(tmp, "SHR,");
    strcat(tmp,tmp_handle);
    ble2_hal_send(tmp);
}

void ble2_write_server_characteristic_value_via_handle (uint16_t handle, char* content)
{
    char tmp[23] = {0};
    char tmp_handle[5];
    char *tmp_value = content;

    WordToHex(handle, tmp_handle);
    ltrim(tmp_handle);

    strcat(tmp, "SHW,");
    strcat(tmp, tmp_handle);
    strcat(tmp, ",");
    strcat(tmp, tmp_value);
    ble2_hal_send(tmp);
}


void ble2_read_server_characteristic_value_via_UUID (char* UUID)
{
    char tmp[33] = {0};
    char* tmp_uuid = UUID;

    strcpy(tmp, "SUR,");
    strcat(tmp,tmp_uuid);
    ble2_hal_send(tmp);
}

void ble2_write_server_characteristic_value_via_UUID (char* UUID, char* content)
{
    char tmp[55] = {0};
    char* tmp_uuid = UUID;
    char* tmp_content = content;

    strcpy(tmp,"SUW,");
    strcat(tmp,tmp_uuid);
    strcat(tmp, ",");
    strcat(tmp,tmp_content);
    ble2_hal_send(tmp);
}

uint8_t ble2_set_private_characteristics(char* UUID, char* bitmap_characteristic, uint8_t max_data_len)
{
     char tmp[100] = {0};
     char* uuid_ptr = UUID;
     char* bmp_ptr = bitmap_characteristic;
     char tmp_max_data_len[10];

     if (max_data_len > 20)
     return -1;

     ByteToStr(max_data_len, tmp_max_data_len);
     strcpy(tmp, "PC");
     strcat(tmp, uuid_ptr);
     strcat(tmp,bmp_ptr);
     strcat(tmp,tmp_max_data_len);
     ble2_hal_send(tmp);
     return 0;
}

void ble2_set_private_service_primary_filter_uuid(char* UUID)
{
    char tmp[33] = {0};
    char *tmp_uuid = UUID;

    strcpy(tmp, "PF,");
    strcat(tmp,tmp_uuid);
    ble2_hal_send(tmp);
}

void ble2_erase_private_service_primary_filter_uuid()
{
    char tmp[8] = "PF,Z";
    ble2_hal_send(tmp);
}

void ble2_set_private_service_uuid(char* uuid)
{
    char tmp[35] = {0};
    char *tmp_uuid = uuid;

    strcpy(tmp, "PS,");
    strcat(tmp,tmp_uuid);
    ble2_hal_send(tmp);
}

void ble2_private_service_clear_all_()
{
    char tmp[6] = "PZ";
    ble2_hal_send(tmp);
}

void ble2_init_mldp_simulation_mode()
{
    char tmp[6] = "I";
    ble2_hal_send(tmp);
}

int8_t ble2_mldp_set_security_mode(security_t additional_security)
{
    char tmp[10] = {0};
    int8_t status = 0;
    strcpy(tmp,"SE,");

    switch(additional_security)
    {
    case 0:
        strcat(tmp,"0");
        break;
    case 1:
        strcat(tmp,"1");
        break;
    case 2:
        strcat(tmp,"2");
        break;
    default:
        return (status - 1);
    }
    ble2_hal_send(tmp);

    return status;
}


//***** scripting commands *****//

void ble2_list_current_script()
{
    char tmp[6] = "LW";
    ble2_hal_send(tmp);
}

void ble2_clear_script()
{
    char tmp[6] = "WC";
    ble2_hal_send(tmp);
}

void ble2_stop_script_execution()
{
    char tmp[6] = "WP";
    ble2_hal_send(tmp);
}

void ble2_start_script_execution()
{
    char tmp[6] = "WR";
    ble2_hal_send(tmp);
}

int8_t ble2_start_script_debug(debug_event_t debug_event)
{
    char tmp[10] = {0};
    char tmp_debug_event[3];
    int8_t status = 0;
    strcpy(tmp, "WR,0");

    switch(debug_event)
    {
    case 0:
        tmp[3] = '0';
        break;
    case 1:
        tmp[3] = '1';
        break;
    case 2:
        tmp[3] = '2';
        break;
    case 3:
        tmp[3] = '3';
        break;
    case 4:
        tmp[3] = '4';
        break;
    case 5:
        tmp[3] = '5';
        break;
    case 6:
        tmp[3] = '6';
        break;
    case 7:
        tmp[3] = '7';
        break;
    case 8:
        tmp[3] = '8';
        break;
    case 9:
        tmp[3] = '9';
        break;
    default:
        return (status -1);
    }
    ble2_hal_send(tmp);
    return status;
}

void ble2_enter_script_input_mode()
{
    char tmp[8] = "WW";
    ble2_hal_send(tmp);
}

void ble2_remote_command_enable(bool enable)
{
    char tmp[10] = {0};
    strcpy(tmp, "!,");
    if (enable)
        strcat(tmp,"1");
    else strcat(tmp,"2");
    ble2_hal_send(tmp);
}

int8_t ble2_update_firmware(fw_update_t interface)
{
    char tmp[10] = {0};
    int8_t status = 0;

    strcpy(tmp, "~,");
    if (interface == 1)
        strcat(tmp, "1");
    else if (interface == 2)
        strcat(tmp,"2");
    else return (status - 1);
    ble2_hal_send(tmp);

    return status;
}

/*************** END OF FUNCTIONS ***************************************************************************/
