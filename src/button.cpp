#include "button.h"

button::button(int btn_pin) {
    _button_pin = btn_pin;
    pinMode(_button_pin, INPUT_PULLUP);
}

int button::getButton() {
    int reading = digitalRead(_button_pin);

    // 버튼 상태가 마지막 상태와 다를 경우, 반동(debounce)을 처리
    if (reading != last_btn_state) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        // 버튼을 누른 상태가 안정적인 경우
        if (reading != _btn_state) {
            _btn_state = reading;

            // 버튼이 눌렸다면 (HIGH에서 LOW로 변경)
            if (_btn_state == LOW) {
                pressCount++;
                lastPressTime = millis(); // 마지막으로 눌린 시간 갱신
                if (!isPressing) {
                    isPressing = true;
                    pressStartTime = millis(); // 꾹 누르기 시작 시간 기록
                }
            } else {
                isPressing = false;
            }
        }
    }

    // 꾹 누르기 감지
    if (isPressing && (millis() - pressStartTime > longPressTime)) {
        isPressing = false;
        pressCount = 0;
        return 4; // 꾹 누르기 감지
    }

    // 마지막으로 버튼이 눌린 시간으로부터 얼마가 지났는지 확인
    if ((millis() - lastPressTime > pressResetTime) && pressCount > 0) {
        // 버튼이 한 번 눌렸다면 원하는 동작 수행
        if (pressCount == 1 && !isPressing) { // 버튼 누르기가 안정적으로 감지되고, 꾹 누르기가 아닐 때
            pressCount = 0;
            return 1;
        } else if (pressCount == 2 && !isPressing) {
            pressCount = 0;
            return 2;
        } else if (pressCount == 3 && !isPressing) {
            pressCount = 0;
            return 3;
        }

        pressCount = 0;
    }

    // 마지막 버튼 상태 저장
    last_btn_state = reading;
    return 0;
}
