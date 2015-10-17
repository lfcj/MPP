#include "spi.h"
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_dma.h"
#include "misc.h"
#include "cc3000_common.h"
#include "cc3000_hal.h"
#include "hci.h"
#include "main.h"
#include "hci.h"
#include "string.h"

#define bool _Bool

#ifndef FALSE
#define FALSE false
#define TRUE true
#endif

typedef struct {
	gcSpiHandleRx SPIRxHandler;
	unsigned short usTxPacketLength;
	unsigned short usRxPacketLength;
	volatile unsigned long ulSpiState;
	unsigned char *pTxPacket;
	unsigned char *pRxPacket;
//tSpiHwConfiguration	sHwSettings;
} tSpiInformation;

typedef struct __attribute__ ((__packed__)) _btspi_hdr {
	unsigned char cmd;
	unsigned short length;
	unsigned char pad[2];
} btspi_hdr;

typedef struct _hci_hdr_t {
	unsigned char ucType;
	unsigned char ucOpcode;
	unsigned char pad[3];
} hci_hdr_t;

typedef struct _hci_data_hdr_t {
	unsigned char ucType;
	unsigned char ucOpcode;
	unsigned char ucArgsize;
	unsigned short usLength;
} hci_data_hdr_t;

typedef struct _hci_data_cmd_hdr_t {
	unsigned char ucType;
	unsigned char ucOpcode;
	unsigned char ucArgLength;
	unsigned short usTotalLength;
} hci_data_cmd_hdr_t;

typedef struct _hci_evnt_hdr_t {
	unsigned char ucType;
	unsigned short usEvntOpcode;
	unsigned char ucLength;
	unsigned char ucStatus;
} hci_evnt_hdr_t;

// UTIL - werden eingesetzt
#define HI(value)			(((value) & 0xFF00) >> 8)
#define LO(value)           ((value) & 0x00FF)
#define ASSERT_CS()         GPIO_ResetBits(GPIOC, GPIO_Pin_1)
#define DEASSERT_CS()       GPIO_SetBits(GPIOC, GPIO_Pin_1)

// SPI CMD
#define READ                3
#define WRITE               1

// Zustand
#define 	eSPI_STATE_POWERUP 				 (0)
#define 	eSPI_STATE_INITIALIZED  		 (1)
#define 	eSPI_STATE_IDLE					 (2)
#define 	eSPI_STATE_WRITE_IRQ	   		 (3)
#define 	eSPI_STATE_WRITE_FIRST_PORTION   (4)
#define 	eSPI_STATE_WRITE_EOT			 (5)
#define 	eSPI_STATE_READ_IRQ				 (6)
#define 	eSPI_STATE_READ_FIRST_PORTION	 (7)
#define 	eSPI_STATE_READ_EOT				 (8)

#define DMA_WINDOW_SIZE         1024
#define SPI_WINDOW_SIZE         DMA_WINDOW_SIZE
#define HEADERS_SIZE_EVNT       (SPI_HEADER_SIZE + 5) //hci.h zeile 5
unsigned char wlan_rx_buffer[CC3000_RX_BUFFER_SIZE];
unsigned char wlan_tx_buffer[CC3000_TX_BUFFER_SIZE];

