#include <stdint.h>
#include <stm32f4xx.h>
#include <stm32f4xx_hal.h>
#include <stm32f4xx_ll_bus.h>
#include <stm32f4xx_ll_pwr.h>
#include <stm32f4xx_ll_utils.h>
#include <stm32f4xx_ll_cortex.h>
#include <stm32f4xx_ll_system.h>
#include <stm32f4xx_ll_rcc.h>
#include <stm32f4xx_ll_gpio.h>
#include <stm32f4xx_ll_tim.h>
#include <stdio.h>


#include "main.h"
#include "server.h"

void init_clocks();
void init_debug();
void init_gpio();
void init_timer();
void deassert_usbfs_dp ();

int main (void)
{
	HAL_Init();

	init_clocks();  // I.e.:this is really just SystemClock_Config() as in the LL examples
	init_debug();
	init_gpio();
	init_timer();

	// De-assert PA12 (USB_FS_DP) on startup in order to cause the host to re-enumerate the USB device
	deassert_usbfs_dp();

	LL_mDelay(1500);

	printf("\nBegin USB Example...\n");

	init_usb_device();

	LL_mDelay(2000);
	printf("Here's more text.\n");
	LL_mDelay(3000);
	printf("And even more...\n");
	LL_mDelay(1000);
	printf("and more...\n");
	LL_mDelay(5000);
	printf("This should really test out the SWO print capability and OpenOCD's ability to deal with it.\n");
	LL_mDelay(800);
	printf("Here's some ");
	LL_mDelay(400);
	printf("quickly printed ");
	LL_mDelay(400);
	printf("text.\n");
	LL_mDelay(5000);
	printf("We hope you've enjoyed this test of the SWO print function.\n");
	LL_mDelay(1000);
	printf("Now resting in a spinloop... Bye!\n");

	while (1) 
	{	
		LL_mDelay(10);	
	}
}


/**
 * Set up clocks (basically stolen from the MSP main.c)
 */ 
void init_clocks ()
{
	// Turn off MCO1 output
	auto cfgr = RCC->CFGR;
	cfgr &= 0x7 << RCC_CFGR_MCO1_Pos;  // clear MCO1 output
	cfgr |= 0x7 << RCC_CFGR_MCO1_Pos;  // MCO1 output = PLLCLK/2
	RCC->CFGR = cfgr;

	LL_RCC_HSE_Enable();
	while (LL_RCC_HSE_IsReady() == 0) {}

	LL_RCC_HSI_Enable();
	while (LL_RCC_HSI_IsReady() ==0) {}

	LL_RCC_PLL_Disable();
	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_4, 168, LL_RCC_PLLP_DIV_2);
	LL_RCC_PLL_ConfigDomain_48M(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_4, 168, LL_RCC_PLLQ_DIV_7);
	
	LL_RCC_PLL_Enable();
	while (LL_RCC_PLL_IsReady() == 0) {};

	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);

	LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);

	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
	
	LL_Init1msTick(TICK_RATE);
	LL_SYSTICK_EnableIT();
	LL_SetSystemCoreClock(TICK_RATE);

	SystemCoreClockUpdate();
}


/**
 * Configure the ITM/ETM for debug printing
 */
void init_debug ()
{
#ifdef SWO_DEBUG
	// Configure the TPIU
	CoreDebug->DEMCR |= (0x1 << CoreDebug_DEMCR_TRCENA_Pos);  // enable the trace port within the ARM Core debug component
	TPI->CSPSR = 0x1;  // set port width to 1
	TPI->FFCR  = 0x0100; // disable the TPIU formatter 
	TPI->SPPR  = 0x02;  // Async / NRZ (8N1 serial mode) 
	TPI->ACPR  = (168000000 / 1000000);
	
	// Configure the DBGMCU
	LL_DBGMCU_SetTracePinAssignment(LL_DBGMCU_TRACE_ASYNCH);
	
	// Configure the ITM
	while (ITM->TCR & 0x00800000U) {};
	ITM->LAR = 0xC5ACCE55;  // lock access register needs this exact value

	// trace control register needs: an ATB number different than zero (1), the SWO clock timestamp counter enabled, and the global ITM bit enabled (default)
	// Also, ITM_TCR_TSENA_Msk causes repeated timestamps and garbage characters to spam openocd's swout file
	// 
	//ITM_TCR_SYNCENA_Msk | ITM_TCR_DWTENA_Msk |
	ITM->TCR = ITM_TCR_DWTENA_Msk | ITM_TCR_ITMENA_Msk;  
	ITM->TPR = ITM_TPR_PRIVMASK_Msk;                 // trace privilege register: unmask stimulus unmask ports 7:0 (default)
	ITM->TER = 0x01;                                 // trace enable register: enable stimulus port 0 (default)
	
	// further values written to ITM->PORT[0] will be written to the SWO/SWV...
#else
	CoreDebug->DEMCR &= ~(0x1 << CoreDebug_DEMCR_TRCENA_Pos); // disable the trace port
	ITM->LAR = 0xC5ACCE55;
	ITM->TCR = 0x0;         // SWO clock and global ITM bit both disabled
	ITM->TER = 0x0;
	ITM->TPR = 0x0;
#endif
}


/**
 * Set up GPIOC so that we can blink an LED to test
 */
void init_gpio ()
{	
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOF);
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	LL_GPIO_InitTypeDef f9 = {0};
	f9.Pin = LL_GPIO_PIN_9;
	f9.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	f9.Speed = LL_GPIO_SPEED_FREQ_LOW;
	f9.Mode = LL_GPIO_MODE_OUTPUT;
	f9.Alternate = 0;
	LL_GPIO_Init(GPIOF, &f9);
	LL_GPIO_ResetOutputPin(GPIOF, LL_GPIO_PIN_9);
}


/**
 * Set up TIM2 as a PWM device
 */
void init_timer ()
{
	// Defines a 1Mhz/200Hz base timer
	uint32_t basefreq = 200;				// PWM frequency 5Hz
	uint32_t tick_rate = 1000000;           // Timer ticks per period
	uint16_t period = static_cast<uint16_t>(tick_rate / basefreq);

	//LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA); // redundant but this lets you comment out other sections
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

	{	
		LL_TIM_InitTypeDef tim2 = {0};
		LL_TIM_StructInit(&tim2);
		tim2.Autoreload = period - 1;
		tim2.Prescaler = TICK_RATE / tick_rate;
		tim2.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
		tim2.CounterMode = LL_TIM_COUNTERMODE_UP;
		LL_TIM_Init(TIM2, &tim2);

		LL_TIM_OC_InitTypeDef oc2 = {0};
		LL_TIM_OC_StructInit(&oc2);
		oc2.CompareValue = period / 2;
		oc2.OCMode = LL_TIM_OCMODE_PWM2;
		oc2.OCState = LL_TIM_OCSTATE_ENABLE;
		oc2.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
		LL_TIM_OC_Init(TIM2, LL_TIM_CHANNEL_CH1, &oc2);
		LL_TIM_OC_ConfigOutput(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_OCPOLARITY_HIGH);

		LL_TIM_SetUpdateSource(TIM2, LL_TIM_UPDATESOURCE_COUNTER);
		LL_TIM_EnableUpdateEvent(TIM2);
		LL_TIM_EnableIT_UPDATE(TIM2);
		LL_TIM_EnableIT_CC1(TIM2);

		NVIC_EnableIRQ(TIM2_IRQn);
		LL_TIM_SetCounter(TIM2, 0);
		LL_TIM_EnableCounter(TIM2);
	}
}


void deassert_usbfs_dp ()
{
	printf("Deasserting USB_FS_DP....       ");

	LL_GPIO_InitTypeDef pa12;
	pa12.Pin = LL_GPIO_PIN_12;
	pa12.Mode = LL_GPIO_MODE_OUTPUT;
	pa12.Speed = LL_GPIO_SPEED_FREQ_LOW;
	pa12.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	LL_GPIO_Init(GPIOA, &pa12);
	
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_12);
	LL_mDelay(150);
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_12);

	printf("[\e[1;32mDone\e[0m]\n");

}


extern "C" void Error_Handler(void)
{
	while(1)
	{
		/* Insert a delay */
		HAL_Delay(50);
	}
}

void HAL_MspInit(void)
{
  	// Seems RCC_SYSCFG and RCC_PWR have their own CLKs to enable
	__HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();            

}