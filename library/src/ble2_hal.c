/*******************************************************************************
* Title                 :   BLE 2 HAL
* Filename              :   ble2_hal.c
* Author                :   VM
* Origin Date           :   02/12/2015
* Notes                 :   None
*******************************************************************************/
/*************** MODULE REVISION LOG ******************************************
*
*    Date    Software Version    Initials   Description
*  11/05/16    1.00                VM      Module Created.
*
*******************************************************************************/
/**
 * @brief This module contains the hal layer for Mikroelektronika's BLE
 *  click board.
 */
/******************************************************************************
* Includes
*******************************************************************************/
#include "ble2_hal.h"
/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/
#define STM32
/******************************************************************************
* Module Typedefs
*******************************************************************************/
#if defined ( __MIKROC_PRO_FOR_ARM__ )       || \
    defined ( __MIKROC_PRO_FOR_PIC32__ )
static void ( *write_uart_p )( unsigned int _data );
static void ( *write_text_p )( unsigned char *_data );
#elif defined ( __MIKROC_PRO_FOR_PIC__ )     || \
      defined ( __MIKROC_PRO_FOR_FT90x__ )
static void ( *write_uart_p )( unsigned char _data );
static void ( *write_text_p )( unsigned char *_data );
#elif defined (ARDUINO_ARCH_AVR)
#include <string.h>
void UART_Wr_Ptr(unsigned char _data);
void UART_Write_Text(unsigned char *_data);
static void ( *write_uart_p )( unsigned char _data );
static void ( *write_text_p )( unsigned char *_data );
#endif
/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
//static write_uart_t _hal_uart_write;

#if defined( __MIKROC_PRO_FOR_ARM__ )      || \
       defined( __MIKROC_PRO_FOR_AVR__ )   || \
       defined( __MIKROC_PRO_FOR_PIC__ )   || \
       defined( __MIKROC_PRO_FOR_PIC32__ ) || \
       defined( __MIKROC_PRO_FOR_DSPIC__ ) || \
       defined( __MIKROC_PRO_FOR_8051__ )  || \
       defined( __MIKROC_PRO_FOR_FT90x__ )

#endif
/******************************************************************************
* Function Prototypes
*******************************************************************************/

/******************************************************************************
* Function Definitions
*******************************************************************************/
void ble2_hal_init()
{
    write_uart_p = UART_Wr_Ptr;
    write_text_p = UART_Write_Text;
}

void ble2_hal_send( char *data_in )
{
   strcat( data_in, "\n" );
   write_text_p(data_in);
}

/*************** END OF FUNCTIONS ***************************************************************************/
