
#include <stdint.h>
#include <math.h>
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_ll_tim.h"


#include <list>
#include "chirp.h"


void init_clocks();
void init_timer();

int main ()
{
	HAL_Init();

	init_clocks();
	Chirp::Initialize();
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
	clk.AHBCLKDivider = RCC_SYSCLK_DIV2;
	clk.APB1CLKDivider = RCC_HCLK_DIV2;
	clk.APB2CLKDivider = RCC_HCLK_DIV1;
	HAL_RCC_ClockConfig(&clk, FLASH_ACR_LATENCY_2);
}


/**
 * Set up TIM2 as a PWM device
 */
void init_timer ()
{
	// set most of the TIM2 registers to their Reset state
	TIM2->CR1 = 0x0; // defaults: no division, edge-aligned, up-count, repeat count, UEV enabled, counter disabled
	TIM2->CR2 = 0x0; // defaults
	TIM2->DIER |= TIM_DIER_UIE; // interrupt on update
	TIM2->CCER = (0x01 << TIM_CCER_CC1E); // output on OC1
	TIM2->CCMR1 |= (0x6 << TIM_CCMR1_OC1M_Pos)   // PWM mode 1
	            |  (0x1 << TIM_CCMR1_OC1PE_Pos); // Preload CCR1;

	// Configure the PA0 as TIM2_CH1.
	GPIOA->CRL |= (0x01 << GPIO_CRL_CNF0_Pos)   // Alternate function
	           |  (0x01 << GPIO_CRL_MODE0_Pos); // 10 Mhz max

	// Determine our phase from the desired sampling rate:
	uint32_t f = 120000;
	uint32_t t_rate = 36000000;

	// Sample period of 600 timer ticks
	TIM2->CCR1 = t_rate / f;

	// start the TIM2 counter
	TIM2->CR1 |= (0x01 << TIM_CR1_CEN_Pos);
	

}


/**
 * 
 */
void TIM2_IRQHandler ()
{
	Chirp::Service();
	
}