uint8_t tSpiReadHeader[] = { READ, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

gcSpiHandleRx g_pfRxHandler = NULL;

unsigned int tx_spi = 1; //TODO

tSpiInformation sSpiInformation;

extern void WlanInterruptEnable(void);
extern void WlanInterruptDisable(void);

void wait_uSek(unsigned long us) {
	// wartet uSekunden gilt aber nur bei 168MHz Taktfrequenz
	us *= 12;
	while (us--) {
		__NOP();
	}
}

void WLAN_IRQ_Handler(void) {
	EXTI_ClearFlag(EXTI_Line10);
	EXTI_ClearITPendingBit(EXTI_Line10);

	if (!ReadWlanInterruptPin()) // WLAN IRQ PIN = 0
	{
//		NVIC_DisableIRQ(EXTI15_10_IRQn);
		// eSPI_STATE_POWERUP
		if (sSpiInformation.ulSpiState == eSPI_STATE_POWERUP) {
			sSpiInformation.ulSpiState = eSPI_STATE_INITIALIZED;
		}

		// eSPI_STATE_IDLE
		else if (sSpiInformation.ulSpiState == eSPI_STATE_IDLE) {
			//TODO bekommt IRQ setzt CS liest ... Fluﬂdiagramm
			//
			// wenn im IDLE-Status IRQ kommt dann Status auf READ_IRQ setzen
			// und CS auf 0 und Lesen
			sSpiInformation.ulSpiState = eSPI_STATE_READ_IRQ;
			ASSERT_CS();
			SpiReadWriteStringInt(TRUE, sSpiInformation.pRxPacket, 10);
//			SpiReadWriteString(TRUE, sSpiInformation.pRxPacket, 10);
		}

		// eSPI_STATE_WRITE_IRQ
		else if (sSpiInformation.ulSpiState == eSPI_STATE_WRITE_IRQ) {
			// Data < 1024
//			if (sSpiInformation.usTxPacketLength <= SPI_WINDOW_SIZE) {
			sSpiInformation.ulSpiState = eSPI_STATE_WRITE_EOT;
			SpiReadWriteStringInt(FALSE, sSpiInformation.pTxPacket,
					sSpiInformation.usTxPacketLength);
//			SpiReadWriteString(FALSE, sSpiInformation.pTxPacket,
//					sSpiInformation.usTxPacketLength);

//			}
//
//			// Data > 1024
//			else {
//				sSpiInformation.ulSpiState = eSPI_STATE_WRITE_EOT;
//								SpiReadWriteStringInt(FALSE, sSpiInformation.pTxPacket,
//										sSpiInformation.usTxPacketLength);
//				sSpiInformation.ulSpiState = eSPI_STATE_WRITE_FIRST_PORTION;
//				SpiReadWriteString(FALSE, sSpiInformation.pTxPacket,
//						SPI_WINDOW_SIZE);
//		}
		}

		else {
			char spi_buf_out[40];
			sprintf(spi_buf_out, "\r\n ##0 WLANIRQ State %lu \r\n ",
					sSpiInformation.ulSpiState);
			uart_send(spi_buf_out);
		}
//		NVIC_EnableIRQ(EXTI15_10_IRQn);

	} else {
		char spi_buf_out[40];
		sprintf(spi_buf_out, " \r\n##1 WLANIRQ State %lu \r\n ",
				sSpiInformation.ulSpiState);
		uart_send(spi_buf_out);
	}
}

void DMA1_Stream4_IRQHandler(void) {
// Stream RX/TX ausschalten
//DMA_Cmd(DMA1_Stream3, DISABLE);
	DMA_Cmd(DMA1_Stream4, DISABLE);
// DMA Interrupt Handler aufrufen
	DMA_Int_Handler();
//DMA Streamx transfer complete flag zur¸cksetzen
	DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4);
	DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3);
// Merker zur¸cksetzen
	tx_spi = 0;
}

