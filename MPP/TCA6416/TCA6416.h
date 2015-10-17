#ifndef __TCA6416_H__
#define __TCA6416_H__

//=========================================================================
// Anschlussbelegung
//=========================================================================
//	/RESET	PC0		0-Reset 	1-normal
//	/INT	PC8		/INT wird bei jeder Pegeländerung generiert
// 	SCL		PA8		SCL3
//	SDA		PC9 	SDA3
//=========================================================================
//	Datenblatt: http://www.ti.com/lit/ds/symlink/tca6416a.pdf
//=========================================================================

//=========================================================================
// cmsis_lib
//=========================================================================
#include "stm32f4xx.h"
#include "misc.h"
//#include "stm32f4xx_adc.h"
//#include "stm32f4xx_can.h"
//#include "stm32f4xx_crc.h"
//#include "stm32f4xx_cryp_aes.h"
//#include "stm32f4xx_cryp_des.h"
//#include "stm32f4xx_cryp_tdes.h"
//#include "stm32f4xx_cryp.h"
//#include "stm32f4xx_dac.h"
//#include "stm32f4xx_dbgmcu.h"
//#include "stm32f4xx_dcmi.h"
//#include "stm32f4xx_dma.h"
#include "stm32f4xx_exti.h"
//#include "stm32f4xx_flash.h"
//#include "stm32f4xx_fsmc.h"
#include "stm32f4xx_gpio.h"
//#include "stm32f4xx_hash_md5.h"
//#include "stm32f4xx_hash_sha1.h"
//#include "stm32f4xx_hash.h"
#include "stm32f4xx_i2c.h"
//#include "stm32f4xx_iwdg.h"
//#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
//#include "stm32f4xx_rng.h"
//#include "stm32f4xx_rtc.h"
//#include "stm32f4xx_sdio.h"
//#include "stm32f4xx_spi.h"
#include "stm32f4xx_syscfg.h"
//#include "stm32f4xx_tim.h"
//#include "stm32f4xx_usart.h"
//#include "stm32f4xx_wwdg.h"


//=========================================================================
// board_lib
//=========================================================================
//#include "beeper.h"
//#include "clock.h"
#include "i2c.h"
//#include "init.h"
//#include "interrupts.h"
//#include "led.h"
//#include "power.h"
//#include "taster.h"
#include "usart.h"


//#include "CC1101.h"
//#include "CC3000.h"
//#include "spi.h"
//#include "MPU9150.h"
//#include "SDCARD.h"
//#include "TCA6416.h"

//#include "usbd_cdc_vcp.h"
//#include "usb_conf.h"

//=========================================================================
// standart_lib
//=========================================================================

//#include "CoOS.h"

//#include "spi.h"
#include "stdio.h"
#include "string.h"


//=============================================================================
#define SLAVE_ADDRESS 	(0x20)
//=============================================================================

//=============================================================================
// Register und Command Byte
//=============================================================================
//===Input======================RD
#define InputPort0				(0x00)
#define InputPort1				(0x01)

//===Output=====================RD/WR
#define OutputPort0				(0x02)
#define OutputPort1				(0x03)

//===PolarityInversion==========RD/WR
//	Invertierung des Inputsignals - Bit: 0-Originalwert; 1-Invertierter Wert
#define PolarityInversionPort0	(0x04)
#define PolarityInversionPort1	(0x05)

//===Configuration==============RD/WR
// 	Richtungswahl der Portleitungen - Bit: 0-Output 1-InputRicht
#define ConfigurationPort0		(0x06)
#define ConfigurationPort1		(0x07)


//=============================================================================
// Structure für die Datenregister des TCA6416
//=============================================================================
typedef struct TCA6416
	{
	unsigned char input_port0;
	unsigned char input_port1;
	unsigned char output_port0;
	unsigned char output_port1;
	}
	TCA6416_DAT;
extern volatile  TCA6416_DAT TCA6416_DATA;


//=============================================================================
// Funktionen
//=============================================================================
void init_TAC6416_I2C3(void);
//	initialisiert die I/O und I2C3 Schnittstelle

//=============================================================================
void reset_TAC6416(void);
//	Reset der TAC6416 Logik (alles Input nicht invertiert)

//=============================================================================
void ISR_IRQ8_TAC6416(void);
//	ISR für deb TCA6416 ist dem EXTI9_5_IRQHandler zuzuordnen

//=============================================================================


//=============================================================================
//	Beispiel Outputports lesen
//=============================================================================
//	I2C_start(I2C3, SLAVE_ADDRESS, I2C_Direction_Transmitter);
//	I2C_write(I2C3,OutputPort0);
//	I2C_stop(I2C3);
//	I2C_start(I2C3, SLAVE_ADDRESS, I2C_Direction_Receiver);
//	TCA6416_DATA.output_port0 = I2C_read_ack(I2C3);
//	TCA6416_DATA.output_port1 = I2C_read_nack(I2C3);
//	I2C_stop(I2C3);
//=============================================================================
//	Beispiel Outputports schreiben
//=============================================================================
//	I2C_start(I2C3, SLAVE_ADDRESS, I2C_Direction_Transmitter);
//	I2C_write(I2C3,OutputPort0);
//	I2C_write(I2C3,TCA6416_DATA.output_port0);
//	I2C_write(I2C3,TCA6416_DATA.output_port1);
//	I2C_stop(I2C3);
//=============================================================================
#endif // __TCA6416_H__
