#include "variant.h"
#include "variant_delay.h"
#include "cmsis.h"
#include "delay.h"
#include <hal_gpt.h>


// Get current timestamp, unit: ms
// tick ==> ms
uint32_t millis(void)
{
	return variant_millis();
}

// Interrupt-compatible version of micros
// Theory: repeatedly take readings of SysTick counter, millis counter and SysTick interrupt pending flag.
// When it appears that millis counter and pending is stable and SysTick hasn't rolled over, use these
// values to calculate micros. If there is a pending SysTick, add one to the millis counter in the calculation.
uint32_t micros( void )
{
	uint32_t ticks, ticks2;
	uint32_t pend, pend2;
	uint32_t count, count2;

	ticks2  = SysTick->VAL;
	pend2   = !!((SCB->ICSR & SCB_ICSR_PENDSTSET_Msk)||((SCB->SHCSR & SCB_SHCSR_SYSTICKACT_Msk)))  ;
	count2  = millis(); // TimingMillis;

	do {
		ticks  = ticks2;
		pend   = pend2;
		count  = count2;
		ticks2 = SysTick->VAL;
		pend2  = !!((SCB->ICSR & SCB_ICSR_PENDSTSET_Msk)||((SCB->SHCSR & SCB_SHCSR_SYSTICKACT_Msk)))  ;
		count2 = millis(); // TimingMillis;
	} while ((pend != pend2) || (count != count2) || (ticks < ticks2));

	return ((count+pend) * 1000) + (((SysTick->LOAD  - ticks)*(1048576/(F_CPU/1000000)))>>20) ;  // F_CPU
	// this is an optimization to turn a runtime division into two compile-time divisions and
	// a runtime multiplication and shift, saving a few cycles
}

void delayMicroseconds(unsigned int us)
{
	hal_gpt_delay_us(us);
}

void yield(void) {

}

void delay( uint32_t ms )
{
	variant_delay(ms);
}
