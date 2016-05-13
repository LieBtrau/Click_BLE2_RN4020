/*
Development sistem: EasyMX PRO v7
MCU: STM32F107VC 72MHz
BLE2 click on mikrobus socket 2
*/
#include "ble2_hw.h"
#define INIT_BLE
#define MAX_BUFFER_LEN        0x40
#define LF  0x0A
#define TIME_OUT       5    //5 seconds
#define INIT_BLE

// TFT module connections
unsigned int TFT_DataPort at GPIOE_ODR;
sbit TFT_RST at GPIOE_ODR.B8;
sbit TFT_RS at GPIOE_ODR.B12;
sbit TFT_CS at GPIOE_ODR.B15;
sbit TFT_RD at GPIOE_ODR.B10;
sbit TFT_WR at GPIOE_ODR.B11;
sbit TFT_BLED at GPIOE_ODR.B9;
// End TFT module connections

sbit RN_WAKE at GPIOC_ODR.B3;
sbit RN_CMD at GPIOD_ODR.B12;
sbit RN_CONN at GPIOA_IDR.B5;
sbit LED at GPIOD_ODR.B0;


uint8_t rx_buff[MAX_BUFFER_LEN];
char data_len,data_ready;
char tmr_cnt,tmr_flg;
char batt_level;
char batt_level_txt[10];



void UART2_Interrupt() iv IVT_INT_USART2 ics ICS_AUTO {
char   tmp = UART2_Read();
   if(tmp == LF) {
     rx_buff[data_len] = 0;
     data_len++;
     data_ready = 1;
   }else{
     rx_buff[data_len] = tmp;
     data_len++;
   }
}

//Timer2 Prescaler :1874; Preload = 63999; Actual Interrupt Time = 1sec

void InitTimer2(){
  RCC_APB1ENR.TIM2EN = 1;
  TIM2_CR1.CEN = 0;
  TIM2_PSC = 1874;
  TIM2_ARR = 63999;
  NVIC_IntEnable(IVT_INT_TIM2);
  TIM2_DIER.UIE = 1;
  TIM2_CR1.CEN = 1;

}

void Timer2_interrupt() iv IVT_INT_TIM2 {
  TIM2_SR.UIF = 0;
  tmr_cnt++;
  if (tmr_cnt >= TIME_OUT) {
     tmr_cnt = 0;
     tmr_flg = 1;
  }
}

void InitGPIO()
{
  GPIO_Digital_Output(&GPIOC_ODR, _GPIO_PINMASK_3); //PC3 - RN_WAKE
  GPIO_Digital_Output(&GPIOD_ODR, _GPIO_PINMASK_12 | _GPIO_PINMASK_0); //PD12 - RN_CMD
  GPIO_Digital_Input(&GPIOA_IDR, _GPIO_PINMASK_5);  //PA5 - RN_CONN
  RN_WAKE = 0;
  RN_CMD = 0;

}


void MCU_Init()
{
  InitGPIO();
  Uart1_Init(115200);
  Uart1_Write_Text("Uart initialized \r\n");
  delay_ms(500);
  UART2_Init(115200);
  Delay_ms(100);
  ble2_hal_init();
  data_len = 0;
  data_ready = 0;
  RXNEIE_USART2_CR1_bit = 1;
  NVIC_IntEnable(IVT_INT_USART2);
  EnableInterrupts();
}

void Display_Init()
{
  TFT_Init_ILI9341_8bit(320, 240);
  TFT_BLED = 1;
}

void DrawFrame()
{
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