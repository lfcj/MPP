#include "CC1101.h"

//=========================================================================
// Anschlussbelegung des CC1101 ISM Transceivers (SPI1)
//=========================================================================
//	GDO0			PC4
// 	GDO2			PC5
// 	SPI-CS			PB12	SS1
// 	SPI-CLK			PA5		SCK1
// 	SPI-MISO		PA6		MISO1 GDO1
// 	SPI-MOSI		PA7		MOSI1
//=========================================================================

//=============================================================================
// Rx Tx Buffer
//=============================================================================
volatile  CC1100_Rx RxCC1100;
volatile  CC1100_Tx TxCC1100;
//=============================================================================
// Knoten ID und Funkkanal
//=============================================================================
unsigned char ID = 255;
unsigned char channel = 0;


//=============================================================================
// PATABLE sollte überarbeitet werden
//=============================================================================
unsigned char paTableIndex = PATABLE;	// Current PATABLE Index
unsigned char paTable[] = {    0x00,		// -52 dBm	0
							   0x23, 		// -15 dBm	1
							   0x33, 		// -12 dBm	2
							   0x34, 		// -10 dBm	3
							   0x28, 		// -8 dBm	4
							   0x2B, 		// -6 dBm	5
							   0x57, 		// -4 dBm	6
							   0x54, 		// -2 dBm	7
							   0x3F, 		// 0 dBm	8
							   0x8C, 		// +2 dBm	9
							   0x8A, 		// +3 dBm	10
							   0x87, 		// +4 dBm	11
							   0x84, 		// +5 dBm	12
							   0xCE, 		// +6 dBm	13
							   0xCC, 		// +7 dBm	14
							   0xC9, 		// +8 dBm	15
							   0xC6, 		// +9 dBm	16
							   0xC3  		// +10 dBm	17
							};
//=============================================================================


//=============================================================================
// Function switchchannel - 11 fest definierte Kanalfrequenzen
//=============================================================================
unsigned char FREQ2[]={0x21,0x21,0x21,0x21,0x21,0x21,0x21,0x21,0x21,0x21,0x21};
unsigned char FREQ1[]={0x71,0x6E,0x6B,0x68,0x65,0x41,0x3F,0x3C,0x39,0x36,0x33};
unsigned char FREQ0[]={0x7A,0x85,0x91,0x9D,0xA9,0xF8,0x03,0x0F,0x1B,0x27,0x33};
//Index		 frequency	 	Arbeitsplatz
//channel=0, f=869.525MHz  	Notrufkanal
//channel=1, f=869.225MHz  	HWP1
//channel=2, f=868.925MHz  	HWP2
//channel=3, f=868.625MHz  	HWP3
//channel=4, f=868.325MHz  	HWP4
//channel=5, f=864.700MHz	HWP5
//channel=6, f=864.400MHz   HWP6
//channel=7, f=864.100MHz  	HWP7
//channel=8, f=863.800MHz  	HWP8
//channel=9, f=863.500MHz   HWP9
//channel=10,f=863.200MHz   HWP10


