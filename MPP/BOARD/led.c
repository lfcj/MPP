#include "led.h"


//=========================================================================
// Anschlussbelegung
//=========================================================================
//	LED_RT	PB8		TIM10C1
//	LED_GB	PB14	TIM12C1
//	LED_GR	PB15	TIM12C2
//=========================================================================


void init_LED(void){

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	GPIO_StructInit(&GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8 | GPIO_Pin_14 | GPIO_Pin_15 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//50MHz;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;//GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;//GPIO_OType_PP;
	GPIO_InitStructure.GPIO_OType =  GPIO_OType_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_Pin_8);
	GPIO_SetBits(GPIOB, GPIO_Pin_14);
	GPIO_SetBits(GPIOB, GPIO_Pin_15);
}