void DMA_Int_Handler(void) {
	unsigned long ucTxFinished, ucRxFinished;
	unsigned short data_to_recv;
	hci_data_hdr_t *pDataHdr;

//DMA Streamx transfer complete flag abfragen
	ucTxFinished = DMA_GetFlagStatus(DMA1_Stream4, DMA_FLAG_TCIF4);
	ucRxFinished = DMA_GetFlagStatus(DMA1_Stream3, DMA_FLAG_TCIF3);

//===== eSPI_STATE_READ_IRQ
	if (sSpiInformation.ulSpiState == eSPI_STATE_READ_IRQ) {
		if (ucTxFinished && ucRxFinished) {
			DMA_und_SPI_ClearInterruptFlag();
			{
				if (!SpiReadDataCont())
					DEASSERT_CS();
				sSpiInformation.ulSpiState = eSPI_STATE_IDLE;
				sSpiInformation.SPIRxHandler(
						sSpiInformation.pRxPacket + sizeof(btspi_hdr));
			}
		}
	}

//===== eSPI_STATE_READ_FIRST_PORTION
	else if (sSpiInformation.ulSpiState == eSPI_STATE_READ_FIRST_PORTION) {
		if (ucRxFinished) {
			DMA_und_SPI_ClearInterruptFlag();
			pDataHdr = (hci_data_hdr_t *) (sSpiInformation.pRxPacket
					+ sizeof(btspi_hdr));
			data_to_recv = pDataHdr->usLength - SPI_WINDOW_SIZE;

			if (!((HEADERS_SIZE_EVNT + data_to_recv) & 1)) {
				data_to_recv++;
			}

			SpiReadData(sSpiInformation.pRxPacket + 10 + SPI_WINDOW_SIZE,
					data_to_recv);

			sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;
		}
	}

//===== eSPI_STATE_READ_EOT
	else if (sSpiInformation.ulSpiState == eSPI_STATE_READ_EOT) {
		if (ucRxFinished) {
			DMA_und_SPI_ClearInterruptFlag();
			DEASSERT_CS();
			sSpiInformation.ulSpiState = eSPI_STATE_IDLE;
			sSpiInformation.SPIRxHandler(
					sSpiInformation.pRxPacket + sizeof(btspi_hdr));
		}
	}

//===== eSPI_STATE_WRITE_EOT
	else if (sSpiInformation.ulSpiState == eSPI_STATE_WRITE_EOT) {
		if (ucTxFinished) {
			while (SPI_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) != RESET) {
				;
			}
			DMA_und_SPI_ClearInterruptFlag();
			DEASSERT_CS();
			sSpiInformation.ulSpiState = eSPI_STATE_IDLE;
		}
	}

//===== eSPI_STATE_WRITE_FIRST_PORTION
	else if (sSpiInformation.ulSpiState == eSPI_STATE_WRITE_FIRST_PORTION) {
		if (ucTxFinished) {
			sSpiInformation.ulSpiState = eSPI_STATE_WRITE_EOT;
			SpiReadWriteString(FALSE,
					sSpiInformation.pTxPacket + SPI_WINDOW_SIZE,
					sSpiInformation.usTxPacketLength - SPI_WINDOW_SIZE);
		}
	} else {
//		char buffer_out[40];
//		sprintf(buffer_out, "DMAIRQ im State %d \r\n ",sSpiInformation.ulSpiState);
//		uart_send(buffer_out);
	}
}

long ReadWlanInterruptPin(void) {
	return GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10);
}

void SpiReadData(unsigned char *data, unsigned short size) {
	SpiReadWriteStringInt(TRUE, data, size);
}
void SpiReadWriteStringInt(uint32_t ulTrueFalse, const uint8_t *ptrData,

uint32_t ulDataSize) {
	wait_uSek(50);
	DMA_Cmd(DMA1_Stream3, DISABLE);
	DMA_Cmd(DMA1_Stream4, DISABLE);

//    tx_spi = 1;

	if (ulTrueFalse == TRUE) {
		// READ
		// RXD - DMA1 Channel 0 Stream3
		DMA_MemoryTargetConfig(DMA1_Stream3, (uint32_t) ptrData, DMA_Memory_0);
		DMA_SetCurrDataCounter(DMA1_Stream3, ulDataSize);
		// TXD - DMA1 Channel 0 Stream4

		memset(wlan_tx_buffer, 0x03, ulDataSize);
		DMA_MemoryTargetConfig(DMA1_Stream4, (uint32_t) wlan_tx_buffer,
				DMA_Memory_0);
		//DMA_MemoryTargetConfig(DMA1_Stream4, (uint32_t)tSpiReadHeader, DMA_Memory_0);
		DMA_SetCurrDataCounter(DMA1_Stream4, ulDataSize);

	} else {
		// WRITE
		// RXD - DMA1 Channel 0 Stream3
		memset(sSpiInformation.pRxPacket, 0x00, ulDataSize);
		DMA_MemoryTargetConfig(DMA1_Stream3,
				(uint32_t) sSpiInformation.pRxPacket, DMA_Memory_0);
		DMA_SetCurrDataCounter(DMA1_Stream3, ulDataSize);
		// TXD - DMA1 Channel 0 Stream4
		DMA_MemoryTargetConfig(DMA1_Stream4, (uint32_t) ptrData, DMA_Memory_0);
		DMA_SetCurrDataCounter(DMA1_Stream4, ulDataSize);
	}

// Enable DMA Interrupt
	DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);

	DMA_Cmd(DMA1_Stream3, ENABLE);
	DMA_Cmd(DMA1_Stream4, ENABLE);