//=============================================================================
// 400 KBit/s, 869.525 MHz, MSK, Quartz: 26 MHz, Addresse=255
//=============================================================================
char conf[39];
char conf[] = {
  0x06, // IOCFG2	GDO2 Signal Konfigurierung Table 34 Packet,CRC,FIFO
  0x2E, // IOCFG1	GDO1 Signal Konfigurierung Table 34 Tristate
  0x0E, // IOCFG0	GDO0 Signal Konfigurierung Table 34 Carrier Sense
  0x0F, // FIFOTHR	Bytes in FIFO
  0x9B, // SYNC1	Sync Word HighByte 0x9B
  0xAD, // SYNC0	Sync Word Low Byte 0xAD
  0x3F, // PKTLEN	Packetlänge = 63 bei variabler Packetlänge
  0x06, // PKTCTRL1	2 Status Bytes anfügen; Adresscheck ON Broadcastadresse 0
  0x45, // PKTCTRL0	variable Packetlänge ON; whitening ON,
  0xFF, // ADDR		Addresse für Packetfilterung (Knotenadresse)
  0x00, // CHANNR
  0x0B, // FSCTRL1
  0x00, // FSCTRL0
  0x21, // FREQ2	Frequenz Control Word High Byte
  0x71, // FREQ1	Frequenz Control Word Middle Byte
  0x7A, // FREQ0	Frequenz Control Word Low Byte
  0x2D, // MDMCFG4
  0xF8, // MDMCFG3
  0x73, // MDMCFG2	Modulationsformat MSK
  0x42, // MDMCFG1	8 Präambel Bytes,
  0xF8, // MDMCFG0
  0x00, // DEVIATN
  0x07, // MCSM2 	(RX_TIME = until end of packet)
  0x03, // MCSM1 	(TX_OFFMODE = RX)
  0x18, // MCSM0	(autom. Frequenzkalibrierung bei IDLE->Tx und IDLE->Rx ON)
  0x1D, // FOCCFG
  0x1C, // BSCFG
  0xC0, // AGCCTRL2	AGC Control Gain
  0x49, // AGCCTRL1	AGC Control Carrier Sense Level
  0xB2, // AGCCTRL0	AGC Control
  0x87, // WOREVT1
  0x6B, // WOREVT0
  0xF8, // WORCTRL	WOR Control
  0xB6, // FREND1	Front End Rx
  0x10, // FREND0	Front End TX - PA Power settings
  0xEA, // FSCAL3
  0x2A, // FSCAL2
  0x00, // FSCAL1
  0x1F  // FSCAL0
};
//=============================================================================


void wait_uSek_CC1101(unsigned long us) {
	// wartet uSekunden gilt aber nur bei 168MHz Taktfrequenz
		us *= 12;
	    while(us--) { __NOP(); }
	}


//=============================================================================
// Funktion wartet bis die SPI des CC1100 verfügbar ist
//=============================================================================
void spiInitTrx(void)
	{
	//===== startet die Kommunikation über die SPI mit dem CC1101:
	//===== 1. Port auf GPIO Input setzen
	// GPIO Strukt anlegen
	GPIO_InitTypeDef GPIO_InitStructure;
	// GPIO Strukt initalisieren
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	// Register schreiben
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	//===== 2. CS setzen
	CC1100_CS_LOW;
	//===== 3. warten ob bereit
	while (CC1100_GDO1) { ; }
	//=====	4. Port auf SPI MISO setzen
	// GPIO Strukt initalisieren
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	// Register schreiben
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	// Zuweisung der Alternativ Funktion
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	}
//=============================================================================
// Funktion sendet und empfängt ein Byte über die SPI1 des STM32
//=============================================================================
unsigned char trx_spi(unsigned char tx_data)
	{
	unsigned char rx_data;
	while (SPI1->SR & SPI_I2S_FLAG_BSY) { ; }
	SPI1->DR = tx_data;
	while (!(SPI1->SR & SPI_I2S_FLAG_TXE)) { ; }
	while (!(SPI1->SR & SPI_I2S_FLAG_RXNE)) { ; }
	rx_data = SPI1->DR;
	return rx_data;
	}
//=============================================================================
// Funktion zum Schreiben eines einzelnen CC1100 Registers
//=============================================================================
void spiWriteReg(unsigned char addr, unsigned char value)
	{
	spiInitTrx();				// Init SPI CS = 0 warten bis bereit
	trx_spi(addr);				// Adresse schreiben
	trx_spi(value);				// Wert schreiben
	CC1100_CS_HIGH;				// CS = 1
	}
//=============================================================================
// Funktion zum Lesen eines einzelnen CC1100 Registers
//=============================================================================
unsigned char spiReadReg(unsigned char addr)
	{
	unsigned char x;			// Variable Rückgabewert
	spiInitTrx();				// Init SPI CS = 0 warten bis bereit
	trx_spi(addr | CC1100_READ_SINGLE); // Kommando schreiben
	x = trx_spi(NOBYTE);		// Wert lesen
	CC1100_CS_HIGH;				// CS = 1
	return x;
	}
