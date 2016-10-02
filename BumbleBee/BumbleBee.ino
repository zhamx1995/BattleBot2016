//Pin summary:

//10: Xbee Receiver


#include <SoftwareSerial.h> //communication library
#include <Wire.h> // compass library
#include <AFMotor.h>

#define Addr 0x1E //compass sensor
#define Rx 10 // communication receiver
#define Tx 11 // communication transmitter

AF_DCMotor ml(2, MOTOR12_64KHZ); // motor left
AF_DCMotor mr(1, MOTOR12_64KHZ); // motor right

SoftwareSerial Xbee (Rx, Tx); //declare communication device

SoftwareSerial mySerial = SoftwareSerial(255, Tx);

int color1 = 1;
int color2 = 2;
int color3 = 3;
int color4 = 4;
int myColor = 0;

float myLoc[2] = {0,0}; // my current location
float myVec[2] = {0,0}; // my current direction
float meToEnemy[3][2] = {0,0,0,0,0,0}; // vector from me to enemey

float prevBotLoc[3][2] = {0,0,0,0,0,0}; // previous location of enemy bots
float curBotLoc[3][2] = {0,1,1,0,1,-1}; // current location of enemy bots
float botVec[3][2] = {0,0,0,0,0,0}; // velocity of enemy bots

float botDist[3] = {2,3,4}; // distance between me and enemy
float dangerR = 2.5;
int botsInDanger = 0;
int nearestBot = -1; // idx of nearest bot in curBotLoc
int nearerBot = -1; // idx of 2nd nearest bot in curBotLoc

void setup() {
  Serial.begin(9600);
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
/* update enemy moving data
*/
  // get location data from broadcast
  resetLocData();
  // update velocity
  for(int i=0;i<3;i++){ // ith bot
    for(int j=0;j<2;j++){ // j=0 for X, j=1 for Y
      botVec[i][j] = curBotLoc[i][j] - prevBotLoc[i][j];
      meToEnemy[i][j] = curBotLoc[i][j] - myLoc[j];
    }
  }
  
/* moving algorithm
  
*/

  // if near edge or face obstacle, backward
  ml.run(BACKWARD);
  mr.run(BACKWARD);
  
  // if 0 or 1 bot in danger circle, move to nearset one
  if(botsInDanger <= 1){
    moveToward(curBotLoc[nearestBot][0],curBotLoc[nearestBot][1]);
  }
  
  // if 2 in danger circle; 
  // if they are close (angle of enemy1,me,enemy2 less than pi/2), move towards
  // else, run in the opposite direction
  if(botsInDanger == 2){
    // cos(angle) = dotProduct(v1,v2)/(|v1|*|v2|)
    float v1x = meToEnemy[nearestBot][0];
    float v1y = meToEnemy[nearestBot][1];
    float v2x = meToEnemy[nearerBot][0];
    float v2y = meToEnemy[nearerBot][1];
    float cosVal = (v1x*v2x+v1y+v2y)/(sqrt(sq(v1x)+sq(v1y))*sqrt(sq(v2x)+sq(v2y)));
    // compare cosVal with cos(pi/2) = 0; positive -> less 90 degree, negative -> larger 90 degree
    if(cosVal >= 0){ // attack, move toward nearest bot
      moveToward(curBotLoc[nearestBot][0], curBotLoc[nearestBot][1]);
    } else { // run, move away from their midpoint
      float midPointX = v1x+(v2x-v1x)/2;
      float midPointY = v1y+(v2y-v1y)/2;
      float targetX = 2*myLoc[0] - midPointX;
      float targetY = 2*myLoc[1] - midPointY;
      moveToward(targetX, targetY);
    }
  }
  // if >=3 in danger circle
  // if I am in the circle, run
  // if I am out of the circle, move towards nearest
  else{
    
  }
}


/*
void processData(){
  // if one color is not found, put its location very far
  
  // update myVec
}
*/

void resetLocData(){
  for(int i=0;i<3;i++){
    for(int j=0;j<2;j++){
      prevBotLoc[i][j] = curBotLoc[i][j];
    }
  }
  // reset current my loc info
  myLoc[0] = 1;
  myLoc[1] = 1;
  // get current enemy loc info from broadcast
  curBotLoc[0][0] = 1;
  curBotLoc[0][1] = 1;
  curBotLoc[0][2] = 1;
  curBotLoc[1][0] = 1;
  curBotLoc[1][1] = 1;
  curBotLoc[1][2] = 1;
}

void findNearest(){
  // calculate distance
  botDist[0] = sqrt(sq(curBotLoc[0][0]-myLoc[0]) + sq(curBotLoc[0][1]-myLoc[1]));
  botDist[1] = sqrt(sq(curBotLoc[1][0]-myLoc[0]) + sq(curBotLoc[1][1]-myLoc[1]));
  botDist[2] = sqrt(sq(curBotLoc[2][0]-myLoc[0]) + sq(curBotLoc[2][1]-myLoc[1]));
  // find nearest, update danger info
  botsInDanger = 0;
  float minDist = 10000; // set to be radius of battle field
  float secondMin = 9000;
  for(int i=0; i<3; i++){
    if (botDist[i] < minDist) {
      minDist = botDist[i];
      secondMin = minDist;
      nearerBot = nearestBot;
      nearestBot = i;
    }
    else if (botDist[i] < secondMin) {
      secondMin = botDist[i];
      nearerBot = i;
    }
    if (botDist[i] < dangerR) botsInDanger = botsInDanger + 1;
  }
}

// myBot moves from myLoc(X,Y) to (targetX, targetY), while currently facing myVec(Vx,Vy)
void moveToward(float targetX, float targetY){
  
}
