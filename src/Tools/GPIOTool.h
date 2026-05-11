#ifndef __GPIO_TOOL_H__
#define __GPIO_TOOL_H__

#include "ToolSet.h"

class GPIOTool : public ToolSet {
    Tool digital_write{"gpio_digital_write", "Set a GPIO pin to LOW (0) or HIGH (1)"};
    Tool digital_read{"gpio_digital_read", "Read the digital state of a GPIO pin, returns 0 or 1"};
    Tool pwm_write{"gpio_pwm_write", "Set a GPIO pin PWM duty cycle and frequency"};
    Tool analog_read{"gpio_analog_read", "Read the ADC value of a GPIO pin, returns 0-4095"};
    Tool pulse_in{"gpio_pulse_in", "Measure the duration of a pulse on a GPIO pin in microseconds"};

    public:
        GPIOTool();
};

#endif
