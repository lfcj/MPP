#include "interrupts.h"

 uint16_t A_wert = 0;
 uint16_t B_wert = 0;
 uint16_t start_wert = 0;
 uint16_t end_wert = 0;
 uint16_t messung_der_periodendauer = 0;
 uint32_t differenz_wert = 0;

int32_t timer = 0;
int32_t Tas1=0;
int32_t Tas2=0;
int16_t T1=0;
int16_t T2=0;
char tasten[50];

 float frequenz = 0;
 float periodendauer = 0;
	char bufi[40];

char	string_out[50];
char	string_in[50];
uint16_t j = 0;

void usart_2_print(char * zeichenkette){

}

void hard_fault_handler_c(unsigned int * hardfault_args);

//=========================================================================
void NMI_Handler(void)
{
}

//=========================================================================
void HardFault_Handler(void) {
	asm ("TST LR, #4");
	asm ("ITE EQ");
	asm ("MRSEQ R0, MSP");
	asm ("MRSNE R0, PSP");
	asm ("B hard_fault_handler_c");
}

//=========================================================================
void MemManage_Handler(void)
{
  while (1){ ; }
}

//=========================================================================
void BusFault_Handler(void)
{
	while (1){ ; }
}

//=========================================================================
void UsageFault_Handler(void)
{
	while (1){ ; }
}

//=========================================================================
void SVC_Handler(void)
{
}

//=========================================================================
void DebugMon_Handler(void)
{
}

//=========================================================================
//void PendSV_Handler(void)
//{
//}

//=========================================================================
void SysTick_Handler(void){
    static unsigned long SysTickCounter0 = 0;
    static unsigned long SysTickCounter1 = 0;
    static unsigned long SysTickCounter2 = 0;
    static unsigned long SysTickPeriode = 0;

    SysTickCounter0++;
    SysTickCounter1++;
    SysTickCounter2++;

    //CoOS_SysTick_Handler(); //CoOs arch.c
    CoOS_SysTick_Handler();

    // CC3000 alle 500ms CC3000_select() aufrufen
	if (SysTickCounter2 >= 50) {
		SysTickCounter2 = 0;
		if ( CC3000_DHCP_done == true && CC3000_is_Connected == true) {
		CC3000_select();}
		}

    // SDCARD alle 10ms Timer für Wartezeiten runterzählen
    //SDCARD_SysTick_Handler();

    if (SysTickCounter1 >= 10) {
        if (timer >= 0 ) { timer--; SysTickCounter1 = 0;}
        else {timer = 0;}
    	}

	if (SysTickCounter0 >= 10) {
		SysTickPeriode++;
		if (SysTickPeriode == 1) {LED_RT_OFF;}
		if (SysTickPeriode >= 29) {LED_RT_ON; SysTickPeriode = 0;}
		SysTickCounter0 = 0;
		}

}

//=========================================================================
void WWDG_IRQHandler(void)
{

}


//=========================================================================
void EXTI0_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line0) == SET)
        {

        }
	    //TASTER2_IRQ();
}

//=========================================================================
void EXTI1_IRQHandler(void) {

}


//=========================================================================
void EXTI2_IRQHandler(void) {

}


//=========================================================================
void EXTI3_IRQHandler(void) {

}


//=========================================================================
void EXTI4_IRQHandler(void) {

}

//=========================================================================
void EXTI9_5_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line5) == SET) {
		ISR_IRQ5_CC1101();
		}
	if (EXTI_GetITStatus(EXTI_Line6) == SET) {
		//uart_send("6 ");
		}
	if (EXTI_GetITStatus(EXTI_Line7) == SET) {
		uart_send("7 ");
		}
	if (EXTI_GetITStatus(EXTI_Line8) == SET) {
		//ISR_IRQ8_TAC6416();
		}
	if (EXTI_GetITStatus(EXTI_Line9) == SET) {
		//uart_send("9 ");
		}
}


//=========================================================================
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line10) == SET) {
		//uart_send("W\r\n");
		WLAN_IRQ_Handler();
		}

	if(EXTI_GetITStatus(EXTI_Line11) == SET) {
		//MPU_IRQ_Handler();
		}

	if (EXTI_GetITStatus(EXTI_Line12) == SET) {
		//uart_send("12 ");
		}

	if(EXTI_GetITStatus(EXTI_Line13) == SET) {
		EXTI_ClearFlag(EXTI_Line13);
		EXTI_ClearITPendingBit(EXTI_Line13);
		//TASTER1_IRQ();
		}

}


//=========================================================================
void RTC_Alarm_IRQHandler(void)
{
  if(RTC_GetITStatus(RTC_IT_ALRA) != RESET)
  {
    RTC_ClearITPendingBit(RTC_IT_ALRA);
    EXTI_ClearITPendingBit(EXTI_Line17);
  }

}


