#include "main.h"
//#include "helper.h"

pros::Controller master(pros::E_CONTROLLER_MASTER);

//motor definitions - drive
pros::Motor left_front_mtr(11);
pros::Motor left_back_mtr(18);
pros::Motor right_front_mtr(3,true);
pros::Motor right_back_mtr(10,true);
pros::Motor arm_turntableA(2);
pros::Motor arm_turntableB(9, true);

//motor definitions - trigger and jaw
pros::Motor trigger (19);
pros::Motor crane_rotate (20);

//define drive motor speed variables by side
int right_speed = 0;
int left_speed = 0;

//define controller variables
int right_x;
int right_y;
int left_x;
int left_y;

//vision sensor stuff
//define vision sensor and signatures
int sensport = 8;
pros::Vision FrontSensor(sensport);

//sensitivity variables
int centerX = 158;
int centerY = 106;
int mtrSpeedX = 0;
int mtrSpeedY = 0;

//error calculations
int errorAmountX = 0;
int errorAmountY = 0;
float gainX = 0.5;
float gainY = 0.6;

//limiters
int motorPosX = 0;
int motorPosY = 0;
bool isOverThreshY1 = false; //over positive thresh
bool isOverThreshY2 = false; //over negative thresh

bool isOverThreshX1 = false;
bool isOverThreshX2 = false;

//signatures
pros::vision_signature_s_t red_target_sig =
FrontSensor.signature_from_utility(1, 8163, 9675, 8918, -595, 177, -208, 3.000, 0);

//objects
pros::vision_object_s_t red_target[3]; //3 is the max amount of detected objects

// _______ \\
//FUNCTIONS\\
//‾‾‾‾‾‾‾‾‾\\

//this sets the trigger's firing mode (full auto)
void setTrigMode(int speedMult, int fireMode){
  pros::lcd::initialize();
    if (fireMode == 1){
      trigger = 127;
      pros::lcd::print(1, "firing");
    }

    else {
      trigger.move_velocity(0);
      pros::lcd::print(1, "standby");
    }
}

/////////////////////

//max dimensions of vision sensor view: 315 in x, 211 in y
//ratio is 315/2 for mid x, 211/2 for mid Y.
//center of target (mid x, mid y) - center coords of the obj = amount of error, or dist from center
//then error * k (a constant)
//multiplying the error makes you approach target quicker,
//and negates the asymptotic effect of linearly proportional motor speeds

void CalculateErrorAmounts(){
  //find the difference between current center pos and desired center pos (in x and then y)
  errorAmountX = int(gainX*(red_target[0].x_middle_coord - centerX));
  errorAmountY = int(gainY*(red_target[0].y_middle_coord - centerY));

  //clamp errorAmounts in range the mtrs can take
  std::clamp(errorAmountX, 127, -127);
  std::clamp(errorAmountY, 127, -127);

  //correct the motor move increments to fit amount of error
  mtrSpeedX = errorAmountX;
  mtrSpeedY = errorAmountY;
}

/////////////////////

void MoveMotors(){
  //limits
  int motorThreshX = 4300;
  int motorThreshY = 1000;
  motorPosX = crane_rotate.get_position();
  motorPosY = arm_turntableA.get_position();

  //use the computed values for mtr speed to move motors
  //on the x (side-side) crane_rotate w/ limits
  crane_rotate = (motorPosX >= motorThreshX) ? isOverThreshX1 = true : mtrSpeedX, isOverThreshX1 = false;
  crane_rotate = (motorPosX <= -motorThreshX) ? isOverThreshX2 = true : mtrSpeedX, isOverThreshX2 = false;

  isOverThreshX1 ? std::clamp(mtrSpeedX, 0, -127) : std::clamp(mtrSpeedX, 127, -127); //is the mtr pos X over the positive threshold? if so, make the speed only be negative.
  isOverThreshX2 ? std::clamp(mtrSpeedX, 127, 0) : std::clamp(mtrSpeedX, 127, -127); //is the mtr pos X over the negative threshold? if so, make the speed only be negative.

  //on the y (up-down) arm_turntableA && B w/ limits
  arm_turntableA = (motorPosY >= motorThreshY) ? isOverThreshY1 = true : mtrSpeedY, isOverThreshY1 = false;
  arm_turntableB = (motorPosY <= -motorThreshY) ? isOverThreshY2 = true : mtrSpeedY, isOverThreshY2 = false;

  isOverThreshY1 ? std::clamp(mtrSpeedY, 0, -127) : isOverThreshY1 = false, std::clamp(mtrSpeedY, 127, -127); //is the motor pos Y over the positive threshold? if so, make the speed only be negative.
  isOverThreshY2 ? std::clamp(mtrSpeedY, 127, 0) : isOverThreshY2 = false, std::clamp(mtrSpeedY, 127, -127); //is the motor pos Y over the negative threshold? if so, make the speed only be postive.

  //if mtr speed is less than 5 and/or greater than -5, set it to 0.
  mtrSpeedX = (mtrSpeedX <= 5 && mtrSpeedX > 0) || (mtrSpeedX >= -5 && mtrSpeedX < 0) ? mtrSpeedX = 0 : mtrSpeedX;
  mtrSpeedY = (mtrSpeedY <= 5 && mtrSpeedY > 0) || (mtrSpeedY >= -5 && mtrSpeedY < 0) ? mtrSpeedY = 0 : mtrSpeedY;
}

