
#include <stdint.h>
#include <math.h>
#include <stm32f103xb.h>
#include <stm32f1xx_hal.h>

#include <stm32f1xx_ll_gpio.h>
#include <stm32f1xx_ll_tim.h>

#include <list>
#include "chirp.h"

void init_interrupts();
void init_clocks();
void init_gpio();
void init_timer();

extern "C" int toggle_led;

int main (void)
{
	__enable_irq();	

	HAL_Init();

	init_interrupts();
	init_clocks();
	init_gpio();
	init_timer();

	__enable_irq();
	
	
	while (1) {
		/*
		if (toggle_led) {
			toggle_led = 0;
			LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_13);
		}
		*/
		
	}
}


void init_interrupts ()
{
	
}

/**
 * Set up clocks (basically stolen from the MSP main.c)
 */ 
void init_clocks ()
{
	// E.g.: HAL_Init() /-> HAL_MspInit()
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	__HAL_RCC_AFIO_CLK_ENABLE();
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_AFIO_REMAP_SWJ_NOJTAG();
		
	// E.g.: SystemClock_Config()
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
	HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2);

}

/**
 * Set up GPIOC so that we can blink an LED to test
 */
void init_gpio ()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	LL_GPIO_InitTypeDef c13;
	c13.Pin = LL_GPIO_PIN_13;
	c13.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
	c13.Speed = LL_GPIO_SPEED_FREQ_LOW;
	c13.Mode = LL_GPIO_MODE_OUTPUT;
	LL_GPIO_Init(GPIOC, &c13);
	LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_13);
	//LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_13);
}

extern "C" void TIM2_IRQHandler(void);

/**
 * Set up TIM2 as a PWM device
 */
void init_timer ()
{
	// determine our period from the desired sampling rate:
	uint32_t f = 5;                              // PWM frequency 5Hz
	uint32_t tick_rate = 10000;                  // Timer ticks per period
	uint16_t period = static_cast<uint16_t> (tick_rate / f);

	// set most of the TIM2 registers to their Reset state
	TIM2->CR1 = 0x0; // defaults: no division, no ARR preload, edge-aligned, up-count, repeat counter, UEV by default methods, UEV enabled, counter disabled

	// Eg: HAL_TIM_PWM_Init /-> TIM_Base_SetConfig
	__HAL_RCC_TIM2_CLK_ENABLE();

	TIM2->ARR = period - 1; // Auto-reload period
	TIM2->PSC = 7200;       // Prescaler
	TIM2->EGR = TIM_EGR_UG; // Force update (to load the prescaler)

    // Eg: HAL_TIMEx_MasterConfigSynchronization
	TIM2->CR2 = 0x0; // defaults: CH1 connected to TI1, Master mode reset, DMA req on CCx event
	TIM2->SMCR = 0x0; // defaults: Non-inverted external trigger, no external clock, ET prescaler off, no ET filter, No MSM, No TS, No Slave mode

	// Eg: HAL_TIM_PWM_ConfigChannel
	TIM2->CR2 = 0x0;   // defaults: CH1 connected to TI1, MM reset, CCx DMA on CCx event
	TIM2->CCMR1 = 0x0; // defaults: ETP non-inverted, EC mode 2 disabled, ET prescaler off, ET filter off
	TIM2->CCR1 = period / 2;
	TIM2->CCER = TIM_CCER_CC1E; // CC1 enabled

	// Eg: LL version of HAL_TIM_MspPostInit 
	LL_GPIO_InitTypeDef a0;
	a0.Pin = LL_GPIO_PIN_0;
	a0.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
	a0.Speed = LL_GPIO_SPEED_FREQ_LOW;
	a0.Mode = LL_GPIO_MODE_ALTERNATE;
	LL_GPIO_Init(GPIOA, &a0);

	// Eg: reg version of HAL_TIM_PWM_Start_IT(htim2,TIM_CHANNEL_1)
	//TIM2->DIER = TIM_DIER_UIE | TIM_DIER_CC1IE; 
	TIM2->DIER = TIM_DIER_CC1IE; // Note: interrupt on CC1 match
	

	// Eg: TIM_CCxChannelCmd(htim->Instance, Channel, TIM_CCx_ENABLE)
	TIM2->CCER = TIM_CCER_CC1E;  // Enable output channel 1
	TIM2->CR1 |= TIM_CR1_CEN;    // Enable the timer
    
}

