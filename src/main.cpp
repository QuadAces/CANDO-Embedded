#include <Arduino.h>
#include "pins.h"
#include "ultrasonic.h"
#include "motor.h"

// put function declarations here:
Ultrasonic ult = Ultrasonic(ULT_TRIG, ULT_ECHO);
Motor motor = Motor(MOTOR_ENC);

void setup() {
  // Ultrasonic
  pinMode(ULT_TRIG, OUTPUT);
  pinMode(ULT_ECHO, INPUT);

  // Motor
  pinMode(MOTOR_ENC, OUTPUT);
  motor.move_to(0);

  Serial.begin(115200);
}

/* -------------------------------------------------------------------------- */
/*                                    LOOP                                    */
/* -------------------------------------------------------------------------- */

void loop() {
  // put your main code here, to run repeatedly:
  float dist = ult.get_distance();
  // Serial.printf("%f\n", dist);
  if (dist <= 25) {
    Serial.printf("\nCAN PRESENT: %f", dist);
    motor.empty_can();
  }
  
  delay(100);
}

// put function definitions here: