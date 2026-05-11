#include "GPIOTool.h"

#include <Arduino.h>

static const long LEVEL_ENUM[] = { 0, 1 };

GPIOTool::GPIOTool() {
    // digitalWrite
    digital_write.addProperty("pin", "GPIO pin number", Tool::TYPE_INTEGER, true);
    digital_write.addEnumProperty("level", "Logic level: 0=LOW, 1=HIGH", LEVEL_ENUM, 2, true);
    digital_write.onCall([](JsonObject args) -> String {
        int pin   = args["pin"].as<int>();
        int level = args["level"].as<int>();
        pinMode(pin, OUTPUT);
        digitalWrite(pin, level);
        return "OK";
    });

    // digitalRead
    digital_read.addProperty("pin", "GPIO pin number", Tool::TYPE_INTEGER, true);
    digital_read.onCall([](JsonObject args) -> String {
        int pin = args["pin"].as<int>();
        pinMode(pin, INPUT);
        return String(digitalRead(pin));
    });

    // PWM
    pwm_write.addProperty("pin", "GPIO pin number", Tool::TYPE_INTEGER, true);
    pwm_write.addProperty("duty", "Duty cycle 0-255", Tool::TYPE_INTEGER, true);
    pwm_write.addProperty("frequency", "PWM frequency in Hz", Tool::TYPE_INTEGER, true);
    pwm_write.onCall([](JsonObject args) -> String {
        int pin = args["pin"].as<int>();
        int duty = args["duty"].as<int>();
        int frequency = args["frequency"].as<int>();
        ledcAttach(pin, frequency, 8); // 8-bit resolution → duty 0-255
        ledcWrite(pin, duty);
        return "OK";
    });

    // analogRead
    analog_read.addProperty("pin", "GPIO ADC pin number", Tool::TYPE_INTEGER, true);
    analog_read.onCall([](JsonObject args) -> String {
        int pin = args["pin"].as<int>();
        return String(analogRead(pin));
    });

    // pulseIn
    pulse_in.addProperty("pin", "GPIO pin number", Tool::TYPE_INTEGER, true);
    pulse_in.addEnumProperty("state", "Pulse level to measure: 0=LOW, 1=HIGH", LEVEL_ENUM, 2, true);
    pulse_in.addProperty("timeout_us", "Timeout in microseconds (max 4294967295)", Tool::TYPE_INTEGER, true);
    pulse_in.onCall([](JsonObject args) -> String {
        int pin = args["pin"].as<int>();
        int state = args["state"].as<int>();
        uint32_t timeout_us = args["timeout_us"].as<uint32_t>();
        pinMode(pin, INPUT);
        unsigned long duration = pulseIn(pin, state, timeout_us);
        if (duration == 0) {
            return "TIMEOUT";
        }
        return String(duration) + " us";
    });

    addTool(&digital_write);
    addTool(&digital_read);
    addTool(&pwm_write);
    addTool(&analog_read);
    addTool(&pulse_in);
}