//=============================================================================
// Funktion zum Schreiben des Strobe Kommandos
//=============================================================================
void spiStrobe(unsigned char strobe)
	{
	spiInitTrx();				// Init SPI CS = 0 warten bis bereit
	trx_spi(strobe);
	CC1100_CS_HIGH;
	}
//=============================================================================
// Funktion zum Schreiben der Register im Burst
//=============================================================================
void spiWriteBurstReg(unsigned char addr, char *buffer, unsigned char count)
	{
	unsigned char i;					// Variable
	spiInitTrx();				// Init SPI CS = 0 warten bis bereit
	trx_spi(addr | CC1100_WRITE_BURST);
	for (i = 0; i < count; i++) {
		trx_spi(buffer[i]);
		}
	CC1100_CS_HIGH;
	}
//=============================================================================
// Funktion zum Lesen der Register des CC1100 im Burst
//=============================================================================
void spiReadBurstReg(unsigned char addr, char *buffer, unsigned char count)
	{
	unsigned char i;			// Variable
	spiInitTrx();				// Init SPI CS = 0 warten bis bereit
	trx_spi(addr | CC1100_READ_BURST);
	for (i = 0; i < count; i++) {
		buffer[i] = trx_spi(NOBYTE);
		}
	CC1100_CS_HIGH;
	}
//=============================================================================
// Funktionen zum Lesen der Status Register des CC1100
//=============================================================================
unsigned char spiReadStatus(unsigned char addr)
	{
	unsigned char x;	// Variable
	spiInitTrx();		// Init SPI CS = 0 warten bis bereit
	trx_spi(addr | CC1100_READ_BURST);
	x = trx_spi(NOBYTE);
	CC1100_CS_HIGH;
	return x;			// Chip Status Byte - Tabelle 17 im CC1100 Data Sheet
	}
//=============================================================================
// Funktion RESET des CC1100
//=============================================================================
void reset_CC1101(void)
	{
	spiInitTrx();				// Init SPI CS = 0 warten bis bereit
	trx_spi(CC1100_SRES);		// Strobe Kommando Reset
	spiInitTrx();				// Init SPI CS = 0 warten bis bereit
	CC1100_CS_HIGH;				// CS = 1
	}
//=============================================================================
//	Funktion RESET des CC1100 nach power_on und warten bis bereit
//=============================================================================
void powerUpReset(void)
	{
	CC1100_CS_HIGH;
	__NOP();
	CC1100_CS_LOW;
	__NOP();
	CC1100_CS_HIGH;
	wait_uSek_CC1101(45); // uSek
	reset_CC1101();
	}
//=============================================================================
// Funktion setzt den Funkkanal (im Bereich 0...8)
//=============================================================================
void set_CHANNEL(unsigned char  c)
	{
	spiStrobe(CC1100_SIDLE);
	spiWriteReg(CC1100_FREQ2,(unsigned char)FREQ2[c]);
	spiWriteReg(CC1100_FREQ1,(unsigned char)FREQ1[c]);
	spiWriteReg(CC1100_FREQ0,(unsigned char)FREQ0[c]);
	spiStrobe(CC1100_SRX);
	spiStrobe(CC1100_SCAL);
	}
//=============================================================================
// Funktion initalisiert CC1100 und setzt CC1100 in den RX Mode
//=============================================================================
void init_CC1101(void)
	{
	init_CC1101_SPI1();
	//unsigned int i_enable = 1;
	memset((void *) TxCC1100.data, 0, MAX_DATA_LENGTH_CC1101);
	memset((void *) RxCC1100.data, 0, MAX_DATA_LENGTH_CC1101);
	// Power up Reset CC1100
	powerUpReset();
	// Konfigurationsregister schreiben
	spiWriteBurstReg(0x00, conf, sizeof(conf));
	// aktuelle Sendeleistung des CC1100 setzen
	spiWriteReg(CC1100_PATABLE, paTable[paTableIndex]);
	// Initialisieren der RSSI und CRC Werte im RxCC1100 Empfangspuffer
	RxCC1100.RSSI = 0x00;
	RxCC1100.CRC_RX = false;
	// CC1101 GDO2 Interrupt auf PC5 erlauben
	init_CC1101_IRQ5();
	// in den RX Mode schalten
	spiStrobe(CC1100_SRX);
	// warten
	wait_uSek_CC1101(120); // 120uSek
	}
