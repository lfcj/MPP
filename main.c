#include "main.h"


int main(void){

	char puffer[50];
	unsigned char i = 1;

	SystemInit();

	InitSysTick();

    start_RTC();

    init_LED();
    usart2_init();

    // Bei Nutzung des CC1101 Transceivers einbinden
    init_CC1101_SPI1(); 	// SPI Initialisierung
    init_CC1101_IRQ5(); 	// Interrupt Initialisierung
    init_CC1101();      	// CC1101 Konfigurieren
    set_CHANNEL(10); 	   	// Einstellen des Funkkanals
    set_ID(10);				// Setzen der eigenen Funk-ID

    // Bei Nutzung des CC3000-WLAN Transceivers einbinden
    CC3000_Init();
    CC3000_ScanNetworks(visibleNetworks);

    // Bei Nutzung des CoOS einbinden
    // CoInitOS ();        	// CoOS Initialisierung
    // CoCreateTask (...); 	// Anmelden der gewünschten Task
    // CoStartOS ();       	// Starten des CoOS

     while(1){

     }
}
