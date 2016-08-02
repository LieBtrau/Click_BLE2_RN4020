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
#include <stdio.h>

#ifdef ARDUINO_ARCH_AVR
#endif
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
    char tmp2[2];

    if(baud_rate > BR_921600)
    {
        return -1;
    }
    strcpy (tmp, "SB,");
    sprintf(tmp2,"%1X",baud_rate);
    strcat (tmp, tmp2);
    ble2_hal_send(tmp);

    return 0;
}

//Only available starting from firmware version 1.20
int8_t ble2_set_transmission_power(tx_pwr_t pwr)
{
    char tmp[16] = {0};
    char tmp2[2];

    if(pwr > PWR_7_5)
    {
        return -1;
    }
    strcpy (tmp, "SP,");
    sprintf(tmp2,"%1X",pwr);
    strcat (tmp, tmp2);
    ble2_hal_send(tmp);

    return 0;
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
    char rst[2];
    int8_t status = 0;

    if(reset != RESET_SOME && reset!=RESET_ALL)
    {
        return -1;
    }
    strcpy( tmp, "SF,");
    strcat( tmp, RESET_SOME ? "1" : "2");

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
    char tmp[15] = {0};

    strcpy (tmp, "SR," );
    sprintf(tmp+3,"%08lx",features);

    ble2_hal_send(tmp);
}

void ble2_set_server_services(uint32_t services)
{
    char tmp [15] = { 0 };

    strcpy (tmp, "SS,");
    sprintf(tmp+3,"%08lx",services);

    ble2_hal_send(tmp);
}

int8_t ble2_set_initial_connection_params (uint16_t interval, uint16_t latency, uint16_t timeout)
{
    char tmp[43];
    char tmp2[10];

    if ( (interval < 0x0006) || (interval > 0x0C80) )
        return -1;
    if ( (latency > 0x01F3) || (latency < (timeout * 10/(interval * 1.25 - 1))) )
        return -1;
    if ( ( timeout < 0x000A ) || ( timeout > 0x0C80 ) )
        return -1;

    strcpy(tmp, "ST,");
    sprintf(tmp2,"%04X",interval);
    strcat(tmp,tmp2);
    strcat(tmp, ",");
    sprintf(tmp2,"%04X",latency);
    strcat(tmp,tmp2);
    strcat(tmp, ",");
    sprintf(tmp2,"%04X",timeout);
    strcat(tmp,tmp2);

    ble2_hal_send(tmp);

    return 0;
}

void ble2_trigger_echo()
{
    ble2_hal_send("+");
}

void ble2_start_advertisment (uint16_t interval, uint16_t window_time)
{
    char tmp[25] = {0};
    char tmp2[5] = {0};

    strcpy (tmp,"A,");
    sprintf(tmp2,"%04X",interval);
    strcat(tmp,tmp2);
    if(window_time>interval)
    {
        sprintf(tmp2,",%04X",interval);
        strcat(tmp,tmp2);
    }
    ble2_hal_send(tmp);
}

void ble2_bond(bond_saving_t parameter)
{
    char tmp[10] = {0};
    strcpy(tmp, "B,");
    strcat(tmp, !parameter ? "0" : "1");

    ble2_hal_send(tmp);
}

void ble2_display_critical_info()
{
    ble2_hal_send("D");
}

int8_t ble2_start_connection(mac_address_t mac_address_type, char* mac_address )
{
    char tmp[20] = {0};

    if (mac_address_type > 1)
        return -1;
    if (strlen (mac_address) > 6)
        return -1;

    strcpy(tmp, "E,");
    strcat(tmp, (!mac_address_type) ? "0" : "1");
    strcat(tmp,",");
    strcat(tmp, mac_address);

    ble2_hal_send(tmp);

    return 0;
}

void ble2_query_peripheral_devices(uint16_t scan_interval, uint16_t scan_window)
{
    char tmp[33] = {0};
    char tmp2[10] = {0};

    strcpy(tmp, "F,");
    sprintf(tmp2,"%04X",scan_interval);
    strcat(tmp,tmp2);
    strcat(tmp,",");
    sprintf(tmp2,"%04X",scan_window);
    strcat(tmp,tmp2);

    ble2_hal_send(tmp);
}

void ble2_receive_broadcast_messages(uint16_t scan_interval, uint16_t scan_window)
{
    ble2_query_peripheral_devices(scan_interval, scan_window);
}

void ble2_observer_role_enable(bool enable)
{
    char tmp[10] = {0};
    strcpy(tmp, "J,");
    strcat(tmp, enable ? "1" : "0");

    ble2_hal_send(tmp);
}

