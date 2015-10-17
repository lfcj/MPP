#ifndef RTC_H_
#define RTC_H_

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
//#include "stm32f4xx_i2c.h"
//#include "stm32f4xx_iwdg.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_rng.h"
#include "stm32f4xx_rtc.h"
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
//#include "i2c.h"
//#include "init.h"
//#include "interrupts.h"
#include "spi.h"
#include "led.h"
//#include "power.h"
//#include "taster.h"
#include "usart.h"

//#include "CC1101.h"
//#include "CC3000.h"
//#include "SDCARD.h"
//#include "TCA6416.h"

//#include "usbd_cdc_vcp.h"
//#include "usb_conf.h"
#include "ntp.h"

//=========================================================================
// standart_lib
//=========================================================================

//#include "CoOS.h"

//#include "spi.h"
#include "stdio.h"
#include "string.h"
#include <stdint.h>
#include <stdbool.h>
#include "time.h"

#define ABSTAND_1970_2000 (946684800);

typedef struct {
	uint16_t year;	/* 1..4095 */
	uint8_t month; 	/* 1..12 */
	uint8_t mday; 	/* 1..31 */
	uint8_t wday; 	/* 0..6, Sunday = 0*/
	uint8_t hour; 	/* 0..23 */
	uint8_t min; 	/* 0..59 */
	uint8_t sec; 	/* 0..59 */
	uint8_t dst; 	/* 0 Winter, !=0 Summer */
} RTC_t;

typedef enum { TIMEBASE_1970 = 0,TIMEBASE_1980 = 1,TIMEBASE_2000= 2, TIMEBASE_1900=3} TimeBase;


extern RTC_TimeTypeDef ntp_sync_startTime;
extern RTC_TimeTypeDef ntp_sync_stop_Time;

extern RTC_DateTypeDef ntp_sync_startDate;
extern RTC_DateTypeDef ntp_sync_stop_Date;

void test_rtc_convert(void); // TEST

uint8_t start_RTC(void);
void init_RTC(void);
void RTC_IRQ_Handler(void);

void show_RTC_Time(void);
void show_RTC_Date(void);
void show_RTC_Alarm(void);

void set_RTC_from_NTPsec(uint64_t sec, int8_t Zeitzone);

_Bool set_RTC_Alarm_in(uint8_t Tagen, uint8_t Std, uint8_t Min, uint8_t Sek, void (*callback)(void));

void start_RTC_Alarm(void);

uint32_t rtc_getSek(TimeBase tb);
uint32_t convert_RTC_struct_to_sek(const RTC_TimeTypeDef *t, RTC_DateTypeDef *d, TimeBase tb);
int convert_RTC_sek_to_struct(uint32_t sec, RTC_TimeTypeDef *t, RTC_DateTypeDef *d, TimeBase tb);
//void convert_RTC_TimeDate_to_RTC_t(RTC_TimeTypeDef* t, RTC_DateTypeDef* d, RTC_t * r);

uint32_t Zufallszahl(void);

#endif
