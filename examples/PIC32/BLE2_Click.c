#include "ble2_hw.h"
#define INIT_BLE
#define MAX_BUFFER_LEN        0x40
#define CR  0x0D
#define LF  0x0A
#define TIME_OUT       50

// TFT module connections
char TFT_DataPort at LATE;
sbit TFT_RST at LATD7_bit;
sbit TFT_BLED at LATD2_bit;
sbit TFT_RS at LATD9_bit;
sbit TFT_CS at LATD10_bit;
sbit TFT_RD at LATD5_bit;
sbit TFT_WR at LATD4_bit;
char TFT_DataPort_Direction at TRISE;
sbit TFT_RST_Direction at TRISD7_bit;
sbit TFT_BLED_Direction at TRISD2_bit;
sbit TFT_RS_Direction at TRISD9_bit;
sbit TFT_CS_Direction at TRISD10_bit;
sbit TFT_RD_Direction at TRISD5_bit;
sbit TFT_WR_Direction at TRISD4_bit;
// End TFT module connections

sbit RN_WAKE at LATC1_BIT;
sbit RN_WAKE_DIR at TRISC1_BIT;
sbit RN_CMD at LATD0_BIT;
sbit RN_CMD_DIR at TRISD0_BIT;
sbit RN_CONN at RB8_BIT;
sbit RN_CONN_DIR at TRISB8_BIT;
//sbit LED at GPIOD_ODR.B0;

char rx_buff[MAX_BUFFER_LEN];
char data_len,data_ready;
char tmr_cnt,tmr_flg;
char batt_level;
char batt_level_txt[10];

void UART2interrupt() iv IVT_UART_2 ilevel 6 ics ICS_AUTO
{
   char tmprd;
   tmprd = UART2_Read();
   if(tmprd == LF) {
     rx_buff[data_len] = 0;
     data_len++;
     data_ready = 1;
   }else{
     rx_buff[data_len] = tmprd;
     data_len++;
   }
   U2RXIF_bit = 0;
}

//Timer2 Prescaler :1874; Preload = 63999; Actual Interrupt Time = 100 ms
void InitTimer2(){
  T2CON             = 0x8070;
  T2IP0_bit         = 1;
  T2IP1_bit         = 1;
  T2IP2_bit         = 1;
  T2IF_bit          = 0;
  T2IE_bit          = 1;
  PR2               = 39063;
  TMR2              = 0;
}

void Timer2Interrupt() iv IVT_TIMER_2 ilevel 7 ics ICS_SRS
{
   T2IF_bit = 0;
  tmr_cnt++;
  if (tmr_cnt >= TIME_OUT) {
     tmr_cnt = 0;
     tmr_flg = 1;
  }
}

void InitGPIO() {
    AD1PCFG = 0XFFFF;
    RN_WAKE_DIR = 0;
    RN_CMD_DIR = 0;
    RN_CONN_DIR = 1;
    RN_WAKE = 0;
    RN_CMD = 0;
}


void MCU_Init() {
  InitGPIO();
  delay_ms(5000);
  UART2_Init(115200);
  Delay_ms(100);
  ble2_hal_init();
  data_len = 0;
  data_ready = 0;
  U2IP0_bit = 0;
  U2IP1_bit = 1;
  U2IP2_bit = 1;
  U2RXIE_bit = 1;
  EnableInterrupts();
}

void Display_Init(){
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

void reset_buff()
{
   memset(rx_buff,0,data_len);
   data_len = 0;
   data_ready = 0;
}

void Display_BatteryLevel()
{
   char txt[3];
   ByteToStr(batt_level,txt);
   TFT_Set_Pen(CL_WHITE, 10);
   TFT_Set_Brush(1, CL_WHITE, 0, 0, 0, 0);
   TFT_Rectangle(150,100,230,115);
   TFT_Write_Text(txt,150,100);
   TFT_Write_Char(0x25,180,100);
}

void Display_Message()
{
   TFT_Set_Pen(CL_WHITE, 10);
   TFT_Set_Brush(1, CL_WHITE, 0, 0, 0, 0);
   TFT_Rectangle(150,120,300,140);
   TFT_Write_Text(rx_buff,150,120);
}

char wait_response(char *value)
{
   char result = 0;
   while(!data_ready);
   if(strstr(rx_buff,value))
   {
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

void main() {
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
  while(1)
  {
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