#include "TCA6416.h"

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

// Structure für die Datenregister des TCA6416
volatile TCA6416_DAT TCA6416_DATA;

void wait_uSek_TCA6416(unsigned long us) {
	// wartet uSekunden gilt aber nur bei 168MHz Taktfrequenz
		us *= 12;
	    while(us--) { __NOP(); }
	}

//=============================================================================
// Funktion initialisiert die I/O und I2C3 Schnittstelle
//=============================================================================
void init_TAC6416_I2C3(void){

	//=== benötigte Clocksystem einschalten

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C3, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	//=== TCA6416 - /RESET-Leitung (PC0) als Output Initialisieren

	GPIO_InitTypeDef GPIO_InitStructure;
	// PC0 als Putput
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	// Leitung auf 1
	GPIO_SetBits(GPIOC, GPIO_Pin_0);
	// RESET TCA6416
	reset_TAC6416();

	//=== TCA6416 - /INT-Leitung (PC8) als Interruptquelle initialisieren

	// EXTI Struct anlegen
	EXTI_InitTypeDef EXTI_InitStructure;
	// NVIC Struct anlegen
	NVIC_InitTypeDef NVIC_InitStructure;

	// TCA6416 /INT - Leitung
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// Interrupt Konfiguration
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource8);

	// EXT_INT8 Line Konfiguration
	EXTI_InitStructure.EXTI_Line = EXTI_Line8;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);

	// NVIC Konfiguration
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//=== I2C3 Init

	// Zurücksetzen der I2C3 Schnittstelle und Default Werte laden
	I2C_DeInit(I2C3);

	// SCL - PA8 Alternativ Funktion
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_I2C3);

	// SDA - PC9 Alternativ Funktion
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_I2C3);

	//	I2C Konfiguration
	I2C_InitTypeDef I2C_InitStructure;
	I2C_InitStructure.I2C_ClockSpeed = 400000;	// 400kHz
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Disable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(I2C3, &I2C_InitStructure);

	// Enable I2C3
	I2C_Cmd(I2C3, ENABLE);
}


//=============================================================================
// Funktion Reset der TAC6416 Logik (alles Input nicht invertiert)
//=============================================================================
void reset_TAC6416(void){	// generiert 101 Sequenz
	GPIO_SetBits(GPIOC, GPIO_Pin_0);
	GPIO_ResetBits(GPIOC,GPIO_Pin_0);
	wait_uSek_TCA6416(1);
	GPIO_SetBits(GPIOC, GPIO_Pin_0);
	wait_uSek_TCA6416(1);
}


//=============================================================================
// ISR für deb TCA6416 ist dem EXTI9_5_IRQHandler zuzuordnen
//=============================================================================
void ISR_IRQ8_TAC6416(void){

	// INT Flags zurücksetzen
	EXTI_ClearITPendingBit(EXTI_Line8);
	EXTI_ClearFlag(EXTI_Line8);

	// INT disable
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	// Input Port0 und Port1 lesen
	I2C_start(I2C3, SLAVE_ADDRESS, I2C_Direction_Transmitter); // start a transmission in Master receiver mode
	I2C_write(I2C3,InputPort0); // write one byte to the slave
	I2C_stop(I2C3); // stop the transmission
	I2C_start(I2C3, SLAVE_ADDRESS, I2C_Direction_Receiver); // start a transmission in Master receiver mode
	TCA6416_DATA.input_port0 = I2C_read_ack(I2C3); // read one byte and request another byte
	TCA6416_DATA.input_port1 = I2C_read_nack(I2C3); // read one byte and don't request another byte
	I2C_stop(I2C3); // stop the transmission

	// INT Flags zurücksetzen
	EXTI_ClearITPendingBit(EXTI_Line8);
	EXTI_ClearFlag(EXTI_Line8);

	// INT enable
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	}





