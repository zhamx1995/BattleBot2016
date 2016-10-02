
#include <AFMotor.h>
#include "ChinaBee.h"
#include <SPI.h>
//computer vision
ChinaBee bee;

//Pin summary:
//NOTE FOR THE MOTORS: LEFT IS RIGHT, FORWARD IS BACKWARD
AF_DCMotor ml(2, MOTOR12_64KHZ); // motor left
AF_DCMotor mr(1, MOTOR12_64KHZ); // motor right
// REMEMBER TO CHANGE BEFORE BATTLE
const int myColor = 2;

// Battle round and obstacle settings
const int ROUND = 1; // frist round, semi, and finals


float myLoc[2] = {0,0}; // my current location
float myVec[2] = {0,0}; // my current direction
float meToEnemy[3][2] = {0,0,0,0,0,0}; // vector from me to enemey

float prevBotLoc[3][2] = {0,0,0,0,0,0}; // previous location of enemy bots
float curBotLoc[3][2] = {0,1,1,0,1,-1}; // current location of enemy bots
float botVec[3][2] = {0,0,0,0,0,0}; // velocity of enemy bots

float botDist[3] = {2,3,4}; // distance between me and enemy
float dangerR = 50;
int botsInDanger = 0;
int nearestBot = -1; // idx of nearest bot in curBotLoc
int nearerBot = -1; // idx of 2nd nearest bot in curBotLoc

/*
protection on the edge
*/
// avoid from dropping off battlefield
const float X_MIN = 80;
const float X_MAX = 260;
const float X_MID = 170;
const float Y_MIN = 10;
const float Y_MAX = 200;
const float Y_MID = 105;
int avoidEdge = 0; // 0->avoid xmin; 1->avoid xmax; 2->avoid ymin; 3->avoid ymax
const int SPEED_LOW = 100;
const int SPEED_MID = 190;
const int SPEED_HIGH = 220;
int SPEED_CUR = 0;
//IR parameters
const int IRcount = 4;
int IRpin[] = {38,39,40,41}; //frontleft,frontright,rearright,rearleft
int IRPowerPin[] = {24,25,26,27};
int IRGroundPin[] = {44,45,46,47};
boolean IRActive[] = {false,false,false,false};

void setup() {
  Serial.begin(115200);
  Serial.print("Starting...\n");
  bee.init(48,49);
  //setting up IR sensors
  for( int i = 0; i < IRcount; i++)
  {
   pinMode(IRpin[i],INPUT);
    pinMode(IRPowerPin[i],OUTPUT);
   pinMode(IRGroundPin[i],OUTPUT);
  digitalWrite(IRPowerPin[i],HIGH);
   digitalWrite(IRGroundPin[i],LOW); 
  }
}

