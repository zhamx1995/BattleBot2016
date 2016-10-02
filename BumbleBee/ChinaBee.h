#include <stdint.h>

#ifndef _DUKE_ROBOTICS_CHINABEE
#define _DUKE_ROBOTICS_CHINABEE

#define CHINA_BEE_NUM_TEAMS 4

typedef struct {
  uint32_t x;
  uint32_t y;
  uint8_t teamNum;
  uint8_t checksum;
} cv_data_t;


typedef struct {
  float x; // meters
  float y; // meters
  bool haveFound; // Have we ever seen them?
  unsigned long timestamp; // millis
} team_status_t;

class ChinaBee {
  public:
    void init(int pin1, int pin2);
    void update();
    team_status_t* get_status(int teamNum);
    int get_num_teams();
    bool send(uint8_t teamNum, uint32_t x, uint32_t y);

  private:
    team_status_t statuses[CHINA_BEE_NUM_TEAMS];
    cv_data_t tempCvData;
    uint8_t generate_checksum(cv_data_t* cvData);

    bool test_checksum(cv_data_t* cvData);
};

#endif