//=============================================================================
// init_CC1101_SPI1 initialisiert das SPI Interface zum CC1101
//=============================================================================
void init_CC1101_SPI1(void)
	{

	//===== PB12 - CC1101_CS	Output
	// Clocksystem einschalten
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	// GPIO Strukt anlegen
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);

	// GPIO Strukt initalisieren
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

	// Register schreiben
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// CS Leitung auf 1 setzen
	GPIO_SetBits(GPIOB, GPIO_Pin_12);

	//===== PA5 - CC1101_CLK	Output
	//===== PA6 - CC1101_MISO	Input
	//===== PA7 - CC1101_MOSI	Output

	// Clocksystem ausschalten
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, DISABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, DISABLE);

	// Clocksystem einschalten
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	// GPIO Strukt anlegen
	GPIO_StructInit(&GPIO_InitStructure);

	// GPIO Strukt initalisieren
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			// Alternativ Funktion
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

	// Register schreiben
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Zuweisung der Alternativ Funktion
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);	// CLK
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);	// MISO
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);	// MOSI

	// SPI Strukt anlegen
	SPI_InitTypeDef SPI_InitStructure;
	SPI_StructInit(&SPI_InitStructure);

	// SPI Strukt initialisieren
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	// set to full duplex mode, seperate MOSI and MISO lines
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master; 						// transmit in master mode, NSS pin has to be always high
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b; 					// one packet of data is 8 bits wide
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low; 							// clock is low when idle
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge; 						// data sampled at first edge
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft | SPI_NSSInternalSoft_Set; // set the NSS management to internal and pull internal NSS high
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; 	// SPI frequency is APB2 frequency / 4
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; 					// data is transmitted MSB first

	// Registerschreiben
	SPI_Init(SPI1, &SPI_InitStructure);

	// SPI enable
	SPI_Cmd(SPI1, ENABLE);
	}

//=============================================================================
// Init vom external Interrupt 0
//=============================================================================
void init_CC1101_IRQ5(void)
	{

	//=====	PC4 - CC1101_GDO0	Input
	//===== PC5 - CC1101_GDO2	Input

	// Clocksystem einschalten
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	// GPIO Strukt anlegen
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure);

	// GPIO Strukt initalisieren
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5; 	// PC4-CC1101_GDO0 und PC5-CC1101_GDO2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; 			//	GPIO Input Mode
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; 		//  High speed
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; 			//  PushPull
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; 			//  PullDown

	// Register schreiben
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//==== PC5 - CC1101_GDO2 als Interrupt Quelle anmelden

	// EXTI Struct anlegen
	EXTI_InitTypeDef EXTI_InitStructure;

	// NVIC Struct anlegen
	NVIC_InitTypeDef NVIC_InitStructure;

	// Clocksystem einschalten
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	// CC1101 GDO2 IRQ
	GPIO_StructInit(&GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// Interrupt Konfiguration
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource5);

	// EXT_INT5 Line Konfiguration
	EXTI_InitStructure.EXTI_Line = EXTI_Line5;
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
	}
//=============================================================================
// Funktion initalisiert CC1100 und setzt CC1100 in den Idle Mode
//=============================================================================
void init_CC1101_IDLE(void)
	{
	// Power up Reset CC1100
	powerUpReset();
	// Konfigurationsregister schreiben
	spiWriteBurstReg(0x00, conf, sizeof(conf));
	// aktuelle Sendeleistung des CC1100 setzen
	spiWriteReg(CC1100_PATABLE, paTable[paTableIndex]);
	// Initialisieren der RSSI und CRC Werte im RxCC1100 Empfangspuffer
	RxCC1100.RSSI = 0x00;
	RxCC1100.CRC_RX = false;
	spiStrobe(CC1100_SIDLE);	// in den IDLE Mode setzen
	wait_uSek_CC1101(120); // 120uSek
	}
