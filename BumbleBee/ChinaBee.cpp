#include "ChinaBee.h"
#include <stdint.h>
#include <SPI.h>
#include "Mirf.h"
#include "nRF24L01.h"
#include "MirfHardwareSpiDriver.h"

void ChinaBee::init(int cePin, int csnPin) {

  // Init mirf
  Mirf.spi = &MirfHardwareSpi;
  Mirf.cePin = cePin;
  Mirf.csnPin = csnPin;
  Mirf.init();
  Mirf.setRADDR((byte *)"serv1");
  Mirf.payload = sizeof(cv_data_t);
  Mirf.config();
  
  for (int i = 0; i < get_num_teams(); i++) {
    this->statuses[i].x = 0;
    this->statuses[i].y = 0;
    this->statuses[i].haveFound = false;
    this->statuses[i].timestamp = 0L;
  }
}

int ChinaBee::get_num_teams() {
  return CHINA_BEE_NUM_TEAMS;
}

void ChinaBee::update() {
  
  while (Mirf.dataReady()) {

    // Read into temp location, check checksum
    Mirf.getData((byte*)&this->tempCvData);
    if (!test_checksum(&this->tempCvData)) {
      Serial.print("Bad Checksum: ");
      Serial.println(this->tempCvData.checksum, HEX);
      break;
    }

    // Check team number
    if (this->tempCvData.teamNum >= get_num_teams()) {
      Serial.print("Unexpected team num: ");
      Serial.println(this->tempCvData.teamNum, DEC);
      break;
    }

    // Store in array
    int teamNum = this->tempCvData.teamNum;
    this->statuses[teamNum].x = this->tempCvData.x;
    this->statuses[teamNum].y = this->tempCvData.y;
    this->statuses[teamNum].haveFound = true;
    this->statuses[teamNum].timestamp = millis();
  }
}

bool ChinaBee::send(uint8_t teamNum, uint32_t x, uint32_t y) {
  Mirf.setTADDR((byte *)"serv1");
  
  cv_data_t data;
  data.x = x;
  data.y = y;
  data.teamNum = teamNum;
  data.checksum = generate_checksum(&data);
  
  Mirf.send((byte*)&data);
  while(Mirf.isSending()){
  }
}

team_status_t* ChinaBee::get_status(int teamNum) {
  if (teamNum >= get_num_teams() || teamNum < 0) {
    Serial.print("Requested invalid team number: ");
    Serial.println(teamNum);
    return 0;
  }
  return &this->statuses[teamNum];
}

uint8_t ChinaBee::generate_checksum(cv_data_t* cvData) {
  const int numChecksummedBytes = 9;
  
  uint8_t *ptr = (uint8_t*)cvData;
  uint8_t checksum = *ptr;
  for (int i=1; i<numChecksummedBytes; i++) {
    checksum = checksum ^ (*(ptr + i));
  }

  return checksum;
}

bool ChinaBee::test_checksum(cv_data_t* cvData) {
  return (generate_checksum(cvData) == cvData->checksum);
}