void loop() {
  //use different delay times for different operation 
  //for optimal performance
  //Edge detection by IR sensors
  if(IRonEdge())
  {
    Serial.println("One or more of the IR sensors have detected danger");
    if(!IRActive[0] && !IRActive[1] && (IRActive[2] || IRActive[3]))
    {
      moveForward();
      delay(150);
    }
    else if(!IRActive[0] && IRActive[1])
    {
      moveBackward();
      delay(100);
      turnLeft();
      delay(100);
    }
    else if(IRActive[0] && !IRActive[1])
    {
      moveBackward();
      delay(100); 
     turnRight();
     delay(100);
    }
    else if(IRActive[0] && IRActive[1] && !IRActive[2])
    {
      moveBackward();
      delay(150); 
     turnRight();
     delay(100);
    }
    else
    {
      moveBackward();
      delay(150);
     turnLeft();
     delay(100);
    }
  }
  else
  {
    /* update enemy moving data */
    // get location data from broadcast
    processData();
    // update velocity
    for(int i=0;i<3;i++){ // ith bot
      for(int j=0;j<2;j++){ // j=0 for X, j=1 for Y
        botVec[i][j] = curBotLoc[i][j] - prevBotLoc[i][j];
        meToEnemy[i][j] = curBotLoc[i][j] - myLoc[j];
      }
    }
    float distanceToEdge = distToEdge();
    float distanceToObstacle = distToObstacle();
    boolean isNearBoundary = nearBoundary(distanceToEdge, distanceToObstacle);
    if (isNearBoundary) {// if front faces edge or obstacle, backward
      switch(avoidEdge){
        case 0:
          moveToward(X_MID, myLoc[1]);
          break;
        case 1:
          moveToward(X_MID, myLoc[1]);
          break;
        case 2:
          moveToward(Y_MID, myLoc[0]);
          break;
        case 3:
          moveToward(Y_MID, myLoc[0]);
        default:
          // moves to center
          moveToward(X_MID, Y_MID);
          break;
      }
    }
    
    findNearest();
    float v1x = meToEnemy[nearestBot][0];
    float v1y = meToEnemy[nearestBot][1];
    float v2x = meToEnemy[nearerBot][0];
    float v2y = meToEnemy[nearerBot][1];
    // if 0 or 1 bot in danger circle, move to nearset one
    if(botsInDanger <= 1){
      moveToward(curBotLoc[nearestBot][0],curBotLoc[nearestBot][1]);
    }
    // if 2 in danger circle; 
    // if they are close (angle of enemy1,me,enemy2 less than pi/2), move towards
    // else, run in the opposite direction
    else if(botsInDanger == 2){
      // cos(angle) = dotProduct(v1,v2)/(|v1|*|v2|)
      float cosVal = (v1x*v2x+v1y*v2y)/(sqrt(sq(v1x)+sq(v1y))*sqrt(sq(v2x)+sq(v2y)));
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
      float Px = myLoc[0];
      float Py = myLoc[1];
      float Ax = curBotLoc[0][0];
      float Ay = curBotLoc[0][1];
      float Bx = curBotLoc[1][0];
      float By = curBotLoc[1][1];
      float Cx = curBotLoc[2][0];
      float Cy = curBotLoc[2][1];
      float M = ((Py-Cy)*(Cx-Bx)+(Cy-By)*(Cx-Px))/((By-Ay)*(Cx-Bx)-(Cy-By)*(Px-Ax));
      
      if (M >= 0) { // P is out of Triangle_ABC, attack nearest
        moveToward(curBotLoc[nearestBot][0], curBotLoc[nearestBot][1]);      
      } else { // P is in Triangle_ABC, ignore farthest, and escape from the other two
        float midPointX = v1x+(v2x-v1x)/2;
        float midPointY = v1y+(v2y-v1y)/2;
        float targetX = 2*myLoc[0] - midPointX;
        float targetY = 2*myLoc[1] - midPointY;
        moveToward(targetX, targetY);
      }
    }
    delay(150);
  }
  
}



void processData(){
  // if one color is not found, put its location very far
//  Serial.print("Entered processData, number of teams is ");
//  Serial.println(bee.get_num_teams());
//  bee.update();
    int count = 0;
  
    for (int i=0; i<bee.get_num_teams(); i++) {
    team_status_t* stat = bee.get_status(i);
    if (stat->haveFound || true) {
      if(i == myColor){
        // update myVec
        myVec[0] = stat->x - myLoc[0];
        myVec[1] = stat->y - myLoc[1];
        // update myLoc
        myLoc[0] = stat->x;
        myLoc[1] = stat->y;
      }
      else
      {
        // update enemy loc
       if (stat->x == 0){
         curBotLoc[count][0] = 1000;
         curBotLoc[count][1] = 1000;
       } else {
         curBotLoc[count][0] = stat->x;
         curBotLoc[count][1] = stat->y;
       }
       count++;
      }
      Serial.print("Team ");
      Serial.print(i);
      Serial.print(" ");
      Serial.print(stat->x);
      Serial.print(" ");
      Serial.print(stat->y);
      Serial.print(" time since (ms): ");
      Serial.println(millis() - stat->timestamp);
    }
  }
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
     secondMin = minDist;
    minDist = botDist[i];
      nearerBot = nearestBot;
      nearestBot = i;
   Serial.print("Distance for bot ");
   Serial.print(i);
   Serial.print(" is ");
  Serial.println(botDist[i]);
    }
    else if (botDist[i] < secondMin) {
      secondMin = botDist[i];
      nearerBot = i;
    }
    if (botDist[i] < dangerR) botsInDanger = botsInDanger + 1;
  }
  Serial.print("Nearest is ");
  Serial.print(nearestBot);
}

