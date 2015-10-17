#ifndef __INTERRUPTS_H__
#define __INTERRUPTS_H__



//=========================================================================
// cmsis_lib
//=========================================================================
#include "stm32f4xx.h"
#include "misc.h"
#include "stm32f4xx_adc.h"
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
//#include "stm32f4xx_i2c.h"
//#include "stm32f4xx_iwdg.h"
//#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
//#include "stm32f4xx_rng.h"
#include "stm32f4xx_rtc.h"
//#include "stm32f4xx_sdio.h"
//#include "stm32f4xx_spi.h"
#include "stm32f4xx_syscfg.h"
//#include "stm32f4xx_tim.h"
//#include "stm32f4xx_usart.h"
#include "stm32f4xx_wwdg.h"

//=========================================================================
// board_lib
//=========================================================================
//#include "beeper.h"
//#include "clock.h"
//#include "i2c.h"
//#include "init.h"
//#include "interrupts.h"
#include "led.h"
//#include "power.h"
//#include "taster.h"
//#include "timer.h"
#include "usart.h"
//#include "aufgabe.h"


//=================
#include "arch.h"// CoOS Systick einbinden
//=================

#include "CC1101.h"
#include "CC3000.h"
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

extern int32_t timer;
extern int32_t Tas1;
extern int32_t Tas2;
extern int16_t T1;
extern int16_t T2;

extern char tasten[50];

#endif // __INTERRUPTS_H__
