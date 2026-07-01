#include "MyServo.h"

void MyServo::write(int degrees)
{
    if (!attached) return;

    int pulseWidth = map(degrees, 0, 180, min_Pulse, max_pulse);

    digitalWrite(pin, HIGH);
    delayMicroseconds(pulseWidth);
    digitalWrite(pin, LOW);
    delayMicroseconds(20000 - pulseWidth);
}

void MyServo::attach(int pin_input)
{
    if (attached) return;

    pin = pin_input;
    pinMode(pin, OUTPUT);
    attached = true;
}

void MyServo::detach()
{
    if (!attached) return;

    pinMode(pin, INPUT);
    attached = false;
}