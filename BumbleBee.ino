//Pin summary:

//10: Xbee Receiver


#include <SoftwareSerial.h> //communication library
#include <Wire.h> // compass library

#define Addr 0x1E //compass sensor
#define Rx 10 // communication receiver

SoftwareSerial Xbee (Rx, Tx); //declare communication device
Servo servoLeft; // declare left and right servos
Servo servoRight;

SoftwareSerial mySerial = SoftwareSerial(255, TxPin);

int color1 = 1;
int color2 = 2;
int color3 = 3;
int color4 = 4;
int myColor = 0;


void setup() {
  Serial.begin(9600);
  servoLeft.attach(13); // Attach left and right signal
  servoRight.attach(12);
  Xbee.begin(9600); // initiate communication
  //compass sensor
  Wire.begin();
  Wire.beginTransmission(Addr); 
  Wire.write(byte(0x02));
  Wire.write(byte(0x00));
  Wire.endTransmission();
  delay(500);
  //set my color beacon
  //myColor = 
}

void loop() {
  //look for nearest
  
  //if defending, turn to the enemy
  
  //if attacking, look for nearest
}