// Warten auf DMA Transfer komplett
	while (DMA_GetCurrDataCounter(DMA1_Stream4))
		; // TX complete
	while (DMA_GetCurrDataCounter(DMA1_Stream3))
		; // RX complete

// wird in der DMA ISR zur¸ckgesetzt wenn transport komplett
	while (tx_spi) {
	}

// Wait for the end of SPI Transport Layer Transaction
	while (SPI_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) != RESET) {
		;
	}
}

void SpiReadWriteString(uint32_t ulTrueFalse, const uint8_t *ptrData,
		uint32_t ulDataSize) {
	wait_uSek(50);
	DMA_Cmd(DMA1_Stream3, DISABLE);
	DMA_Cmd(DMA1_Stream4, DISABLE);

	if (ulTrueFalse == TRUE) {
		// READ
		DMA_MemoryTargetConfig(DMA1_Stream3, (uint32_t) ptrData, DMA_Memory_0);
		DMA_SetCurrDataCounter(DMA1_Stream3, ulDataSize);

		memset(wlan_tx_buffer, 0x03, ulDataSize);
		DMA_MemoryTargetConfig(DMA1_Stream4, (uint32_t) wlan_tx_buffer,
				DMA_Memory_0);
		//DMA_MemoryTargetConfig(DMA1_Stream4, (uint32_t)tSpiReadHeader, DMA_Memory_0);
		DMA_SetCurrDataCounter(DMA1_Stream4, ulDataSize);
	} else {
		// WRITE
		memset(sSpiInformation.pRxPacket, 0x00, ulDataSize);
		DMA_MemoryTargetConfig(DMA1_Stream3,
				(uint32_t) sSpiInformation.pRxPacket, DMA_Memory_0);
		DMA_SetCurrDataCounter(DMA1_Stream3, ulDataSize);
		DMA_MemoryTargetConfig(DMA1_Stream4, (uint32_t) ptrData, DMA_Memory_0);
		DMA_SetCurrDataCounter(DMA1_Stream4, ulDataSize);
	}

// DMA Transfer starten
	DMA_Cmd(DMA1_Stream3, ENABLE);
	DMA_Cmd(DMA1_Stream4, ENABLE);

// Warten bis Transfer komplett
	while (DMA_GetCurrDataCounter(DMA1_Stream4))
		; // TX complete
	while (DMA_GetCurrDataCounter(DMA1_Stream3))
		; // RX complete

// wird in der DMA ISR zur¸ckgesetzt wenn Transport komplett
	while (tx_spi) {
	}

// Wait for the end of SPI Transport Layer Transaction
	while (SPI_GetFlagStatus(SPI2, SPI_I2S_FLAG_BSY) != RESET) {
		;
	}
}

void SpiOpen(gcSpiHandleRx pfRxHandler) {
	sSpiInformation.ulSpiState = eSPI_STATE_POWERUP;

	sSpiInformation.SPIRxHandler = pfRxHandler;
//TODO folgende Zeile war ausgeklammert:
//sSpiInformation.pTxPacket = wlan_tx_buffer;
	sSpiInformation.pRxPacket = wlan_rx_buffer;

	sSpiInformation.usTxPacketLength = 0; //sizeof(wlan_tx_buffer);
	sSpiInformation.usRxPacketLength = 0; //sizeof(wlan_rx_buffer);

	init_CC3000_SPI();

	tSLInformation.WlanInterruptEnable();
}