//=============================================================================
// Funktion setzt CC1100 in PowerDown
//=============================================================================
void init_CC1101_POWERDOWN(void)
	{
	spiStrobe(CC1100_SPWD);		// in den PowerDown Mode setzen
	wait_uSek_CC1101(120); // 120uSek
	}

//=============================================================================
// ISR für empfangenes Datenpaket
//=============================================================================
void ISR_IRQ5_CC1101(void)
	{
	char crc;
	crc = receive_Packet();
	if (crc){
		EXTI_ClearITPendingBit(EXTI_Line5);
		EXTI_ClearFlag(EXTI_Line5);
		print_Packet();
		}
	spiStrobe(CC1100_SIDLE); // Switch to IDLE
	spiStrobe(CC1100_SFRX); // Flush the RX FIFO
	spiStrobe(CC1100_SRX);	// Rx Mode
	EXTI_ClearITPendingBit(EXTI_Line5);
	EXTI_ClearFlag(EXTI_Line5);

	}
//=============================================================================
// Funktion setzt die Adresse des CC1100 (ist im Bereich von 0...255 möglich)
//=============================================================================
void set_ID(unsigned char id)
	{
	if (id > MAX_UID || id < MIN_UID) return; // falsche Adresse abfangen
	spiStrobe(CC1100_SIDLE);				  // in den IDLE Mode setzen
	spiWriteReg(CC1100_ADDR, id);			  // Adressregister schreiben
	spiStrobe(CC1100_SRX);					  // in den RX Mode schalten
	wait_uSek_CC1101(120); 						// 120uSek
	}
//=============================================================================
// Funktion setzt die Sendeleistung des CC1100 siehe auch paTable
//=============================================================================
char set_PA(unsigned char paIdx)
	{
	if (paIdx < 18)
		{
		spiStrobe(CC1100_SIDLE);	// CC1100 in den IDLE Mode setzen
		paTableIndex = paIdx;		// PA Index setzen
		spiWriteReg(CC1100_PATABLE, paTable[paTableIndex]); // PA Wert schreiben
		spiStrobe(CC1100_SRX);		// CC1100 in den RX Mode setzen
		wait_uSek_CC1101(120); 			// 120uSek
		return true;				// Rückgabe true
		}
	return false;					// Rückgabe false da paIdx zu groß
	}

//=============================================================================
// Funktion versendet ein Datenpaket (ohne CarrierSense) danach im Rx Mode
//=============================================================================
void send_Packet(unsigned char ziel,unsigned char quelle,  char *data, unsigned char length)
	{
	//  -----------------------------
	//	| 0 | 1 | 2 | 3  |....| 62  |  Datenpaket aus 63 Byte
	//	-----------------------------
	//	| L | Z | Q | D1 |....| D59 |
	// 	  L--------------------------- Länge
	//        Z----------------------- Ziel
	//            Q------------------- Quelle
	//                D1........D59--- 59 Datenbytes
	unsigned char i;

	if (length > MAX_DATA_LENGTH_CC1101-1 )	// max 59 Byte
		{
		length = MAX_DATA_LENGTH_CC1101-1 ;	// zu große Packete werden auf 59 Byte begrenzt
		}
	// Packetlänge = 1 Byte (Zieladdresse) + data length
	TxCC1100.length = 2 + length;
	// Zieladresse eintragen
	TxCC1100.dest = ziel;
	TxCC1100.source = quelle;

	// Quelladresse eintragen
	//Sendepuffer füllen
	for (i = 0; i < length; i++)
		{
		TxCC1100.data[i] = data[i];
		}

	// Interrupt auf GDO2 verbieten
	EXTI_ClearITPendingBit(EXTI_Line5);
	EXTI_ClearFlag(EXTI_Line5);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);

	// setzt CC1100 in den IDLE Mode
	spiStrobe(CC1100_SIDLE);
    // löscht den TX FIFO des CC1100
    spiStrobe(CC1100_SFTX);
	// Packet in TX FIFO schreiben +Länge+Ziel+Quelle
	spiWriteBurstReg(CC1100_TXFIFO, (char *) &TxCC1100, length+3);
	// setzt CC1100 in den Tx Mode
	spiStrobe(CC1100_STX);

	// warten GDO2=1 - sync transmitted
	while (!CC1100_GDO2);
	// warten GDO2=0 - Packet Ende
	while (CC1100_GDO2);

	// Interrupt auf GDO2 erlauben
	EXTI_ClearITPendingBit(EXTI_Line5);
	EXTI_ClearFlag(EXTI_Line5);
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	}

