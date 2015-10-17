#ifndef __I2C_H__
#define __I2C_H__

//=========================================================================
// cmsis_lib
//=========================================================================
#include "stm32f4xx.h"
//#include "misc.h"
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
//#include "stm32f4xx_exti.h"
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
//#include "stm32f4xx_syscfg.h"
//#include "stm32f4xx_tim.h"
//#include "stm32f4xx_usart.h"
//#include "stm32f4xx_wwdg.h"


//=========================================================================
// board_lib
//=========================================================================
//#include "beeper.h"
//#include "clock.h"
//#include "i2c.h"
//#include "init.h"
//#include "interrupts.h"
//#include "led.h"
//#include "power.h"
//#include "taster.h"
//#include "timer.h"
#include "usart.h"


//#include "CC1101.h"
//#include "CC3000.h"
//#include "MPU9150.h"
//#include "SDCARD.h"
//#include "TCA6416.h"

//#include "usbd_cdc_vcp.h"
//#include "usb_conf.h"


//=========================================================================
// standart_lib
//=========================================================================

#include "stdio.h"
#include "string.h"


//=============================================================================
int I2C_write_buf(	unsigned char slave_addr,
					unsigned char reg_addr,
					unsigned char length,
					unsigned char const *data);
//

//=============================================================================
int I2C_read_buf(	unsigned char slave_addr,
					unsigned char reg_addr,
					unsigned char length,
					unsigned char *data);
//

//=============================================================================
int I2C_start(I2C_TypeDef* I2Cx,
				uint8_t addr,
				uint8_t rdwr);
//	sendet Start Condition + Slave Adresse und RW-Bit

//=============================================================================
void I2C_stop(I2C_TypeDef* I2Cx);
//	Stop Condition

//=============================================================================
int I2C_write(I2C_TypeDef* I2Cx,
				uint8_t data);
//	sendet ein Byte an den I2C-Slave

//=============================================================================
uint8_t I2C_read_ack(I2C_TypeDef* I2Cx);
//	empfängt ein Byte vom I2C-Slave und generiert ein ACK

//=============================================================================
uint8_t I2C_read_nack(I2C_TypeDef* I2Cx);
//	empfängt ein Byte vom I2C-Slave und generiert kein ACK

//=============================================================================
int I2C_restart(I2C_TypeDef* I2Cx,
				uint8_t addr,
				uint8_t rdwr);
//

//=============================================================================
int I2C_check_dev(I2C_TypeDef* I2Cx,
					uint8_t addr);
//

//=============================================================================
int I2C_timeout(char *msg);
// gibt eine Fehlermeldung aus


#endif // __I2C_H__
