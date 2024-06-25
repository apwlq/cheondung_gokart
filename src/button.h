#ifndef BUTTON_H
#define BUTTON_H

#include "Arduino.h"

class button {
public:
    button(int btn_pin);
    int getButton();

private:
    int _button_pin;
    int _btn_state = HIGH; // 초기값은 HIGH로 설정, 버튼이 눌리지 않은 상태
    int last_btn_state = HIGH; // 초기값은 HIGH로 설정
    int pressCount = 0;
    bool isPressing = false;

    unsigned long lastDebounceTime = 0;
    unsigned long lastPressTime = 0;
    unsigned long pressStartTime = 0;

    const unsigned long debounceDelay = 50;
    const unsigned long pressResetTime = 500;
    const unsigned long longPressTime = 3000;
};

#endif
