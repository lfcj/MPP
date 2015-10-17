#ifndef __AUFGABE_H__
#define __AUFGABE_H__

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
//#include "stm32f4xx_gpio.h"
//#include "stm32f4xx_hash_md5.h"
//#include "stm32f4xx_hash_sha1.h"
//#include "stm32f4xx_hash.h"
//#include "stm32f4xx_i2c.h"
#include "stm32f4xx_iwdg.h"
//#include "stm32f4xx_pwr.h"
//#include "stm32f4xx_rcc.h"
//#include "stm32f4xx_rng.h"
//#include "stm32f4xx_rtc.h"
//#include "stm32f4xx_sdio.h"
//#include "stm32f4xx_spi.h"
//#include "stm32f4xx_syscfg.h"
//#include "stm32f4xx_tim.h"
//#include "stm32f4xx_usart.h"
#include "stm32f4xx_wwdg.h"


//=========================================================================
// board_lib
//=========================================================================


//=========================================================================
// standart_lib
//=========================================================================
//#include "spi.h"
//#include "stdio.h"
//#include "string.h"


//=========================================================================
// CooCox CoOS
//=========================================================================
//#include "CoOS.h"


//=========================================================================
// Eigene Funktionen, Macros und Variablen
//=========================================================================
//========== Macros


//========== Variablen
extern char usart2_rx_buffer[50];
extern char usart2_tx_buffer[50];

//========== Funktionen
void init_leds(void);
void init_taste_1(void);
void init_taste_2(void);
void init_PC09(void);
void init_usart_2_tx(void);
void print_usart_2(char* zeichenkette);
void init_usart_2(void);
void init_taste_1_irq(void);
void init_taste_2_irq(void);
void init_usart_2_irq_rx(void);



#endif
