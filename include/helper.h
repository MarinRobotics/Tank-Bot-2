#include <stdlib.h>
#include "main.h"

extern pros::Controller master;
extern struct control {
  struct {
    int x = 0;
    int y = 0;
  } left;
  struct {
    int x = 0;
    int y = 0;
  } right;
} position;

bool btnpressed(std::string s) {
  if (s == "l1") return master.get_digital(DIGITAL_L1);
  if (s == "l2") return master.get_digital(DIGITAL_L2);
  if (s == "r1") return master.get_digital(DIGITAL_R1);
  if (s == "r2") return master.get_digital(DIGITAL_R2);
  if (s == "up") return master.get_digital(DIGITAL_UP);
  if (s == "down") return master.get_digital(DIGITAL_DOWN);
  if (s == "left") return master.get_digital(DIGITAL_LEFT);
  if (s == "right") return master.get_digital(DIGITAL_RIGHT);
  if (s == "x") return master.get_digital(DIGITAL_X);
  if (s == "b") return master.get_digital(DIGITAL_B);
  if (s == "y") return master.get_digital(DIGITAL_Y);
  if (s == "a") return master.get_digital(DIGITAL_A);
}

void updatePosition() {
  position.left.x = master.get_analog(ANALOG_LEFT_X);
  position.left.y = master.get_analog(ANALOG_LEFT_Y);
  position.right.x = master.get_analog(ANALOG_RIGHT_X);
  position.right.y = master.get_analog(ANALOG_RIGHT_Y);
}
