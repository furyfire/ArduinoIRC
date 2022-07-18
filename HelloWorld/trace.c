#include <Arduino.h>
#include "avr_mcu_section.h"

AVR_MCU(F_CPU, "atmega328");
const struct avr_mmcu_vcd_trace_t _mytrace[]  _MMCU_ = {
	{ AVR_MCU_VCD_SYMBOL("PORTB"), .what = (void*)&PORTB, },
	{ AVR_MCU_VCD_SYMBOL("UDR0"), .what = (void*)&UDR0, }
};