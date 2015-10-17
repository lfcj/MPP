#ifndef __LED_H__
#define __LED_H__


//=========================================================================
// Anschlussbelegung
//=========================================================================
//	LED_RT	PB8		TIM10C1
//	LED_GB	PB14	TIM12C1
//	LED_GR	PB15	TIM12C2
//=========================================================================


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
//#include "stm32f4xx_i2c.h"
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
//#include "usart.h"


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

//#include "CoOS.h"

//#include "spi.h"
//#include "stdio.h"
//#include "string.h"


#define LED_RT_ON		( GPIO_ResetBits(GPIOB, GPIO_Pin_8) )
#define LED_RT_OFF		( GPIO_SetBits(GPIOB, GPIO_Pin_8) )
#define LED_RT_TOGGLE	( GPIO_ToggleBits(GPIOB, GPIO_Pin_8) )

#define LED_GE_ON		( GPIO_ResetBits(GPIOB, GPIO_Pin_14) )
#define LED_GE_OFF		( GPIO_SetBits(GPIOB, GPIO_Pin_14) )
#define LED_GE_TOGGLE	( GPIO_ToggleBits(GPIOB, GPIO_Pin_14) )

#define LED_GR_ON		( GPIO_ResetBits(GPIOB, GPIO_Pin_15) )
#define LED_GR_OFF		( GPIO_SetBits(GPIOB, GPIO_Pin_15) )
#define LED_GR_TOGGLE	( GPIO_ToggleBits(GPIOB, GPIO_Pin_15) )

void init_LED(void);


#endif // __LED_H_