//=============================================================================
// Funktion liest im Interrupt ein empfangenes Datenpaket aus dem CC1100 aus
//=============================================================================
char receive_Packet()
	{
	// für RSSI und CRC Status vorbereiten
	char status[2];
	// mit 0 initialisieren um aktuelle Länge zu speichern
	unsigned char packetLength = 0;
	// Wenn Bytes im RX FIFO vorhanden sind dann...
	if ((spiReadStatus(CC1100_RXBYTES) & BYTES_IN_RXFIFO))
		{
		// Längenbyte des aktuellen Packetes aus dem RX FIFO lesen (erstes Byte)
        packetLength = spiReadReg(CC1100_RXFIFO); //Das erste Byte ist LängenByte
		// Wenn Packetlänge OK dann...
        if (packetLength <= PACKET_LENGTH)
			{
			// Längenbyte in den RxCC1100 Puffer schreiben
			RxCC1100.length = packetLength;// packetLength;
			// Den Rest des Packetes in RxCC1100 mit aktueller Länge schreiben
            spiReadBurstReg(CC1100_RXFIFO,(char *)RxCC1100.data, packetLength);
            // Lesen der zwei Status Bytes (status[0] = RSSI, status[1] = LQI)
            spiReadBurstReg(CC1100_RXFIFO, status, 2);
			// RSSI Werte in den RxCC1100 Puffer schreiben
			RxCC1100.RSSI = status[I_RSSI];
			// CRC Wert in den RxCC1100 Puffer schreiben
			RxCC1100.CRC_RX = (status[I_LQI] & CRC_OK) >> 7;
			// Zieladresse in den RxCC1100 Puffer schreiben
			RxCC1100.dest = RxCC1100.data[0];
			// Quelladresse in den RxCC1100 Puffer schreiben
			RxCC1100.source = RxCC1100.data[1];
			// Rückgabewert CRC true or false
			return RxCC1100.CRC_RX;
        	}
    	//...sonst...
		else
			{
			// ...CC1100 in den IDLE Mode setzen um...
            spiStrobe(CC1100_SIDLE);
            // ...den RX FIFO zu löschen...
            spiStrobe(CC1100_SFRX);
			// ...und Wert für Rückgabe ist false
            return false;
        	}
		}
	// ...sonst da keine Bytes im RX FIFO
	else
		{
		// ...Rückgabe false
		return false;
		}
	}
//=============================================================================
// Funktion gibt ein Datenpaket auf die serielle Schnittstelle aus
//=============================================================================
void print_Packet()
	{
	unsigned int i;
	char daten[65];
	char z[] = "\n\r";
		sprintf(daten,"\n\rQuelladresse= %u", RxCC1100.source);
		usart2_send(daten);
		sprintf(daten,"\n\rZieladresse = %u", RxCC1100.dest);
		usart2_send(daten);
		sprintf(daten,"\n\rRSSI = %u", RxCC1100.RSSI);
		usart2_send(daten);
		sprintf(daten,"\n\rPaketlänge = %u", RxCC1100.length);
		usart2_send(daten);
		usart2_send(z);
		usart2_send("\n\rDatenpaket = ");
		for (i=0;i<((RxCC1100.length-2));i++)
			{
			daten[i] = RxCC1100.data[2+i];
			}
		daten[i] = '0';//0x00;
		usart2_send(daten);
		usart2_send(z);
	}
