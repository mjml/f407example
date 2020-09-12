#include <stdint.h>
#include <math.h>
#include <list>

struct Chirp
{
	uint16_t freq1;
	uint16_t freq2;
	uint16_t intensity1;
	uint16_t intensity2;
	uint32_t duration; // microseconds
	
	Chirp (int f, int d) : freq1(f), freq2(f), duration(d) {}
	Chirp (int f1, int f2, int d) : freq1(f1), freq2(f2), duration(d) {}
	~Chirp () {}

	static Chirp Sec(int f, int s) { return { .freq1 = f, .freq2 = f, .d = s * 1000 * 1000}; }

};

void init_clocks();
void init_trig();
void init_pwm();


int main ()
{
	init_clocks();
	init_trig();
	init_pwm();

	while (1) {
		;
	}
}

/**
 * Set up clocks
 */ 
void init_clocks ()
{

}

uint16_t sina[256];
/**
 * Compute 256-point sine approximation
 */
void init_trig ()
{
	for (int i=0; i < 256; i++) {
		float f = sin(i / 256.0);
		sina[i] = (int)(f * INT16_MAX);
	}
}


/**
 * Set up TIM2 as a PWM device
 */
void init_pwm ()
{
	
}


/**
 * 
 */
void TIM2_IRQHandler ()
{
	// Compute current waveform contributions

	// Set PWM level
	
	
}