void ble2_kill_active_connection()
{
    ble2_hal_send("K");
}

void ble2_get_signal_strength()
{
    ble2_hal_send("M");
}

int8_t ble2_init_broadcast(uint32_t advertisment_content)
{
    char tmp[33] = {0};

    strcpy(tmp,"N,");
    sprintf(tmp+3,"%08lx",advertisment_content);
    ble2_hal_send(tmp);
    return 0;
}

void ble2_dormant_mode_enable()
{
    ble2_hal_send("O");
}

void ble2_get_connection_status()
{
    ble2_hal_send("Q,0");
}

void ble2_get_bonded_status()
{
    ble2_hal_send("Q,1");
}

void ble2_device_reboot()
{
    ble2_hal_send("R,1");
}

uint8_t ble2_change_connection_parameters(uint16_t interval, uint16_t latency, uint16_t timeout)
{
    char tmp[43];
    char tmp2[10];

    if ( (interval < 0x0006) || (interval > 0x0C80) )
        return -1;
    if ( (latency > 0x01F3) || (latency < (timeout * 10/(interval * 1.25 - 1))) )
        return -1;
    if ( ( timeout < 0x000A ) || ( timeout > 0x0C80 ) )
        return -1;

    strcpy(tmp, "T,");
    sprintf(tmp2,"%04X",interval);
    strcat(tmp,tmp2);
    strcat(tmp, ",");
    sprintf(tmp2,"%04X",latency);
    strcat(tmp,tmp2);
    strcat(tmp, ",");
    sprintf(tmp2,"%04X",timeout);
    strcat(tmp,tmp2);

    ble2_hal_send(tmp);

    return 0;
}

void ble2_remove_bonding()
{
    ble2_hal_send("U");
}

void ble2_display_firmware_version()
{
    ble2_hal_send("V");
}

void ble2_stop_inquiry_process()
{
    ble2_hal_send("X");
}

void ble2_stop_receiving_broadcast()
{
    ble2_hal_send("X");
}

void ble2_stop_advertising()
{
    ble2_hal_send("Y");
}

void ble2_stop_connection_process()
{
    ble2_hal_send("Z");
}

void ble2_list_client_services()
{
    ble2_hal_send("LC");
}

void  ble2_list_client_services_uuid_service(char* uuid)
{
    char tmp [15] = {0};

    strcpy(tmp, "LC,");
    strcat(tmp, uuid);
    ble2_hal_send(tmp);
}

void  ble2_list_client_services_uuid_characteristic(char* uuid_service, char* uuid_characteristic)
{
    char tmp [15] = {0};

    strcpy(tmp, "LC,");
    strcat(tmp, uuid_service);
    strcat(tmp, ",");
    strcat(tmp, uuid_characteristic);
    ble2_hal_send(tmp);
}

void ble2_list_server_services()
{
    ble2_hal_send("LS");
}

void ble2_read_characteristic_content(uint16_t handle)
{
    char tmp [15] = {0};
    char tmp2 [5] = {0};

    strcpy(tmp, "CHR,");
    sprintf(tmp2,"%04X",handle);
    strcat(tmp, tmp2);
    ble2_hal_send(tmp);
}

void  ble2_write_characteristic_content(uint16_t handle, char* content)
{
    char tmp[20] = {0};
    char tmp2 [5] = {0};

    strcpy(tmp, "CHW,");
    sprintf(tmp2,"%04X",handle);
    strcat(tmp, tmp2);
    strcat(tmp,",");
    strcat(tmp, content);
    ble2_hal_send(tmp);
}

void ble2_read_characteristic_configuration ( char* UUID )
{
    char tmp[48] = {0};

    strcpy(tmp, "CURC,");
    strcat(tmp, UUID);
    ble2_hal_send(tmp);
}

void ble2_read_characteristic_content_via_UUID(char* UUID)
{
    char tmp[48] = {0};

    strcpy(tmp, "CURV,");
    strcat(tmp, UUID);
    ble2_hal_send(tmp);
}

void ble2_write_characteristic_configuration_via_UUID(char* UUID, configuration_t configuration)
{
    char tmp[48] = {0};
    char tmp2[3]={0};

    strcpy(tmp, "CUWC,");
    strcat(tmp, UUID);
    strcat(tmp,",");
    sprintf(tmp2,"%d", configuration);
    strcat(tmp, tmp2);
    ble2_hal_send(tmp);
}

