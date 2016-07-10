#include "rn4020.h"
#include "ble2_hw.h"

static SoftwareSerial* sPort;

#ifdef __cplusplus
extern "C"{
#endif
void UART_Wr_Ptr(unsigned char _data)
{
    sPort->write(_data);
}

void UART_Write_Text(unsigned char *_data)
{
    sPort->print((const char*)_data);
}
#ifdef __cplusplus
} // extern "C"
#endif

rn4020::rn4020(SoftwareSerial &s, byte pinCts, byte pinWake_sw, byte pinWake_hw, byte pinEnPwr, byte pinBtConnected):
    _pinCts(pinCts),
    _pinWake_sw(pinWake_sw),
    _pinWake_hw(pinWake_hw),
    _pinEnPwr(pinEnPwr),
    _pinConnection(pinBtConnected)
{
    sPort=&s;
    ble2_hal_init();
}

bool rn4020::begin(bool bCentral)
{
    reset();
    sPort->begin(115200);
    if(bCentral)
    {
	//Central
	ble2_reset_to_factory_default(RESET_SOME);
	ble2_set_server_services(0xC0000000);
	ble2_set_supported_features(0x82480000);
	ble2_device_reboot();
    }else
    {
        //Peripheral
        //ble2_reset_to_factory_default(RESET_SOME);
        //ble2_set_server_services(0xC0000000);
        while(1);
//        ble2_set_supported_features(0x02480000);
//        ble2_set_baud_rate(BR_2400);
//        ble2_set_transmission_power(PWR_MIN19_1dBm);
//        ble2_device_reboot();
    }
}

void rn4020::getResponse(unsigned int uiTimeout, String &readData)
{
    sPort->setTimeout(uiTimeout);
    readData=sPort->readStringUntil('\n');
}

void rn4020::reset()
{
    pinMode(_pinEnPwr, OUTPUT);
    digitalWrite(_pinEnPwr, LOW);
    pinMode(_pinWake_hw, OUTPUT);
    digitalWrite(_pinWake_hw, LOW);
    pinMode(_pinWake_sw, OUTPUT);
    digitalWrite(_pinWake_sw, LOW);
    pinMode(_pinCts, OUTPUT);
    digitalWrite(_pinCts,LOW);
    pinMode(_pinConnection, INPUT);
    delay(500);
    digitalWrite(_pinEnPwr, HIGH);
    //1160ms after power up, there's a power dip that causes to reset the module.  The power dip can be avoided
    //by powering the RN4020 through a low Rds FET like the DMP2035 instead of powering it directly through a
    //GPIO of the MCU.
    delay(1500);
    for(byte i=0;i<3;i++)
    {
	digitalWrite(_pinWake_hw, HIGH);
	delay(100);
	digitalWrite(_pinWake_hw, LOW);
	delay(100);
    }
}