//=========================================================================
void ADC_IRQHandler(void)
{
	if(ADC_GetITStatus(ADC1, ADC_IT_EOC) == SET)
		{
		// ... Code für Ende Wandlung
		//ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
		}
	if(ADC_GetITStatus(ADC1, ADC_IT_AWD) == SET)
		{
		// ... Code für analogen Watchdog
		//ADC_ClearITPendingBit(ADC1, ADC_IT_AWD);
		}
}


//=========================================================================
void USART2_IRQHandler(void){

}


//=========================================================================
void USART6_IRQHandler(void){

}


//=========================================================================
void DMA2_Stream6_IRQHandler(void){

}


//=========================================================================
void RTC_WKUP_IRQHandler(void)
{
  if(RTC_GetITStatus(RTC_IT_WUT) != RESET)
  {
	LED_GR_TOGGLE;
	RTC_ClearITPendingBit(RTC_IT_WUT);
	EXTI_ClearITPendingBit(EXTI_Line22);
  }
}


//=========================================================================
void TIM5_IRQHandler(void)
{

}


//=========================================================================
void TIM7_IRQHandler(void)
{

}

//=========================================================================
void TIM6_DAC_IRQHandler()
{

}


//=========================================================================
// From Joseph Yiu, minor edits by FVH
// hard fault handler in C,
// with stack frame location as input parameter
// called from HardFault_Handler in file xxx.s
void hard_fault_handler_c(unsigned int * hardfault_args) {
//	unsigned int stacked_r0;
//	unsigned int stacked_r1;
//	unsigned int stacked_r2;
//	unsigned int stacked_r3;
//	unsigned int stacked_r12;
//	unsigned int stacked_lr;
//	unsigned int stacked_pc;
//	unsigned int stacked_psr;
//
//	stacked_r0 = ((unsigned long) hardfault_args[0]);
//	stacked_r1 = ((unsigned long) hardfault_args[1]);
//	stacked_r2 = ((unsigned long) hardfault_args[2]);
//	stacked_r3 = ((unsigned long) hardfault_args[3]);
//
//	stacked_r12 = ((unsigned long) hardfault_args[4]);
//	stacked_lr = ((unsigned long) hardfault_args[5]);
//	stacked_pc = ((unsigned long) hardfault_args[6]);
//	stacked_psr = ((unsigned long) hardfault_args[7]);
//
//	printf("\n\n[Hard fault handler - all numbers in hex]\n");
//	printf("R0 = %x\n", stacked_r0);
//	printf("R1 = %x\n", stacked_r1);
//	printf("R2 = %x\n", stacked_r2);
//	printf("R3 = %x\n", stacked_r3);
//	printf("R12 = %x\n", stacked_r12);
//	printf("LR [R14] = %x  subroutine call return address\n", stacked_lr);
//	printf("PC [R15] = %x  program counter\n", stacked_pc);
//	printf("PSR = %x\n", stacked_psr);
//	printf("BFAR = %x\n", (*((volatile unsigned long *) (0xE000ED38))));
//	printf("CFSR = %x\n", (*((volatile unsigned long *) (0xE000ED28))));
//	printf("HFSR = %x\n", (*((volatile unsigned long *) (0xE000ED2C))));
//	printf("DFSR = %x\n", (*((volatile unsigned long *) (0xE000ED30))));
//	printf("AFSR = %x\n", (*((volatile unsigned long *) (0xE000ED3C))));
//	printf("SCB_SHCSR = %x\n", SCB ->SHCSR);
//
//	if (SCB->HFSR & SCB_HFSR_DEBUGEVT_Msk) {
//		printf(" This is a DEBUG FAULT\n");
//	} else if (SCB ->HFSR & SCB_HFSR_FORCED_Msk) {
//		printf(" This is a FORCED FAULT\n");
//
//		if (SCB ->CFSR & (0x1 << SCB_CFSR_USGFAULTSR_Msk)) {
//			printf("undefined instruction\n");
//		} else if (SCB ->CFSR & (0x2 << SCB_CFSR_USGFAULTSR_Pos)) {
//			printf("instruction makes illegal use of the EPSR\n");
//		} else if (SCB ->CFSR & (0x4 << SCB_CFSR_USGFAULTSR_Pos)) {
//			printf("Invalid PC load UsageFault, caused by an invalid PC load by EXC_RETURN\n");
//		} else if (SCB ->CFSR & (0x8 << SCB_CFSR_USGFAULTSR_Pos)) {
//			printf("The processor does not support coprocessor instructions\n");
//		} else if (SCB ->CFSR & (0x100 << SCB_CFSR_USGFAULTSR_Pos)) {
//			printf("Unaligned access\n");
//		} else if (SCB ->CFSR & (0x200 << SCB_CFSR_USGFAULTSR_Pos)) {
//			printf("Divide by zero\n");
//		}
//	} else if (SCB ->HFSR & SCB_HFSR_VECTTBL_Pos) {
//		printf(" This is a BUS FAULT\n");
//	}
	while (1){;}
}
