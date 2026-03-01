#ifndef _ULTRASONIC_H_
#define _ULTRASONIC_H_

#pragma once

#include <iostream>
#include <Arduino.h>

class Ultrasonic {
    public:
    int TRIGGER_PIN;
    int ECHO_PIN;
    Ultrasonic(int trigger_pin, int echo_pin);
    int get_distance();
};

#endif