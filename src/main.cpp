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

//motor definitions - arm and jaw
pros::Motor jaw (19);
pros::Motor crane_rotate (20);


//vision sensor stuff
//define vision sensor and signatures
int sensport = 8;
pros::Vision FrontSensor(sensport);

//signatures
pros::vision_signature_s_t neutral_mogii_sig =
FrontSensor.signature_from_utility(1, 2449, 2703, 2576, -3221, -3007, -3114, 3.000, 0);

//objects
pros::vision_object_s_t nutral_mogii[3]; //3 is the max amount of detected neutral mogii

//auton vision test to make sure it works
void vision_test () {
 pros::lcd::initialize();
 bool isDoingStuff_UD = false;
 bool isDoingStuff_RO = false;

 while (true) {
	 pros::lcd::clear();
	 FrontSensor.read_by_sig(0, neutral_mogii_sig.id, 3, nutral_mogii); //The vision sensor takes a picture, finds the areas with the matching color signature provided, (3 is the max amount of objects) then stores them into an area of those objects
	 pros::screen::set_pen(COLOR_BLUE_VIOLET);
	 pros::screen::print(TEXT_MEDIUM, 4, "mogus object 0: (%d, %d)", nutral_mogii[0].x_middle_coord, nutral_mogii[0].y_middle_coord); //prints the details of the first mogii object in array on the screen
	 pros::screen::print(TEXT_MEDIUM, 5, "object count: %d", FrontSensor.get_object_count()); //prints the amount of objects detected by vision sensor

	 pros::screen::set_pen(COLOR_YELLOW);
	 //move the arm up and down to keep the signature centered
	 //check in y dimension
	 // if (nutral_mogii[0].y_middle_coord > 10 && !isDoingStuff_UD){ //NOTE: 10 may be too small a number. Check w/ testing
		//  arm_turntableA = 100;
		//  arm_turntableB = 100;
		//  pros::screen::print(TEXT_LARGE, 1,"arm moving down");
		//  isDoingStuff_UD = true;
	 // } if (nutral_mogii[0].y_middle_coord < -10 && !isDoingStuff_UD) {
		//  arm_turntableA = -100;
		//  arm_turntableB = -100;
		//  pros::screen::print(TEXT_LARGE, 1,"arm moving up");
		//  isDoingStuff_UD = true;
	 // } else {
		//  arm_turntableA = 0;
		//  arm_turntableB = 0;
		//  pros::screen::print(TEXT_LARGE, 1,"centered");
		//  isDoingStuff_UD = false;
	 // }

	 if (nutral_mogii[0].x_middle_coord > 8 && !isDoingStuff_RO){ //NOTE: 10 may be too small a number. Check w/ testing
		 crane_rotate = 50; //NOTE: check if this is moving in the right direction
		 pros::screen::print(TEXT_LARGE, 2,"arm going right");
		 isDoingStuff_RO = true;
	 } if (nutral_mogii[0].x_middle_coord < -8 && !isDoingStuff_RO) {
		 crane_rotate = -50; //NOTE: check if this is moving in the right direction
		 pros::screen::print(TEXT_LARGE, 2,"arm going left");
		 isDoingStuff_RO = true;
	 } else {
		 crane_rotate = 0;
		 pros::screen::print(TEXT_LARGE, 2,"centered");
		 isDoingStuff_RO = false;
	 }

	 pros::delay(15);

   }
}


//awp selection
bool pressed = false;

void on_center_button() {
	pressed = !pressed;
	if (pressed) {
		pros::lcd::print(4, "AWP-B selected (on line)");
	} else {
		pros::lcd::print(4, "AWP-A selected (ramp)");
	}

	//helper struct
	struct control {
		struct {
			int x = 0;
			int y = 0;
		} left;
		struct {
			int x = 0;
			int y = 0;
		} right;
	} position;

	//define drive motor speed variables by side
	int right_speed = 0;
	int left_speed = 0;

	//define controller variables
	int right_x;
	int right_y;
	int left_x;
	int left_y;
	int controller_reversed = -1;
	bool a_pressed = false;
}


/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	pros::lcd::initialize();
	//set mtr brake modes
	jaw.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
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
		//awp-b variables
		int right_move = 2800;
		int left_move = 2550;

		int right_speed = 90;
		int left_speed = 90;

		if (pressed){
			//awp b
			//move to goal
			right_back_mtr.move_relative(right_move, right_speed);
			right_front_mtr.move_relative(right_move, right_speed);
			left_back_mtr.move_relative(left_move, left_speed);
			left_front_mtr.move_relative(left_move, left_speed);
			pros::delay(2500);

			//rotate crane over the goal
			crane_rotate.move_relative(-800, 90);
			pros::delay(1000);

			//release the rings from jaw
			jaw.tare_position();
			jaw.move_absolute(550, 60);
			pros::delay(1000);
			jaw.move_absolute(-450, 60);
			pros::delay(1000);

		} else {
			vision_test();
			//awp A
			//release preload rings into the goal
			// jaw.tare_position();
			// jaw.move_absolute(550, 60);
			// pros::delay(1000);
			// jaw.move_absolute(-450, 60);
			// pros::delay(1000);
		}
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

	pros::lcd::initialize();

	//limiter variables
	crane_rotate.set_encoder_units(pros::E_MOTOR_ENCODER_DEGREES);
	arm_turntableA.set_encoder_units(pros::E_MOTOR_ENCODER_DEGREES);
	jaw.set_encoder_units(pros::E_MOTOR_ENCODER_DEGREES);

	//toggle variables
	bool a_pressed = true;

	//set the jaw's current pos as 0
	jaw.tare_position();

	while(true){

		//get stick inputs
		right_x = master.get_analog(ANALOG_RIGHT_X);
		right_y = master.get_analog(ANALOG_RIGHT_Y);
		left_y = master.get_analog(ANALOG_LEFT_Y);
		left_x = master.get_analog(ANALOG_LEFT_X);

		//get bumper presses
		bool left_front_bumper = master.get_digital(DIGITAL_L1);
		bool left_back_bumper = master.get_digital(DIGITAL_L2);

		//print stick inputs - debug
		pros::lcd::print(0, "right x: %d", right_x);
		pros::lcd::print(1, "right y: %d", right_y);

		pros::lcd::print(2, "left x: %d", left_x);
		pros::lcd::print(3, "left y: %d", left_y);

		//print motor power - debug
		pros::lcd::print(5, "right motors: %d", right_y - right_x);
		pros::lcd::print(6, "left motors: %d", right_y + right_x);

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

	  if (left_front_bumper) {
			jaw.move_absolute(0, 100);
	  }else if (left_back_bumper) {
	  	jaw.move_absolute(175, 100);
	  }

		pros::delay(20);
	}
}
