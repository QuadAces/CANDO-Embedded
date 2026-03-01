#ifndef _MOTOR_HPP_
#define _MOTOR_HPP_

#pragma once

#include <iostream>
#include <Arduino.h>
#include <Servo.h>

class Motor {
    public:
    int ENC_PIN;
    Servo servo_motor;
    Motor(int enc_pin=0);
    void move_to(float degree);
    void empty_can();
};

#endif // MOTOR