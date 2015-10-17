#include "init.h"



// Zuweisung Handle USE_DEVICE_MODE
//__ALIGN_BEGIN USB_OTG_CORE_HANDLE USB_OTG_dev __ALIGN_END;



void InitSysTick(void){
	// Bei f= 168000000Hz und Teiler 100 gibt es alle 10ms einen SysTick
	// SysTick Intervall = (1/f) * (SystemCoreClock / 100) = 0,01 Sekunde
	if (SysTick_Config(SystemCoreClock / 100)) { while (1); }
	}




void init_board(void){

	//=====================================================================
	// 	LTC2950 - Power-Control
	//=====================================================================
	//init_POWER_ON();



	//=====================================================================
	// 	UART - Serielle Schnittstelle; 8N1, 921600 Bit/s
	//=====================================================================
	//usart2_init();
	//usart2_send("\r\n\r\nStart Init ...\r\n");
	//usart6_init();
	//uart6_init_IRQ();
	//usart6_send("\r\n\r\nStart Init ...\r\n");




	//=====================================================================
	// 	USB - Virtuelle Serielle Schnittstelle
	//=====================================================================
//		USBD_Init(&USB_OTG_dev, 	// pdev: device instance
//				USB_OTG_FS_CORE_ID, // core_address: USB OTG core ID
//				&USR_desc, 			// Device Descriptor
//				&USBD_CDC_cb, 		// Device Call Back
//				&USR_cb 			// usr_cb: User callback structure address
//				);



	//=====================================================================
	// 	CC3000 - 2,4GHz WLAN Transceiver
	//=====================================================================
	//	 scanner();



	//=====================================================================
	// 	CC1101 - 868MHz Transceiver
	//=====================================================================
	//	init_CC1101_SPI1();
	//	init_CC1101_IRQ5();
	//	init_CC1101();

	//=====================================================================
	// 	MPU-9150 - IMU
	//=====================================================================
	//	MPU_I2C1_init();
	//  MPU_IRQ_init();



	//=====================================================================
	// 	TDA6416 - I/O Port-Expander
	//=====================================================================
	// 	init_TAC6416_I2C3();
	// 	init_TAC6416_IRQ8();


	//=====================================================================
	// 	SD-Card mit FAT-File System
	//=====================================================================
	//sdcard_init ();



	//=====================================================================
	// 	LED
	//=====================================================================
	//init_LED();




	//=====================================================================
	// 	Tasten Taster1 () Taster2 ()
	//=====================================================================
	//init_TASTER1();
	//init_Taster1_IRQ13();

	//init_TASTER2();
	//init_Taster2_IRQ0();




	//=====================================================================
	// 	Signalgeber
	//=====================================================================
	// init_BEEPER();



	//=====================================================================
	// 	MAX9634 - Strommessung
	//=====================================================================
	// init_MAX9634_ADC1IN9




	//=====================================================================
	// 	Li-Ion Akku - Spannungsmessung
	//=====================================================================
	//init_VBAT_ADC1IN18();




}


void StartBootLoader(unsigned int BootLoaderStatus)
	{
//	SysMemBootJump=(void (*)(void)) (*((u32*) 0x1fff0004));
//	if (BootLoaderStatus==1)
//		{
//		RCC_DeInit();
//		SysTick->CTRL=0;
//		SysTick->LOAD=0;
//		SysTick->VAL=0;
//		//RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
//		__set_PRIMASK(1);		// Disable Interrupts
//		__set_MSP(0x200010000); // Set the main stack pointer
//		SysMemBootJump();
//		while(1){;}
//		}
	}






