/*****************************************************************************
*
*  spi.h  - CC3000 Host Driver Implementation.
*  Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*    Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*****************************************************************************/


#ifndef __SPI_H__
#define __SPI_H__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>


//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef  __cplusplus
extern "C" {
#endif

typedef void (*gcSpiHandleRx)(void *p);
typedef void (*gcSpiHandleTx)(void);

extern unsigned char wlan_tx_buffer[];

extern _Bool is22;
//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************
extern void SpiOpen(gcSpiHandleRx pfRxHandler);
extern void SpiClose(void);
extern long SpiWrite(unsigned char *pUserBuffer, unsigned short usLength);
extern void SpiResumeSpi(void);
//extern void SpiCleanGPIOISR(void);



//*****************************************************************************
void WriteWlanPin(unsigned char val);	// Set/Reset WLAN Enable Leitung

void WlanInterruptEnable(void);			// WLAN Interrupt Enable
void WlanInterruptDisable(void);		// WLAN Interrupt Disable
long ReadWlanInterruptPin(void);		// WLAN IRQ Leitung lesen

void WLAN_IRQ_Handler(void);			// WLAN IRQ Handler
void DMA_Int_Handler(void);				// DMA 	IRQ Handler
void DMA_und_SPI_ClearInterruptFlag(void);	// Clear Flags DMA und SPI

void SpiReadHeader(void);
void SpiReadData(unsigned char *data, unsigned short size);
long SpiReadDataCont(void);

void SpiReadWriteStringInt(uint32_t ulTrueFalse, const uint8_t *ptrData, uint32_t ulDataSize);
void SpiReadWriteString(uint32_t ulTrueFalse, const uint8_t *ptrData, uint32_t ulDataSize);
//void SpiTriggerRxProcessing(void);

//void SpiDelayOneSecond(void);
void SysCtlDelay(unsigned long ulDelay);
void wait_uSek(unsigned long us);
//*****************************************************************************



//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef  __cplusplus
}
#endif // __cplusplus

#endif

