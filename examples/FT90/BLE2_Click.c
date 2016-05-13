#include <built_in.h>
#include "ble2_hw.h"
#define INIT_BLE
#define MAX_BUFFER_LEN        0x40
#define LF  0x0A

void Auxiliary_Code(void);
void Set_Index(unsigned short index);
void Write_Command(unsigned short cmd);
void Write_Data(unsigned int _data);

// TFT module connections
char TFT_DataPort at GPIO_PORT_32_39;
sbit TFT_RST at GPIO_PIN42_bit;
sbit TFT_RS at GPIO_PIN40_bit;
sbit TFT_CS at GPIO_PIN41_bit;
sbit TFT_RD at GPIO_PIN46_bit;
sbit TFT_WR at GPIO_PIN47_bit;
sbit TFT_BLED at GPIO_PIN58_bit;
// End TFT module connections

void Auxiliary_Code() {
  TFT_WR = 0;
  asm nop;
  TFT_WR = 1;
}

void Set_Index(unsigned short index) {
  TFT_RS = 0;
  asm nop;
  TFT_DataPort = index;
  asm nop; asm nop;
  Auxiliary_Code();
}

void Write_Command(unsigned short cmd) {
  TFT_RS = 1;
  asm nop;
  TFT_DataPort = cmd;
  asm nop; asm nop;
  Auxiliary_Code();
}

void Write_Data(unsigned int _data) {
  TFT_RS = 1;
  asm nop;
  TFT_DataPort = Hi(_data);
  asm nop; asm nop;
  Auxiliary_Code();
  asm nop;
  TFT_DataPort = Lo(_data);
  asm nop; asm nop;
  Auxiliary_Code();
}

sbit RN_WAKE at GPIO_PIN1_BIT;
sbit RN_CMD at GPIO_PIN56_BIT;
sbit RN_CONN at GPIO_PIN9_BIT;

char rx_buff[MAX_BUFFER_LEN];
char data_len,data_ready;
char tmr_cnt,tmr_flg;
char batt_level;
char batt_level_txt[10];

#define TIME_OUT       5    //5 seconds
#define INIT_BLE

void UART_RX_ISR() iv IVT_UART2_IRQ ics ICS_AUTO
{
   char tmp_read;
   tmp_read = UART2_Read();
   if(tmp_read == LF) {
     rx_buff[data_len] = 0;
     data_len++;
     data_ready = 1;
   }else{
     rx_buff[data_len] = tmp_read;
     data_len++;
   }
}

//Place/Copy this part in declaration section
void InitTimer2()
{
  TIMER_CONTROL_0 = 2;
  TIMER_SELECT = 1;
  TIMER_PRESC_LS = 36;
  TIMER_PRESC_MS = 244;
  TIMER_WRITE_LS = 63;
  TIMER_WRITE_MS = 6;
  TIMER_CONTROL_3 = 0;
  TIMER_CONTROL_4 |= 18;
  TIMER_CONTROL_2 |= 32;
  TIMER_INT |= 8;
  TIMER_CONTROL_1 |= 2;
}


void Timer2_interrupt() iv IVT_TIMERS_IRQ
{
  tmr_cnt++;
  if (tmr_cnt >= TIME_OUT) {
     tmr_cnt = 0;
     tmr_flg = 1;
  }
    if (TIMER_INT_B_bit){
    TIMER_INT = (TIMER_INT & 0xAA) | (1 << 2);
  }
}

void InitGPIO()
{
  GPIO_Digital_Output( &GPIO_PORT_00_07, _GPIO_PINMASK_1 );
  GPIO_Digital_Output( &GPIO_PORT_48_55, _GPIO_PINMASK_1 );
  GPIO_Digital_Input( &GPIO_PORT_08_15, _GPIO_PINMASK_1 );
  RN_WAKE = 0;
  RN_CMD = 0;
}


void MCU_Init() {
  InitGPIO();
  delay_ms(500);
  UART2_Init_Advanced( 115200, _UART_DATA_BITS_8,
                                 _UART_PARITY_NONE,
                                 _UART_STOP_BITS_1 );
  Delay_ms(100);
  ble2_hal_init();
  data_len = 0;
  data_ready = 0;

  UART2_FCR |= ( 1 << FIFO_EN ) | ( 1 << RCVR_RST );
  UART2_INT_ENABLE |= ( 1 << RX_AVL_EN );

  IRQ_CTRL &= ~( 1 << GLOBAL_INTERRUPT_MASK );
}

void Display_Init(){
  TFT_Set_Active(Set_Index, Write_Command, Write_Data);
  TFT_Init_ILI9341_8bit(320, 240);
  TFT_BLED = 1;
}

