#include "motor.h"

Motor::Motor(int enc_pin) {
    this->ENC_PIN = enc_pin;
    this->servo_motor.attach(this->ENC_PIN);
}

// degree: 0 to 180
void Motor::move_to(float degree) {
    this->servo_motor.write(degree);
}

void Motor::empty_can() {
    this->move_to(180);
    delay(1000);
    this->move_to(0);
}