void  ble2_write_characteristic_content_via_UUID(char* UUID, char* content)
{
    char tmp[53] = {0};

    strcpy(tmp, "CUWV,");
    strcat(tmp,UUID);
    strcat(tmp, ",");
    strcat(tmp,content);
    ble2_hal_send(tmp);
}

void ble2_read_server_characteristic_value_via_handle (uint16_t handle)
{
    char tmp[13] = {0};
    char tmp2[5];

    strcpy(tmp, "SHR,");
    sprintf(tmp2,"%04X",handle);
    strcat(tmp,tmp2);
    ble2_hal_send(tmp);
}

void ble2_write_server_characteristic_value_via_handle (uint16_t handle, char* content)
{
    char tmp[23] = {0};
    char tmp2[5];

    strcat(tmp, "SHW,");
    sprintf(tmp2,"%04X",handle);
    strcat(tmp,tmp2);
    strcat(tmp, ",");
    strcat(tmp, content);
    ble2_hal_send(tmp);
}


void ble2_read_server_characteristic_value_via_UUID (char* UUID)
{
    char tmp[33] = {0};

    strcpy(tmp, "SUR,");
    strcat(tmp,UUID);
    ble2_hal_send(tmp);
}

void ble2_write_server_characteristic_value_via_UUID (char* UUID, char* content)
{
    char tmp[55] = {0};

    strcpy(tmp,"SUW,");
    strcat(tmp,UUID);
    strcat(tmp, ",");
    strcat(tmp,content);
    ble2_hal_send(tmp);
}

uint8_t ble2_set_private_characteristics(const char* UUID, uint8_t bitmap_characteristic, uint8_t max_data_len)
{
    char tmp[100] = {0};
    char tmp2[10];

    if (max_data_len > 20)
        return -1;

    strcpy(tmp, "PC,");
    strcat(tmp, UUID);
    sprintf(tmp2,",%02X",bitmap_characteristic);
    strcat(tmp,tmp2);
    sprintf(tmp2,",%02X",max_data_len);
    strcat(tmp,tmp2);
    ble2_hal_send(tmp);
    return 0;
}

void ble2_set_private_service_primary_filter_uuid(char* UUID)
{
    char tmp[33] = {0};

    strcpy(tmp, "PF,");
    strcat(tmp,UUID);
    ble2_hal_send(tmp);
}

void ble2_erase_private_service_primary_filter_uuid()
{
    ble2_hal_send("PF,Z");
}

void ble2_set_private_service_uuid(const char* uuid)
{
    char tmp[35] = {0};

    strcpy(tmp, "PS,");
    strcat(tmp,uuid);
    ble2_hal_send(tmp);
}

void ble2_private_service_clear_all_()
{
    ble2_hal_send("PZ");
}

void ble2_init_mldp_simulation_mode()
{
    ble2_hal_send("I");
}

int8_t ble2_mldp_set_security_mode(security_t additional_security)
{
    char tmp[10]= {0};
    char tmp2[3]= {0};

    if(additional_security > 2)
    {
        return -1;
    }
    strcpy(tmp,"SE,");
    sprintf(tmp2,"%1X",additional_security);
    strcat(tmp, tmp2);
    ble2_hal_send(tmp);

    return 0;
}


//***** scripting commands *****//

void ble2_list_current_script()
{
    ble2_hal_send("LW");
}

void ble2_clear_script()
{
    ble2_hal_send("WC");
}

void ble2_stop_script_execution()
{
    ble2_hal_send("WP");
}

void ble2_start_script_execution()
{
    ble2_hal_send("WR");
}

int8_t ble2_start_script_debug(debug_event_t debug_event)
{
    char tmp[10] = {0};
    char tmp2[3];

    if(debug_event>9)
    {
        return -1;
    }
    strcpy(tmp, "WR,");
    sprintf(tmp2,"%02X",debug_event);
    strcat(tmp, tmp2);
    ble2_hal_send(tmp);
    return 0;
}

void ble2_enter_script_input_mode()
{
    ble2_hal_send("WW");
}

void ble2_remote_command_enable(bool enable)
{
    char tmp[10] = {0};
    strcpy(tmp, "!,");
    strcat(tmp,enable ? "1" : "2");
    ble2_hal_send(tmp);
}

int8_t ble2_update_firmware(fw_update_t interface)
{
    char tmp[10] = {0};

    if(interface >2)
    {
        return -1;
    }
    strcpy(tmp, "~,");
    strcat(tmp, interface == 1 ?"1" : "2");
    ble2_hal_send(tmp);

    return 0;
}

/*************** END OF FUNCTIONS ***************************************************************************/