void SpiClose(void) {
	if (sSpiInformation.pRxPacket) {
		sSpiInformation.pRxPacket = 0;
	}
	WlanInterruptDisable();
}

long SpiWrite(unsigned char *pUserBuffer, unsigned short usLength) {
	unsigned char ucPad = 0;

	if (!(usLength & 0x0001)) {
		ucPad++;
	}

	pUserBuffer[0] = WRITE;
	pUserBuffer[1] = HI(usLength + ucPad);
	pUserBuffer[2] = LO(usLength + ucPad);
	pUserBuffer[3] = 0;
	pUserBuffer[4] = 0;

	usLength += (sizeof(btspi_hdr) + ucPad);

// eSPI_STATE_POWERUP
	if (sSpiInformation.ulSpiState == eSPI_STATE_POWERUP) {
		while (sSpiInformation.ulSpiState != eSPI_STATE_INITIALIZED) {
			;
		}
	}

// eSPI_STATE_INITIALIZED
	if (sSpiInformation.ulSpiState == eSPI_STATE_INITIALIZED) {
		// First Write
		ASSERT_CS();
		SpiReadWriteString(FALSE, pUserBuffer, 4);
		SpiReadWriteString(FALSE, pUserBuffer + 4, 6);
		sSpiInformation.ulSpiState = eSPI_STATE_IDLE;
		DEASSERT_CS();

		while (eSPI_STATE_IDLE != sSpiInformation.ulSpiState) {
			;
		}
	}

// ???
	else {
		//
		// We need to prevent here race that can occur in case 2 back to back packets are sent to the
		// device, so the state will move to IDLE and once again to not IDLE due to IRQ
		//

//		tSLInformation.WlanInterruptDisable();

		while (sSpiInformation.ulSpiState != eSPI_STATE_IDLE) {
			;
		}

		// IRQ verbieten und Zustand neu setzen auf eSPI_STATE_WRITE_IRQ
//		tSLInformation.WlanInterruptDisable();
		NVIC_DisableIRQ(EXTI15_10_IRQn);

		// Zustand ‰ndern da aus IRQ angestossen
		sSpiInformation.ulSpiState = eSPI_STATE_WRITE_IRQ;
		sSpiInformation.pTxPacket = pUserBuffer;
		sSpiInformation.usTxPacketLength = usLength;

		// DMA Transfer und IRQ erlauben
//		DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);
//		tSLInformation.WlanInterruptEnable();
		EXTI_ClearFlag(EXTI_Line10); // TESTWEISE
		EXTI_ClearITPendingBit(EXTI_Line10); // TESTWEISE
		NVIC_EnableIRQ(EXTI15_10_IRQn);
		//
		// Assert the CS line and wait till SSI IRQ line is active and then initialize write operation
		//
		ASSERT_CS();
		//
		// Re-enable IRQ - if it was not disabled - this is not a problem...
		//

//		tSLInformation.WlanInterruptEnable();
		NVIC_EnableIRQ(EXTI15_10_IRQn);
		// warten ....
		while (eSPI_STATE_IDLE != sSpiInformation.ulSpiState) {
			;
		}

		//Actual transmission does not occur in the function itself,
		//but rather in the IRQ handler, which is triggered by
		//IRQ line assertion from the CC3000 device.
		//Also observe the blocking behavior of the function:
		//the function is blocked until the SPI is not in IDLE state.
		//This blocking behavior is expected from any other implementation.

	}
	return (0);
}

void SpiResumeSpi(void) {
	WlanInterruptEnable();
}

void WriteWlanPin(unsigned char val) {
	if (val) {
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
	} else {
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	}
}

