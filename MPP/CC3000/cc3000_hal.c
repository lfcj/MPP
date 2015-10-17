#include "spi.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_dma.h"
#include "misc.h"
#include "cc3000_common.h"
#include "stm32f4xx.h"

//=========================================================================
// Anschlussbelegung des CC3000 WLAN Transceivers
//=========================================================================
//	WLAN-Enable		PC13
// 	WLAN-IRQ		PA10
// 	SPI-CS			PC1
// 	SPI-CLK			PB10
// 	SPI-MISO		PC2
// 	SPI-MOSI		PC3
//=========================================================================


void init_CC3000_SPI(void) {
	unsigned long ulSpiIRQState;
	//=========================================================================
	// Clocksystem einschalten
	//=========================================================================
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;

	//=========================================================================
	// CC3000 PWR_EN Setup
	//=========================================================================

	// PWR_EN
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;		//TODO WLAN Enable
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);			//TODO WLAN Enable

	// WLAN PIN = 0 - CC3000 ausgeschaltet
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);				//TODO WLAN Enable

	//=========================================================================
	// CC3000 GPIO Setup
	//=========================================================================

	// CS - PB12
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// CS PIN = 1 - SPI nicht aktiv
	GPIO_SetBits(GPIOC, GPIO_Pin_1);

	// CLK - PB10
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_SPI2);

	// MOSI - PC3
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_SPI2);

	// MISO - PC2
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource2, GPIO_AF_SPI2);

	//=========================================================================
	// CC3000 SPI2 Setup
	//=========================================================================
	// Taktquelle RCC_APB1Periph_SPI2 mit 42Mhz geteilt durch 4 = SPI-CLK 10,5MHz

	SPI_InitTypeDef SPI_InitStructure;

	// SPI2
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge; //SPI_CPHA_1Edge
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2; //SPI_BaudRatePrescaler_4
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_Init(SPI2, &SPI_InitStructure);

	// Enable SPI
	SPI_Cmd(SPI2, ENABLE);


	//=========================================================================
	// CC3000 DMA Setup
	//=========================================================================

	// Structure auf default setzen
	DMA_InitTypeDef DMA_InitStruct;

	// RXD - DMA1 Channel 0 Stream3
	DMA_StructInit(&DMA_InitStruct);

	// RXD - DMA1 Channel 0 Stream4
	DMA_InitStruct.DMA_Channel = DMA_Channel_0;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & (SPI2->DR);
	DMA_InitStruct.DMA_Memory0BaseAddr = 0 /*NULL (uint32_t) wlan_rx_buffer*/; // TODO
	DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStruct.DMA_BufferSize = 0;						// TODO
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal; // DMA_Mode_Circular wenn man dauernd senden will;
	DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream3, &DMA_InitStruct);

	// Structure auf default setzen
	DMA_StructInit(&DMA_InitStruct);

	// TXD - DMA1 Channel 0 Stream4
	DMA_InitStruct.DMA_Channel = DMA_Channel_0;
	DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t) & (SPI2->DR);
	DMA_InitStruct.DMA_Memory0BaseAddr = 0 /* NULL (uint32_t) wlan_tx_buffer*/;	// TODO
	DMA_InitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStruct.DMA_BufferSize = 0;						// TODO
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Normal; // DMA_Mode_Circular wenn man dauernd senden will;
	DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream4, &DMA_InitStruct);

	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Rx, ENABLE);
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);

	// DMA Interrupt RXD - DMA1 Channel 0 Stream4 freigeben
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Stream 3 und 4 Disable
	DMA_Cmd(DMA1_Stream3, DISABLE);
	DMA_Cmd(DMA1_Stream4, DISABLE);
	DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);

	//=========================================================================
	// CC3000 IRQ Setup
	//=========================================================================

	// SPI_IRQ
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Interrupt Konfiguration
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource10);

	EXTI_InitTypeDef EXTI_SPIGPIO_InitStructure;
	EXTI_SPIGPIO_InitStructure.EXTI_Line = EXTI_Line10;
	EXTI_SPIGPIO_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_SPIGPIO_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_SPIGPIO_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_SPIGPIO_InitStructure);

	NVIC_InitTypeDef NVIC_InitSPIGPIOStructure;
	NVIC_InitSPIGPIOStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitSPIGPIOStructure.NVIC_IRQChannel = EXTI15_10_IRQn; //EXTI15_10_IRQn;
	NVIC_InitSPIGPIOStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitSPIGPIOStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_Init(&NVIC_InitSPIGPIOStructure);

	//=============================================================================
	// Anfangssynchronisation mit dem CC3000 Modul
	//=============================================================================
	// Warten ... IRQ Status lesen ... und dann Modul einschalten
	tSLInformation.WriteWlanPin(WLAN_DISABLE);
	wait_uSek(100000);
	ulSpiIRQState = tSLInformation.ReadWlanInterruptPin();
	tSLInformation.WriteWlanPin(WLAN_ENABLE);

	// wenn IRQ 1 war
	if ( ulSpiIRQState != 0 )
		{

		// und warten auf IRQ-Flanke HL
		while(tSLInformation.ReadWlanInterruptPin() != 0) { ; }

		}

	// wenn IRQ 0 war
	else
		{

		//dann WLAN-EN HLH-Flanke mit kurzem Warten
		tSLInformation.WriteWlanPin(WLAN_DISABLE);
		wait_uSek(1000000);
		tSLInformation.WriteWlanPin(WLAN_ENABLE);

		// und warten auf IRQ-Flanke LH
		while(tSLInformation.ReadWlanInterruptPin() != 0) { ; }

		// dann nochmal WLAN-EN HLH-Flanke mit kurzem Warten
		tSLInformation.WriteWlanPin(WLAN_DISABLE);
		wait_uSek(1000000);
		tSLInformation.WriteWlanPin(WLAN_ENABLE);

		// und warten auf IRQ-Flanke HL
		while(tSLInformation.ReadWlanInterruptPin() == 0) { ; }

		}
	// jetzt ist CC3000 bereit
	//=============================================================================
}



