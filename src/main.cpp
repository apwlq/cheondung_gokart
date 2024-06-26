//------------------------------------------------------//
// Copyright (C) 2024 JuwonBang
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//------------------------------------------------------//

#define FW_NAME "Cheondung" // 펌웨어 이름
#define VERSION "1.0.0-mtr"  // 펌웨어 버전

#define SOURCE_CODE "https://github.com/apwlq/cheondungFW"

// 라이브러리 불러오기
#include <Arduino.h>
#include <mcp_can.h>
#include <SPI.h>
#include <EEPROM.h>
#include <SdFat.h>
#include <DFRobotDFPlayerMini.h>
#include <Adafruit_NeoPixel.h>
#include <RTClib.h>

// 커스텀 라이브러리 불러오기
#include "motorctl.h"
#include "button.h"

// 이 정의어를 변경하지 마세요
#define ON 1
#define OFF 0

#define S_OFF 0
#define S_ACC 1
#define S_ON 2
#define S_RUN 3


// 아두이노 메가 핀 맵핑
#define MTR1_OUP_PWM_PIN 9
#define MTR1_OUP_DIR_PIN 10
#define MTR2_OUP_PWM_PIN 11
#define MTR2_OUP_DIR_PIN 12

#define ACL_INP_PIN A1
#define ACL_INP_MIN 182
#define ACL_INP_MAX 875

#define BRK_INP_PIN A2
#define BRK_INP_MIN 182
#define BRK_INP_MAX 875

#define BRK_ENA_POW 200


// 변수 정의
#define BAT1_INP_PIN A3
const float BAT1_INP_MIN = 2.5f;
const float BAT1_INP_LOW = 9.0f;
const float BAT1_INP_MAX = 12.6f;

#define BAT2_INP_PIN A4
const float BAT2_INP_MIN = 2.5f;
const float BAT2_INP_LOW = 9.0f;
const float BAT2_INP_MAX = 12.6f;

int mtr_dir = 0;
int mtr1_val = 0; // L
int mtr2_val = 0; // R

int acl_val = 0;
int brk_val = 0;

int whl_val = 0;
int whl_mtr_val = 0;

int acl_sud_err = 0;

int toggle_sidong = 0;


// 함수 사용 준비
motorctl RightMotor(MTR1_OUP_DIR_PIN, MTR1_OUP_PWM_PIN);
motorctl LeftMotor(MTR2_OUP_DIR_PIN, MTR2_OUP_PWM_PIN);

button engine_btn(9);

MCP_CAN CAN0(10);

// 디버그 모드
#define DEBUG_PIN 3
int is_debug = OFF;

void debug(const char *text, bool ln = true);
void emergency(int type = 0);
void(* resetFunc) (void) = 0; //declare reset function @ address 0


void setup() {
    // 디버그 모드 확인 만약 디버그 모드라면, 모든 활동을 시리얼로 출력함.
    pinMode(DEBUG_PIN, INPUT_PULLUP);
    is_debug = digitalRead(DEBUG_PIN);

    // 시리얼 통신 시작
    Serial.begin(9600);

    // 펌웨어 출력
    Serial.print("\nStarting ");
    Serial.print(FW_NAME);
    Serial.print(" v");
    Serial.print(VERSION);
    Serial.print(" (");
    Serial.print(SOURCE_CODE);
    Serial.print(")");
    Serial.println("");

    // 디바이스 아이디 불러온 후 출력
    Serial.print("Device ID: ");
    for (int i = 0; i < 16; i++) {
        byte serial_key = EEPROM.read(i);
        Serial.print(serial_key);
    }
    Serial.print("\r\n");

    // 디버그 핀 활성화 유무 출력
    Serial.print("debug mode: ");
    Serial.println(is_debug == 1 ? "true" : "false");
    Serial.print("\r\n");

    // 시동 전 연결 확인

    // 디버그 모드 활성화 시 CAN 통신 활성화 유무 출력
    if(CAN0.begin(CAN_500KBPS) == CAN_OK) {
        debug("Can init ok!!");
    } else {
        debug("Can init fail!!");
    }

    // 시동 준비 완료
    debug("Standby Mode");

    while(toggle_sidong == 0) {
        int result = engine_btn.getButton();
        brk_val = map(analogRead(BRK_INP_PIN), BRK_INP_MIN, BRK_INP_MAX, 0, 1024) / 4;
        brk_val = constrain(brk_val, 0, 255);
        if (result == 4 || result == 3) {
            emergency(0);
        } else if (result == 1 && brk_val > BRK_ENA_POW) {
            debug("Go-kart START/RUN Mode!!");
            toggle_sidong = 1;
        } else if (result == 1) {
            debug("Go-kart ACC Mode!!");
            toggle_sidong = 2;
        }
    }
    Serial.println("");

}

void loop() {
    acl_val = map(analogRead(ACL_INP_PIN), ACL_INP_MIN, ACL_INP_MAX, 0, 1024) / 4;
    acl_val = constrain(acl_val, 0, 255);

    brk_val = map(analogRead(BRK_INP_PIN), BRK_INP_MIN, BRK_INP_MAX, 0, 1024) / 4;
    brk_val = constrain(brk_val, 0, 255);

    int is_engine_btn = engine_btn.getButton();
    if (is_engine_btn == 4 || is_engine_btn == 3) {
        emergency(0);
    }
        switch (toggle_sidong) {
        case 1:
            if (is_engine_btn == 1 && brk_val > BRK_ENA_POW) {
                emergency(0);
            } else if (is_engine_btn == 1) {
                debug("You have to step on the brakes to do that.");
            }
            break;
        case 2:
            if (is_engine_btn == 1 && brk_val > BRK_ENA_POW) {
                debug("Go-kart START/RUN Mode!!");
                toggle_sidong = 1;
            } else if (is_engine_btn == 1) {
                debug("Go-kart ON Mode!!");
                toggle_sidong = 3;
            }
            break;
        case 3:
            if (is_engine_btn == 1 && brk_val > BRK_ENA_POW) {
                debug("Go-kart START/RUN Mode!!");
                toggle_sidong = 1;
            } else if (is_engine_btn == 1) {
                emergency(0);
            }
            break;
    }
//#if (WHEEL == ON)
//    mtr1_val = acl_val - brk_val - whl_mtr_val;
//    mtr2_val = acl_val - brk_val + whl_mtr_val;
//#elif (WHEEL == OFF)
//    mtr1_val = acl_val - brk_val;
//  mtr2_val = acl_val - brk_val;
//#endif
//
//    LeftMotor.Move(mtr_dir, mtr1_val);
//    RightMotor.Move(mtr_dir, mtr2_val);
//
//    if (Serial.read() == 'c') {
//        if(mtr_dir == 1) {
//            mtr_dir = 0;
//        } else {
//            mtr_dir = 1;
//        }
//    }
}

void emergency(int type) {
    switch (type) {
        case 1:
            LeftMotor.Move(0, 0);
            RightMotor.Move(0, 0);
            break;
        default:
            debug("Go-kart Shutdown!!");
            delay(1000);
            resetFunc();
            break;
    }
}

void debug(const char *text, bool ln) {
    if (is_debug == ON) {
        Serial.print("[DEBUG] ");
        if (ln == true) {
            Serial.println(text);
        } else {
            Serial.print(text);
        }
    }
}