void WlanInterruptEnable(void) {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource1);

	EXTI_ClearITPendingBit(EXTI_Line10);

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
}

void WlanInterruptDisable(void) {
	EXTI_ClearITPendingBit(EXTI_Line10);

	EXTI_InitTypeDef EXTI_SPIGPIO_InitStructure;
	EXTI_SPIGPIO_InitStructure.EXTI_Line = EXTI_Line10;
	EXTI_SPIGPIO_InitStructure.EXTI_LineCmd = DISABLE;
	EXTI_SPIGPIO_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_SPIGPIO_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_SPIGPIO_InitStructure);

	NVIC_InitTypeDef NVIC_InitSPIGPIOStructure;
	NVIC_InitSPIGPIOStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_InitSPIGPIOStructure.NVIC_IRQChannel = EXTI15_10_IRQn; //EXTI15_10_IRQn;
	NVIC_InitSPIGPIOStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitSPIGPIOStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_Init(&NVIC_InitSPIGPIOStructure);

//SYSCFG_DeInit();
}

void DMA_und_SPI_ClearInterruptFlag(void) {
	DMA_ClearFlag(DMA1_Stream4, DMA_FLAG_TCIF4);
	DMA_ClearFlag(DMA1_Stream3, DMA_FLAG_TCIF3);
	SPI_I2S_ClearFlag(SPI2, SPI_IT_TXE | SPI_IT_RXNE);
}

long SpiReadDataCont(void) {
	hci_hdr_t *hci_hdr;
	long data_to_recv;
//	hci_evnt_hdr_t *hci_evnt_hdr;
	unsigned char *evnt_buff;
	hci_data_hdr_t *pDataHdr;

	evnt_buff = sSpiInformation.pRxPacket;
	data_to_recv = 0;
	hci_hdr = (hci_hdr_t *) (evnt_buff + sizeof(btspi_hdr));

	switch (hci_hdr->ucType) {
	case HCI_TYPE_DATA: {
		//uart_send("hci type data \n\r");
		pDataHdr = (hci_data_hdr_t *) (evnt_buff); //pfeiffer + sizeof(btspi_hdr));

		if (pDataHdr->usLength >= SPI_WINDOW_SIZE) {
			data_to_recv = eSPI_STATE_READ_FIRST_PORTION;
			SpiReadData(evnt_buff + 10, SPI_WINDOW_SIZE);
			sSpiInformation.ulSpiState = eSPI_STATE_READ_FIRST_PORTION;
		} else {
			//data_to_recv = pDataHdr->usLength;
			STREAM_TO_UINT16((char *)(evnt_buff + SPI_HEADER_SIZE),
					HCI_DATA_LENGTH_OFFSET, data_to_recv);
			if (!((HEADERS_SIZE_EVNT + data_to_recv) & 1)) {
				data_to_recv++;
			}

			if (data_to_recv) {
				SpiReadData(evnt_buff + 10, data_to_recv);
				data_to_recv = 0;
			}
			//TODO liuers hats ausgebaut
			sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;
		}
		break;
	}
	case HCI_TYPE_EVNT: {
		//TODO
		//uart_send("hci type event \n\r");
		hci_hdr = (hci_hdr_t *) (evnt_buff); // + sizeof(btspi_hdr)); //pfeiffer FLASCH ?

//		hci_evnt_hdr = (hci_evnt_hdr_t *) hci_hdr;
//		data_to_recv = hci_evnt_hdr->ucLength - 1; // vorher -5
		STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE),
				HCI_EVENT_LENGTH_OFFSET, data_to_recv);
		data_to_recv -= 1;

		if ((HEADERS_SIZE_EVNT + data_to_recv) & 1) {

			data_to_recv++;
		}

		if (data_to_recv) {
			SpiReadData(evnt_buff + 10, data_to_recv);
			data_to_recv = 0;
			//TODO wieder einkommentieren
		}
		sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;
		break;
	}
	}
	return (/*data_to_recv*/0);
}
