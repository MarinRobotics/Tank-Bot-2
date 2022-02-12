#include "main.h"
//#include "helper.h"

pros::Controller master(pros::E_CONTROLLER_MASTER);

pros::Motor left_front_mtr(11);
pros::Motor left_back_mtr(18);
pros::Motor right_front_mtr(3,true);
pros::Motor right_back_mtr(10,true);
pros::Motor arm_turntableA(2);
pros::Motor arm_turntableB(9, true);

pros::Motor jaw (19);
pros::Motor crane_rotate (20);
bool pressed = false;

void on_center_button() {
	pressed = !pressed;
	if (pressed) {
		pros::lcd::print(2, "Auton 2 selected %d", pressed);
	} else {
		pros::lcd::clear_line(2);
	}

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

	int right_speed = 0;
	int left_speed = 0;

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
	pros::lcd::set_text(1, "Hello PROS User!");
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
		//awp A
		//To Do: add a way to switch between auton modes by pressing a button

		//rotate crane from diagonal to the right to over the goal
		crane_rotate.move_relative(70, 60);

		//release preload rings into the goal
		jaw.tare_position();
		jaw.move_absolute(550, 60);
		pros::delay(1000);
		jaw.move_absolute(-450, 60);

		//turns to the right
		right_back_mtr.move_relative(100, 127);
		right_front_mtr.move_relative(100, 127);

		left_front_mtr.move_relative(-100, 127);
		left_back_mtr.move_relative(-100, 127);

		//pushes goal up the ramp

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
	//motor gearset for movement
	int f = floor(300/127);
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

		right_x = master.get_analog(ANALOG_RIGHT_X);
		right_y = master.get_analog(ANALOG_RIGHT_Y);
		left_y = master.get_analog(ANALOG_LEFT_Y);
		left_x = master.get_analog(ANALOG_LEFT_X);


		f = floor(300/127); // 300 is the rpm of the current motor gearbox/127 is the max input the motors will take

		//tank steer
		//right
		right_front_mtr = right_y - right_x;
		right_back_mtr = right_y - right_x;
		//left
		left_front_mtr = right_y + right_x;
		left_back_mtr = right_y + right_x;

		if(master.get_digital(DIGITAL_LEFT)){
		  crane_rotate = 80;
		} else if(master.get_digital(DIGITAL_RIGHT)){
			crane_rotate = -80;
		} else{
			crane_rotate = 0;
		}
		//crane rotation code w/ limiters
		if (crane_rotate.get_position() > -1950 && crane_rotate.get_position() < 1950) {
			crane_rotate = 0.5 * left_x;
		} else if (crane_rotate.get_position() < -1950) {
			if (left_x > 0) {
					crane_rotate = 0.5 * left_x;
			} else {
				crane_rotate = 0;
			}
		} else if (crane_rotate.get_position() > 1950) {
			if (left_x < 0) {
				crane_rotate = 0.5 * left_x;
			} else {
				crane_rotate = 0;
			}
		} else {
			crane_rotate = 0;
		}

		//crane up/down code w/ limiters
		arm_turntableA = 0.5 * left_y;
		arm_turntableB = 0.5 * left_y;

		bool left_front_bumper = master.get_digital(DIGITAL_L1);
		bool left_back_bumper = master.get_digital(DIGITAL_L2);

	  if (left_front_bumper) {
			jaw.move_absolute(0, 100);
	  }else if (left_back_bumper) {
	  	jaw.move_absolute(175, 100);
	  }

		pros::lcd::set_text(1, "Hello PROS User!");
		// pros::lcd::print(3, "%d", left_y);

		pros::delay(20);
	}
}
