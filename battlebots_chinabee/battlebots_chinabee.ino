#include "ChinaBee.h"
#include <SPI.h>  

// Computer vision
ChinaBee bee;

void setup() {
  Serial.begin(115200);
  Serial.print("Starting...\n");
  bee.init(48, 49);
}

void loop() {

  // This must be called every loop
  bee.update();
  
  for (int i=0; i<bee.get_num_teams(); i++) {
    team_status_t* stat = bee.get_status(i);
    if (stat->haveFound || true) {
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


