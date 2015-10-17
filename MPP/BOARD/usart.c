#include "usart.h"

void uart_send(char* chars) {
	int i = 0;
	for(i = 0;i < strlen(chars);i++)
	{
		USART_SendData(USART2, chars[i]);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){}
	}
}

void usart2_send(char* chars) {
	int i = 0;
	for(i = 0;i < strlen(chars);i++)
	{
		USART_SendData(USART2, chars[i]);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){}
	}
}

void usart2_send_buffer(char* chars, char len )
{
	int i = 0;
	for(i = 0; i < len; i++)
	{
		USART_SendData(USART2, chars[i]);
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET){}
	}
}

void usart6_send(char* chars)
{
	int i = 0;
	for(i = 0;i < strlen(chars);i++)
	{
		USART_SendData(USART6, chars[i]);
		while (USART_GetFlagStatus(USART6, USART_FLAG_TC) == RESET){}
	}
}

void usart2_init(void) {
GPIO_InitTypeDef GPIO_InitStructure;
USART_InitTypeDef USART_InitStructure;

/* enable peripheral clock for USART2 */
RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);


/* GPIOA clock enable */
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

/* GPIOA Configuration:  USART2 TX on PA2 */
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
GPIO_Init(GPIOA, &GPIO_InitStructure);

/* Connect USART2 pins to AF2 */
// TX = PA2
GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);

USART_InitStructure.USART_BaudRate = 921600; //TODO 115200;
USART_InitStructure.USART_WordLength = USART_WordLength_8b;
USART_InitStructure.USART_StopBits = USART_StopBits_1;
USART_InitStructure.USART_Parity = USART_Parity_No;
USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
USART_InitStructure.USART_Mode = USART_Mode_Tx;
USART_Init(USART2, &USART_InitStructure);

USART_Cmd(USART2, ENABLE); // enable USART2
}

void usart6_init(void)
{
	// Struct Anlegen
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	// Periperie Clocksystem Einschalten
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

	// GPIO Initialisieren
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// Alternativ Funktion Zuweisen
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

	// USART als Alternativ Funktion Initialisieren
	USART_InitStructure.USART_BaudRate = 921600; //TODO 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx ;
	USART_Init(USART6, &USART_InitStructure);

	// USART Einschalten
	USART_Cmd(USART6, ENABLE);

	// DMA Interface aktivieren
	USART_DMACmd(USART6, USART_DMAReq_Tx, ENABLE);

}

void usart6_init_IRQ(void){
	// RXNE interrupt erlauben
	// Erlaubt Interrupt wenn Zeichen Empfangen
	USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);

	// TXE interrupt erlauben
	// Erlaubt interrupt wenn Zeichen versendet
	// USART_ITConfig(USART6, USART_IT_TXE, ENABLE);

	// USART6 Interrupt global erlauben
	NVIC_EnableIRQ(USART6_IRQn);

	// USART6 Interrupt global verbieten
	//NVIC_DisableIRQ(USART6_IRQn);
}




void USART6_IRQ(void)
{
    char zeichen;

    // RxD - Empfangsinterrupt
    if (USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)
    	{
    	zeichen = (char)USART_ReceiveData(USART6);
    	//Zeichen in den Empfangspuffer schreiben
        Buffer[0] = zeichen;
    	}

    // TxD - Sendeinterrupt
    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    	{
//        if (BufferGet(&U1Tx, &ch) == SUCCESS)//if buffer read
//			{
//			// Zeichen aus dem Puffer senden
//			USART_SendData(USART1, ch);
//			}
//		else
//			{
//			// Wenn der Puffer leer ist TxD-Interrupt verbieten
//			USART_ITConfig(USART6, USART_IT_TXE, DISABLE);
//			}
    	}
}



void DMA2_Stream6_IRQ(void){
//	DMA2_Stream6 Channel5 Tx
//	DMA2_Stream1 Channel5 Rx
//  DMA Stream komplett?
    if (DMA_GetITStatus(DMA2_Stream6, DMA_IT_TCIF6))
        {
        // Clear DMA Stream Transfer Complete interrupt pending bit
        DMA_ClearITPendingBit(DMA2_Stream6, DMA_IT_TCIF6);
        }
}

void DMA2_TX_init(void){

	sprintf(Buffer,"dma test ausgabe\r\n");
	usart6_send(Buffer);
	// jetzt erfolgt die Interrupt Controller Konfiguration

	NVIC_InitTypeDef NVIC_InitStructure;

	// Configure the Priority Group to 2 bits
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	// Enable the UART6 TX DMA Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Hier erfolgt die Konfiguration der DMA

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

	DMA_InitTypeDef  DMA_InitStructure;

	DMA_DeInit(DMA2_Stream6);

	DMA_InitStructure.DMA_Channel = DMA_Channel_5;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)Buffer;
	DMA_InitStructure.DMA_BufferSize = (uint16_t)strlen(Buffer);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART6->DR;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

	DMA_Init(DMA2_Stream6, &DMA_InitStructure);

	// Enable the USART Rx DMA request
	USART_DMACmd(USART6, USART_DMAReq_Tx, ENABLE);

	// Enable DMA Stream Half Transfer and Transfer Complete interrupt
	DMA_ITConfig(DMA2_Stream6, DMA_IT_TC, ENABLE);
	//DMA_ITConfig(DMA2_Stream6, DMA_IT_HT, ENABLE);

	// Enable the DMA TX Stream
	DMA_Cmd(DMA2_Stream6, ENABLE);

}
