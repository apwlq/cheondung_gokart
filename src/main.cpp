//------------------------------------------------------//
// 부산동고등학교 파키오 고카트 전용 펌웨어
// 천둥(cheondung)
// style: K&R, Snake Case
#define VERSION 1.0
//------------------------------------------------------//

#include <Arduino.h>
#include <mcp_can.h>
#include <SPI.h>
#include <EEPROM.h>

#include "motorctl.h"



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


//------------------------------------------------------//
// Do Not Change This define
//------------------------------------------------------//
#define ON 1
#define OFF 0

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

#define BAT1_INP_PIN A3
const float BAT1_INP_MIN = 2.5f;
const float BAT1_INP_LOW = 9.0f;
const float BAT1_INP_MAX = 12.6f;

#define BAT2_INP_PIN A4
const float BAT2_INP_MIN = 2.5f;
const float BAT2_INP_LOW = 9.0f;
const float BAT2_INP_MAX = 12.6f;

int is_debug = OFF;
#define WHEEL ON
#define DEBUG_PIN 3


int mtr_dir = 0;
int mtr1_val = 0; // L
int mtr2_val = 0; // R

int acl_val = 0;
int brk_val = 0;

int whl_val = 0;
int whl_mtr_val = 0;

int acl_sud_err = 0;


motorctl RightMotor(MTR1_OUP_DIR_PIN, MTR1_OUP_PWM_PIN);
motorctl LeftMotor(MTR2_OUP_DIR_PIN, MTR2_OUP_PWM_PIN);

MCP_CAN CAN0(10);


void setup() {
    pinMode(DEBUG_PIN, INPUT_PULLUP);
    is_debug = digitalRead(DEBUG_PIN);

    Serial.begin(9600);
    Serial.println("------------------------------------------------------");
    Serial.println("");
    Serial.println("부산동고등학교 파키오 고카트 운영체제 천둥(Cheondung)");
    Serial.println("");
    Serial.print("version: ");
    Serial.println(VERSION);

    Serial.print("serial_key: ");
    for (int i = 0; i < 16; i++) {
        byte serial_key = EEPROM.read(i);
        Serial.print(serial_key);
    }
    Serial.println("");

    Serial.print("Debug: ");
    Serial.println(is_debug);
    Serial.println("------------------------------------------------------");
    Serial.print("starting");
    while(true) {
        #if (DEBUG == ON)
        #else
        if(CAN0.begin(CAN_500KBPS) == CAN_OK) Serial.print("Can init ok!!\r\n"); // 500Kbs의 속도로 CAN버스를 초기화 합니다.
        else Serial.print("Can init fail!!\r\n");
        #endif

        delay(300);
        Serial.print(".");
    }
    Serial.println("");

}

void loop() {
    acl_val = map(analogRead(ACL_INP_PIN), ACL_INP_MIN, ACL_INP_MAX, 0, 1024) / 4;
    acl_val = constrain(acl_val, 0, 255);

    brk_val = map(analogRead(BRK_INP_PIN), BRK_INP_MIN, BRK_INP_MAX, 0, 1024) / 4;
    brk_val = constrain(brk_val, 0, 255);

#if (WHEEL == ON)
    mtr1_val = acl_val - brk_val - whl_mtr_val;
    mtr2_val = acl_val - brk_val + whl_mtr_val;
#elif (WHEEL == OFF)
    mtr1_val = acl_val - brk_val;
  mtr2_val = acl_val - brk_val;
#endif

// 급격한 변화 방지
    if (acl_val) {
        acl_val = 0;
        acl_sud_err = 1;
    } else {
        acl_sud_err = 0;
    }

    LeftMotor.Move(mtr_dir, mtr1_val);
    RightMotor.Move(mtr_dir, mtr2_val);

    if (Serial.read() == 'c') {
        if(mtr_dir == 1) {
            mtr_dir = 0;
        } else {
            mtr_dir = 1;
        }
    }
}
