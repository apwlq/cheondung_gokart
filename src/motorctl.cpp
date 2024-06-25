#include "Arduino.h"
#include "motorctl.h"

motorctl::motorctl(int dir_pin, int pwm_pin) {
    pinMode(dir_pin, OUTPUT);
    pinMode(pwm_pin, OUTPUT);
    _dir_pin = dir_pin;
    _pwm_pin = pwm_pin;
}

void motorctl::Move(int dir, int pwm)
{
    digitalWrite(_dir_pin, dir);
    analogWrite(_pwm_pin, pwm);
}