float distToEdge(){
  float toXMin = myLoc[0] - X_MIN;
  float toXMax = X_MAX - myLoc[0];
  float toYMin = myLoc[1] - Y_MIN;
  float toYMax = Y_MAX - myLoc[1];
  float result = toXMin;
  avoidEdge = 0;
  if (toXMax < result) {
    result = toXMax;
    avoidEdge = 1;
  }
  if (toYMin < result) {
    result = toYMin;
    avoidEdge = 2;
  }
  if (toYMax < result) {
    result = toYMax;
    avoidEdge = 3;
  }
  return result;
}

float distToObstacle(){
  if (ROUND == 1) return 150.0;
  else if (ROUND == 2) {
    
  }
}

boolean nearBoundary(float dToE, float dToO){ // to edge; to obstacle
  if (myLoc[0] < 0.5 && myLoc[1] < 0.5)
 { 
   Serial.println("broadcast down, moving cautiously");
   SPEED_CUR = SPEED_LOW;
 }
  else if (dToE >= 60 && dToO >= 30) 
  {
    Serial.println("Center of field, moving full speed");
    SPEED_CUR = SPEED_HIGH;
  }
  else if (dToE >= 16 && dToO >= 16) 
  {
    SPEED_CUR = SPEED_MID;
    Serial.println("between center and edge, moving moderately");
  }
  else {
    Serial.println("near edge, moving cautiously");
    SPEED_CUR = SPEED_LOW;
    return true;
  }
  return false;
}

// myBot moves from myLoc(X,Y) to (targetX, targetY), while currently facing myVec(Vx,Vy)
void moveToward(float targetX, float targetY){
  float targetVec[2] = {targetX-myLoc[0], targetY-myLoc[1]};
  float v1x = myVec[0];
  float v1y = myVec[1];
  float v2x = targetVec[0];
  float v2y = targetVec[1];
  float cosVal = (v1x*v2x+v1y*v2y)/(sqrt(sq(v1x)+sq(v1y))*sqrt(sq(v2x)+sq(v2y)));
  if (cosVal >= 0.98) { // theta <= 10 degree, move straight forward
    moveForward();
  } else if (cosVal < 0.98 && cosVal >= -0.5) { // turn right or left between 10-120 degree
    // sin(theta) = cross product (v1, v2)/(|v1| * |v2|), cross(v1,v2) = v1x*v2y-v1y*v2x
    float sinVal = (v1x*v2y - v1y*v2x)/(sqrt(sq(v1x)+sq(v1y))*sqrt(sq(v2x)+sq(v2y)));
    if (sinVal > 0) { // turn left
      turnLeft();
    } else if (sinVal < 0) { // turn right
      turnRight();
    }
  } else { // theta >= 120 degree
    if(v1x == 0 &&v1y == 0)
    {
     //didn't get own direction, move forwared regardlessly
     moveForward();
    }
    else{
      //turn around
      turnAround();
    }
  }
  
}

boolean IRonEdge()
{
  boolean result = false;
   for(int i = 0; i < IRcount; i++)
  {
   if(digitalRead(IRpin[i])==LOW)
     {
       IRActive[i] = true;
       result = true;
     }
     else
     {
      IRActive[i] = false; 
     }
  }
 return result; 
}

void moveForward(){
  Serial.println("Moving forward");
    ml.setSpeed(SPEED_CUR);
    mr.setSpeed(SPEED_CUR);
    ml.run(BACKWARD);
    mr.run(BACKWARD);
}
void turnLeft(){
  Serial.println("Turning left");
  ml.setSpeed(255);
  mr.setSpeed(255);
  ml.run(BACKWARD);
  mr.run(FORWARD);
}
void turnRight(){
  Serial.println("Turning right");
      ml.setSpeed(255);
      mr.setSpeed(255);
      ml.run(FORWARD);
      mr.run(BACKWARD);
}
void turnAround(){
    Serial.print("turning around");
    turnRight();
}
void moveBackward(){
    Serial.println("Moving backward");
    ml.setSpeed(255);
    mr.setSpeed(255);
    ml.run(FORWARD);
    mr.run(FORWARD);
}
