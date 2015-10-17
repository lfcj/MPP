#ifndef CC3000_HAL_H
#define CC3000_HAL_H

//=========================================================================
// Anschlussbelegung des CC3000 WLAN Transceivers
//=========================================================================
//	CC3000-PIN		STM32-Portleitung
//	WLAN-Enable		PC13
// 	WLAN-IRQ		PA10
// 	SPI-CS			PC1
// 	SPI-CLK			PB10
// 	SPI-MISO		PC2
// 	SPI-MOSI		PC3
//=========================================================================

void init_CC3000_SPI(void);


#endif // CC3000_HAL_H
