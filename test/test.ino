#include <AFMotor.h>

AF_DCMotor motor2(2, MOTOR12_64KHZ); // create motor #2, 64KHz pwm
AF_DCMotor motor1(1, MOTOR12_64KHZ);

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Motor test!");
  motor1.setSpeed(250);     // set the speed to 200/255
  motor2.setSpeed(150);
  Serial.print("tick");
  
  motor1.run(FORWARD);      // turn it on going forward
  motor2.run(FORWARD);      // turn it on going forward
  delay(143000); // broadcast period of 1/7 second
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  
}

void loop() {
//  Serial.print("tick");
  
//  motor1.run(FORWARD);      // turn it on going forward
//  motor2.run(FORWARD);      // turn it on going forward
//  delay(1000);

  //Serial.print("tock");
  //motor1.run(BACKWARD);     // the other way
  //motor2.run(BACKWARD);     // the other way
  //delay(1000);
  //
  //Serial.print("tack");
  //motor1.run(RELEASE);      // stopped
//  motor2.run(RELEASE);      // stopped
//  delay(1000);
}
