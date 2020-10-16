#include <stdint.h>
#include <stm32f103xb.h>
#include <stm32f1xx_ll_bus.h>
#include <stm32f1xx_ll_pwr.h>
#include <stm32f1xx_ll_utils.h>
#include <stm32f1xx_ll_cortex.h>
#include <stm32f1xx_ll_system.h>
#include <stm32f1xx_ll_rcc.h>
#include <stm32f1xx_ll_gpio.h>
#include <stm32f1xx_ll_tim.h>
#include <stdio.h>


#include "main.h"

void init_clocks();
void init_debug();
void init_gpio();
void init_timer();

int main (void)
{
	init_clocks();  // I.e.:this is really just SystemClock_Config() as in the LL examples
	init_debug();
	init_gpio();
	init_timer();

	printf("Hello, SWOrld!\n");

	while (1) {
		/*
		if (toggle_led) {
			toggle_led = 0;
			LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_13);
		}
		*/
		LL_mDelay(2000);
		
	}
}


/**
 * Set up clocks (basically stolen from the MSP main.c)
 */ 
void init_clocks ()
{
	auto cfgr = RCC->CFGR;
	cfgr &= 0x7 << RCC_CFGR_MCO_Pos;  // clear MCO output
	cfgr |= 0x7 << RCC_CFGR_MCO_Pos;  // MCO output = PLLCLK/2
	RCC->CFGR = cfgr;                 
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
    GPIOA->CRH &= ~0X0000000F;
    GPIOA->CRH |=  0X0000000B; // MCO output on PA8

	LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);

	LL_RCC_HSE_Enable();
	while (LL_RCC_HSE_IsReady() == 0) {};

	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE_DIV_1,LL_RCC_PLL_MUL_9);
	LL_RCC_PLL_Enable();
	while (LL_RCC_PLL_IsReady() == 0) {};

	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);

	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

	SystemCoreClockUpdate();

	LL_Init1msTick(TICK_RATE);

	LL_SetSystemCoreClock(TICK_RATE);

	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_GPIOA);
}


/**
 * Configure the ITM/ETM for debug printing
 */
void init_debug ()
{
#ifdef SWO_DEBUG
	// Configure the TPIU
	CoreDebug->DEMCR |= (0x1 << CoreDebug_DEMCR_TRCENA_Pos);  // enable the trace port within the ARM Core debug component
	TPI->CSPSR = 0x1;  // set port size to 1
	TPI->FFCR = 0x102; // enable the TPIU formatter
	TPI->SPPR = 0x0;   // set "trace port mode" protocol (not Manchester or NRZ)
	TPI->ACPR = 72000000 / 1000000;
	
	// Configure the DBGMCU
	LL_DBGMCU_SetTracePinAssignment(LL_DBGMCU_TRACE_ASYNCH);

	// Configure the ITM
	while (ITM->TCR & 0x00800000U) {};
	ITM->LAR = 0xC5ACCE55;  // lock access register needs this exact value
	ITM->TCR = 0x00010005;  // trace control register needs: an ATB number different than zero (1), the SWO clock timestamp counter enabled, and the global ITM bit enabled
	ITM->TER = 0x01;        // trace enable register: enable stimulus port 0
	ITM->TPR = 0x01;        // trace privilege register: unmask stimulus unmask ports 7:0

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
	LL_GPIO_InitTypeDef c13;
	c13.Pin = LL_GPIO_PIN_13;
	c13.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	c13.Speed = LL_GPIO_MODE_OUTPUT_2MHz;
	c13.Mode = LL_GPIO_MODE_OUTPUT;
	LL_GPIO_Init(GPIOC, &c13);
	LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_13);
}

/**
 * Set up TIM2 as a PWM device
 */
void init_timer ()
{
	uint32_t basefreq = 5;				// PWM frequency 5Hz
	uint32_t tick_rate = 10000; // Timer ticks per period
	uint16_t period = static_cast<uint16_t>(tick_rate / basefreq);
	
	{ // Defines a 10000Hz/5Hz base timer
		
		LL_TIM_InitTypeDef tim2;
		LL_TIM_StructInit(&tim2);
		tim2.Autoreload = period - 1;
		tim2.Prescaler = TICK_RATE / tick_rate;
		tim2.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
		tim2.CounterMode = LL_TIM_COUNTERMODE_UP;
		LL_TIM_Init(TIM2, &tim2);

		LL_TIM_OC_InitTypeDef oc2;
		LL_TIM_OC_StructInit(&oc2);
		oc2.CompareValue = period / 2;
		oc2.OCMode = LL_TIM_OCMODE_PWM2;
		oc2.OCState = LL_TIM_OCSTATE_ENABLE;
		oc2.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
		LL_TIM_OC_Init(TIM2, LL_TIM_CHANNEL_CH1, &oc2);
		LL_TIM_OC_ConfigOutput(TIM2, LL_TIM_CHANNEL_CH1, LL_TIM_OCPOLARITY_HIGH);

		LL_GPIO_InitTypeDef a0; // PA0 is TIM2.CH1
		LL_GPIO_StructInit(&a0);
		a0.Pin = LL_GPIO_PIN_0;
		a0.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
		a0.Speed = LL_GPIO_MODE_OUTPUT_2MHz;
		a0.Mode = LL_GPIO_MODE_ALTERNATE;
		LL_GPIO_Init(GPIOA, &a0);
		LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_0);

		LL_TIM_EnableCounter(TIM2);
	}
}
