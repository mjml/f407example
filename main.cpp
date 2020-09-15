
#include <stdint.h>
#include <math.h>
#include <stm32f103xb.h>
#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_ll_gpio.h>
#include <stm32f1xx_ll_tim.h>

#include <list>
#include "chirp.h"

void init_clocks();
void init_gpio();
void init_timer();

int main ()
{
	HAL_Init();

	init_clocks();
	init_gpio();
	init_timer();

	while (1) {
		;
	}
}

/**
 * Set up clocks (basically stolen from the MSP main.c)
 */ 
void init_clocks ()
{
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_TIM2_CLK_ENABLE();
	

	

	RCC_OscInitTypeDef osc;
	osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	osc.HSEState = RCC_HSE_ON;
	osc.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	osc.HSIState = RCC_HSI_ON;
	osc.PLL.PLLState = RCC_PLL_ON;
	osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	osc.PLL.PLLMUL = RCC_PLL_MUL9;
	HAL_RCC_OscConfig(&osc);

	RCC_ClkInitTypeDef clk;
	clk.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
	clk.APB1CLKDivider = RCC_HCLK_DIV2;
	clk.APB2CLKDivider = RCC_HCLK_DIV1;
	HAL_RCC_ClockConfig(&clk, FLASH_ACR_LATENCY_2);

	
}

/**
 * Set up GPIOC so that we can blink an LED to test
 */
void init_gpio ()
{
	LL_GPIO_InitTypeDef c13;
	c13.Pin = LL_GPIO_PIN_13;
	c13.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	c13.Speed = LL_GPIO_SPEED_FREQ_LOW;
	c13.Mode = LL_GPIO_MODE_OUTPUT;
	LL_GPIO_Init(GPIOC, &c13);
	LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_13);
	
	LL_GPIO_InitTypeDef a0;
	a0.Pin = LL_GPIO_PIN_0;
	a0.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	a0.Speed = LL_GPIO_SPEED_FREQ_LOW;
	a0.Mode = LL_GPIO_MODE_ALTERNATE;
	LL_GPIO_Init(GPIOA, &a0);
}


/**
 * Set up TIM2 as a PWM device
 */
void init_timer ()
{
	// set most of the TIM2 registers to their Reset state
	TIM2->CR1 = 0x0; // defaults: no division, edge-aligned, up-count, repeat count, UEV enabled, counter disabled
	TIM2->CR2 = 0x0; // defaults
	TIM2->DIER  |= (0x01 << TIM_DIER_UIE_Pos);   // interrupt on update
	TIM2->CCER   = (0x01 << TIM_CCER_CC1E_Pos);  // output on OC1
	TIM2->CCMR1 |= (0x06 << TIM_CCMR1_OC1M_Pos); // PWM mode 1
	
	// temporary: set a prescaler to test
	TIM2->PSC = 7200;

	// Determine our period from the desired sampling rate:
	uint32_t f = 5;                              // PWM frequency 3Hz
	uint32_t tick_rate = 10000;                  // Timer ticks per period
	uint16_t period = static_cast<uint16_t> (tick_rate / f);

	TIM2->ARR = period - 1;
	TIM2->CCR1 = period / 2;

	// enable the interrupt
	NVIC_EnableIRQ(TIM2_IRQn);
	NVIC_SetPriority(TIM2_IRQn, 1);

    // force an update
	TIM2->DIER |= (0x01 << TIM_DIER_UIE_Pos); // ensure this is set!
	TIM2->EGR |= (0x01 << TIM_EGR_UG_Pos);

	// start the TIM2 counter
	TIM2->CR1 |= (0x01 << TIM_CR1_CEN_Pos);

}