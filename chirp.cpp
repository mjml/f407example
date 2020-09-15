#define _CHIRP_CPP
#include "stm32f1xx_hal.h"

#include "chirp.h"

uint16_t Chirp::sina[256];

void Chirp::Initialize ()
{

}

void Chirp::Finalize ()
{

}

void Chirp::Service ()
{
    auto tick = HAL_GetTick();
    
    
}