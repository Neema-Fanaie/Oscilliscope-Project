#pragma once

#include <Arduino.h>

class MyServo
{
private:
    int pin;
    bool attached = false;
    int min_Pulse = 1000;
    int max_pulse = 2000;

public:
    void write(int degrees);
    void attach(int pin_input);
    void detach();
};