#include <stm32f1xx_ll_gpio.h>


/**
 * Fired at the sampling frequency 
 */
void TIM2_IRQHandler (void)
{
    LL_GPIO_TogglePin(GPIOC, LL_GPIO_PIN_13);
}