/////////////////////

void setParams(){
  pros::lcd::clear();
  FrontSensor.read_by_sig(0, red_target_sig.id, 3, red_target); //The vision sensor takes a picture, finds the areas with the matching color signature provided, (3 is the max amount of objects) then stores them into an area of those objects
  pros::screen::set_pen(COLOR_BLUE_VIOLET); //these help distinguish between debug sections
  pros::screen::print(TEXT_SMALL, 4, "mogus object 0: (%d, %d)", red_target[0].x_middle_coord, red_target[0].y_middle_coord); //prints the details of the first mogii object in array on the screen
  pros::screen::print(TEXT_SMALL, 5, "object count: %d", FrontSensor.get_object_count()); //prints the amount of objects detected by vision sensor
  pros::screen::print(TEXT_SMALL, 6, "abs mtr pos X: %d", motorPosX); //prints the amount of objects detected by vision sensor
  pros::screen::set_pen(COLOR_YELLOW);
}

/////////////////////

//auton vision test to make sure it works
void vision_test () {
 pros::lcd::initialize();

 while (true) {
	 setParams();
	 CalculateErrorAmounts();
   MoveMotors();
	 pros::delay(10);
   }
}

/////////////////////

//awp selection
bool pressed = false;

void on_center_button() {
	pressed = !pressed;
	if (pressed) {
		pros::lcd::print(4, "Autonomous Targeting");
	} else {
		pros::lcd::print(4, "Manual Targeting");
	}
}

/////////////////////

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	pros::lcd::initialize();
	//set mtr brake modes
	arm_turntableA.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
	arm_turntableB.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);

	pros::lcd::register_btn1_cb(on_center_button);
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
  vision_test();
}

//I hate this robot so much. you dont understand.
//and people say I dont comment my code
/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
	//define inputs (in this case the sticks)
	int right_x;
	int right_y;
	int left_y;
	int left_x;

  //tare trigger pos for single-shot
  trigger.tare_position();

  //if the trigger is in the act of firing
  bool isFiring = false;

  //initialize lcd for prints
	pros::lcd::initialize();

	//limiter variables
	crane_rotate.set_encoder_units(pros::E_MOTOR_ENCODER_DEGREES);
	arm_turntableA.set_encoder_units(pros::E_MOTOR_ENCODER_DEGREES);


	while(true){

		//get stick inputs
		right_x = master.get_analog(ANALOG_RIGHT_X);
		right_y = master.get_analog(ANALOG_RIGHT_Y);
		left_y = master.get_analog(ANALOG_LEFT_Y);
		left_x = master.get_analog(ANALOG_LEFT_X);

		//get bumper presses
		bool right_front_bumper = master.get_digital(DIGITAL_R1);

		//one-stick tank steer
		float motor_mult = 1.5; //motor speed multiplier

		//right drive
		right_front_mtr.move_velocity((right_y - right_x)*motor_mult);
		right_back_mtr.move_velocity((right_y - right_x)*motor_mult);

		//left drive
		left_front_mtr.move_velocity((right_y + right_x)*motor_mult);
		left_back_mtr.move_velocity((right_y + right_x)*motor_mult);

		//crane rotation code w/ limiters
		if (crane_rotate.get_position() > -1950 && crane_rotate.get_position() < 1950) {
			crane_rotate = 0.8 * left_x;
		} else if (crane_rotate.get_position() < -1950) {
			if (left_x > 0) {
					crane_rotate = 0.8 * left_x;
			} else {
				crane_rotate = 0;
			}
		} else if (crane_rotate.get_position() > 1950) {
			if (left_x < 0) {
				crane_rotate = 0.8 * left_x;
			} else {
				crane_rotate = 0;
			}
		} else {
			crane_rotate = 0;
		}

		//crane up/down code w/ limiters
		arm_turntableA = 0.8 * left_y;
		arm_turntableB = 0.8 * left_y;

    //firing modes
	  if (right_front_bumper && isFiring == false) { //single shot
			trigger.move_relative(-900, 127);
      isFiring = true;
      pros::delay(450);
	  }
    else if (!right_front_bumper && isFiring){
      isFiring = false;
    }
    if (master.get_digital(DIGITAL_A)){
      trigger.move_absolute(0, -127);
    }
		pros::delay(20);
	}
}