void DrawFrame() {
  TFT_Fill_Screen(CL_WHITE);
  TFT_Set_Pen(CL_BLACK, 1);
  TFT_Line(20, 220, 300, 220);
  TFT_Line(20,  46, 300,  46);
  TFT_Set_Font(&TFT_defaultFont, CL_RED, FO_HORIZONTAL);
  TFT_Write_Text("BLE2  Click  Board  Demo", 25, 14);
  TFT_Set_Font(&TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
  TFT_Write_Text("EasyMx PRO v7", 19, 223);
  TFT_Set_Font(&TFT_defaultFont, CL_RED, FO_HORIZONTAL);
  TFT_Write_Text("www.mikroe.com", 200, 223);
  TFT_Set_Font(&TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
  TFT_Write_Text("BATTERY SERVICE", 40, 60);
  TFT_Write_Text("BATTERY LEVEL:", 40, 100);
  TFT_Write_Text("RECEIVE:", 40, 120);
}

void reset_buff() {
   memset(rx_buff,0,data_len);
   data_len = 0;
   data_ready = 0;
}

char send_succcess() {
   char success = 0;
   while (!data_ready);
   if(strstr(rx_buff,"AOK")) {
     success = 1;
   }
   reset_buff();
   return success;
}

void Display_BatteryLevel() {
   char txt[3];
   ByteToStr(batt_level,txt);
   TFT_Set_Pen(CL_WHITE, 10);
   TFT_Set_Brush(1, CL_WHITE, 0, 0, 0, 0);
   TFT_Rectangle(150,100,230,115);
   TFT_Write_Text(txt,150,100);
   TFT_Write_Char(0x25,180,100);
}

void Display_Message() {
   TFT_Set_Pen(CL_WHITE, 10);
   TFT_Set_Brush(1, CL_WHITE, 0, 0, 0, 0);
   TFT_Rectangle(150,120,300,140);
   TFT_Write_Text(rx_buff,150,120);
}

char wait_response(char *value) {
   char result = 0;
   while(!data_ready);
   if(strstr(rx_buff,value)) {
     result = 1;
   }
   memset(rx_buff,0,data_len);
   data_len = 0;
   data_ready = 0;
   return result;
}

void BLE_Init()
{
  unsigned int line_no;
   line_no = 10;
  TFT_Write_Text("Start initialization",10,line_no);
  RN_WAKE = 1;
  while (!wait_response("CMD"));                                                //Wait to response CMD
  line_no += 20;
  Delay_ms(100);
  ble2_reset_to_factory_default(1);                                             //Reset RN4020 to factory settings
  while (!wait_response("AOK"));                                                //Wait to response AOK
  TFT_Write_Text("Factory reset",10,line_no);
  line_no += 20;
  Delay_ms(100);
  TFT_Write_Text("Set module name: BLE2_Click",10,line_no);
  ble2_set_device_name("BLE2_Click");                                           //Set name BLE2_Click
  while (!wait_response("AOK"));                                                //Wait to response AOK
  line_no += 20;
  Delay_ms(100);
  TFT_Write_Text("Configure services",10,line_no);

  /*
  Example:
  Table: 2-6 in Datasheet
  Health Thermometer + Heart Rate + Battery Services
  UART_WRite_Line("SS,F0000000");
  */

  ble2_set_server_services(40000000);                                           //Battery
  while (!wait_response("AOK"));                                                //Wait to response AOK
  line_no += 20;
  Delay_ms(100);
  TFT_Write_Text("Configure features",10,line_no);
  ble2_set_supported_features(20000000);                                        //Auto Advertise (Table: 2-5 in Datasheet)
  while (!wait_response("AOK"));                                                //wait to response AOK
  line_no += 20;
  Delay_ms(100);
  TFT_Write_Text("Reboot module",10,line_no);
  ble2_device_reboot();                                                       //reboot
  while (!wait_response("Reboot"));                                             //wait to response Reboot
}

void main()
{
  Display_Init();
  MCU_Init();
  #ifdef INIT_BLE
  BLE_Init();
  delay_ms(2000);
  #else
  RN_WAKE = 1;
  wait_response("CMD");
  #endif
  DrawFrame();
  InitTimer2();
  while(1) {
    if(data_ready)
    {
      //If characteristic is configured as write
      //received messages come here
      Display_Message();
      reset_buff();
    }
    else
    {
       //Test: every 5sec increase baterry level (0 to 100%)
       //and send value via Bluetooth Low Energy
       if (tmr_flg)
       {
          batt_level++;
          if(batt_level > 100)
          {
            batt_level = 0;
          }
          Display_BatteryLevel();
          if(RN_CONN)
          { //send battery level value if BLE connected
            shorttohex(batt_level, batt_level_txt);
            ltrim(batt_level_txt);
            ble2_write_server_characteristic_value_via_UUID("2A19",batt_level_txt);
          }
          tmr_flg = 0;
       }
    }
  }
}