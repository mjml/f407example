
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

#include "main.h"

void init_clocks();
void init_gpio();
void init_timer();

int main (void)
{
	init_clocks();  // I.e.:this is really just SystemClock_Config() as in the LL examples
	init_gpio();
	init_timer();

	while (1) {
		/*
		if (toggle_led) {
			toggle_led = 0;
			LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_13);
		}
		*/
		
	}
}


/**
 * Set up clocks (basically stolen from the MSP main.c)
 */ 
void init_clocks ()
{
	auto cfgr = RCC->CFGR;
	cfgr &= 0x7 << RCC_CFGR_MCO_Pos;
	cfgr |= 0x6 << RCC_CFGR_MCO_Pos;
	RCC->CFGR = cfgr;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_AFIOEN;
    GPIOA->CRH &= ~0X0000000F;
    GPIOA->CRH |=  0X0000000B;

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
