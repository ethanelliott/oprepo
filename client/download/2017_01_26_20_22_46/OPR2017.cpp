/*
* 		  File: OPR2017.cpp
 *  Created on: Jan. 2016
 *      Author: Malik Ricci
 */

#include "WPILib.h"
#include "CANTalon.h"

#include "LogitechGamepad.h"
#include "SimPID.h"
#include "OPRGyro.h"

#include "math.h"
#include "stdio.h"

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include <IterativeRobot.h>
#include <LiveWindow/LiveWindow.h>
#include <SmartDashboard/SendableChooser.h>
#include <SmartDashboard/SmartDashboard.h>

// -~- Joystick Mapping -~- //
#define DRIVER_GAME_PAD 	0

// -~- Robot Port Mapping -~- //

//PWM Ports (Motors)
#define DRIVE_LA        0
#define DRIVE_LB		1
#define DRIVE_LC 		2
#define DRIVE_RA		3
#define DRIVE_RB 		4
#define DRIVE_RC        5
#define INTAKE          6
#define CONVEYOR        7
#define SHOOTER_A       8
#define SHOOTER_B       9
#define HANGER_A        10
#define HANGER_B        11

//PCM Ports (Pneumatics)
#define SHIFTER_A       0
#define SHIFTER_B       1
#define GEAR_HOPPER     2

//Analog Port Mapping
#define GYRO           0

//Digital Port Mapping
//No Digital Sensors Currently

// -~- PID Constants -~- //
//Drive Magnitude
const float KP_MAGNITUDE 	= 0.0;
const float KI_MAGNITUDE 	= 0.0;
const float KD_MAGNITUDE 	= 0.0;
const float EPS_MAGNITUDE	= 0.0;

//Drive Direction
const float KP_TURN 		= 0.0;
const float KI_TURN 		= 0.0;
const float KD_TURN 		= 0.0;
const float EPS_TURN 		= 0.0;

//Drive Velocity
const float KP_DRIVE		= 0.0;
const float KI_DRIVE		= 0.0;
const float KD_DRIVE		= 0.0;
const float KF_DRIVE		= 0.0;
const float EPS_DRIVE		= 0.0;

//Intake
const float KP_INTAKE 		= 0.0;
const float KI_INTAKE 		= 0.0;
const float KD_INTAKE 		= 0.0;
const float KF_INTAKE		= 0.0;
const float EPS_INTAKE 		= 0.0;

//Conveyor
const float KP_CONVEYOR 	= 0.0;
const float KI_CONVEYOR		= 0.0;
const float KD_CONVEYOR 	= 0.0;
const float KF_CONVEYOR		= 0.0;
const float EPS_CONVEYOR	= 0.0;

//Shooter
const float KP_SHOOTER 		= 0.0;
const float KI_SHOOTER 		= 0.0;
const float KD_SHOOTER 		= 0.0;
const float KF_SHOOTER 		= 0.0;
const float EPS_SHOOTER 	= 0.0;

//*****// Custom Constants //*****//
const float HANGER_DEADBAND = 0.2;

//*****// Unit Conversion //*****//
const double pi = 3.14159;

// -~-~-~-~-~-~-~-~-~-~- Enumerations -~-~-~-~-~-~-~-~-~-~- //
enum Drive 		{	//driveState
	NO_DRIVE,
	JOYSTICK_DRIVE,
	AUTO_DRIVE,
	BASELOCK_DRIVE,
};
enum Intake 	{	//intakeState
	INTAKE_OFF,
	INTAKE_IN,
	INTAKE_OUT
};
enum Convyer 	{	//conveyorState
	 CONVEYOR_OFF,
	 CONVEYOR_IN,
	 CONVEYOR_OUT
 };
enum Shooter 	{	//shooterState
	SHOOTER_OFF,
	SHOOTER_ON
};
enum Hopper 	{	//hopperState
	HOPPER_IN,
	HOPPER_OUT
};	//hopperState

class OPR2017: public frc::IterativeRobot
{
	// -~-~-~-~-~-~-~-~-~-~- Object Initialization -~-~-~-~-~-~-~-~-~-~- //

	// -~- Robot Drive -~- //
	LogitechGamepad* controller;
	DoubleSolenoid* shifter;
	RobotDrive* driveTrain;
	SimPID* pidDrive;
	SimPID* pidTurn;

	CANTalon* driveLMaster;
	CANTalon* driveLFollowA;
	CANTalon* driveLFollowB;

	CANTalon* driveRMaster;
	CANTalon* driveRFollowA;
	CANTalon* driveRFollowB;

	// -~- Electrical -~- //
	PowerDistributionPanel * pdp;
	Compressor* compressor;

	OPRGyro* gyro;

	// -~- Actuators -~- //
	CANTalon* intake;
	CANTalon* conveyor;

	CANTalon* shooterA;
	CANTalon* shooterB;

	VictorSP* hangerA;
	VictorSP* hangerB;

	Solenoid* hopper;

	// -~- Global Variables -~- //
	int autoState;
	int driveState, intakeState, conveyorState, shooterState, hopperState;

	float hangerValue;
	float encoderOld;
	float xPos, yPos;

public:

	// -~- Constructor -~- //
	OPR2017(void){
		//DriveTrain
		controller = new LogitechGamepad(DRIVER_GAME_PAD);
		shifter 	= new DoubleSolenoid(SHIFTER_A, SHIFTER_B);
		driveTrain	= new RobotDrive(DRIVE_LA, DRIVE_RA);
		pidDrive 	= new SimPID();
		pidTurn 	= new SimPID();

		driveLMaster = new CANTalon(DRIVE_LA);
		driveLFollowA = new CANTalon(DRIVE_LB);
		driveLFollowB = new CANTalon(DRIVE_LC);

		driveRMaster = new CANTalon(DRIVE_RA);
		driveRFollowA = new CANTalon(DRIVE_RB);
		driveRFollowB = new CANTalon(DRIVE_RC);

		//Electrical
		pdp = new PowerDistributionPanel();
		compressor = new Compressor();

		//Sensors
		gyro = new OPRGyro();

		//Intake
		intake = new CANTalon(INTAKE);

		//Conveyor
		conveyor = new CANTalon(CONVEYOR);

		//Shooters
		shooterA = new CANTalon(SHOOTER_A);
		shooterB = new CANTalon(SHOOTER_B);

		//Hanger
		hangerA = new VictorSP(HANGER_A);
		hangerB = new VictorSP(HANGER_B);

		//Hopper
		hopper = new Solenoid(GEAR_HOPPER);


		//*****// State Initialization //*****//
		hopperState = HOPPER_IN;
		shooterState = SHOOTER_OFF;
		conveyorState = CONVEYOR_OFF;
		intakeState = INTAKE_OFF;
		driveState = NO_DRIVE;

		autoState = 0; //Update Later

		hangerValue = 0;

		encoderOld = 0;
		xPos = yPos = 0;

		SmartDashboard::init();

	}	//Constructor

	// -~- Destructor -~- //
	~OPR2017(void){

		delete controller;
		delete shifter;
		delete driveTrain;

		delete driveLMaster;
		delete driveLFollowA;
		delete driveLFollowB;

		delete driveRMaster;
		delete driveRFollowA;
		delete driveRFollowB;

		delete pdp;
		delete gyro;
		delete compressor;

		delete intake;
		delete conveyor;
		delete shooterA;
		delete shooterB;
		delete hangerA;
		delete hangerB;
		delete hopper;

	}	//Desctructor

private:

	void RobotInit(){
		Initialize();
	}

	void DisabledInit(){

	}

	void DisabledPeriodic(){

	}

	void AutonomousInit(){

	}

	void AutonomousPeriodic(){


	}

	void TeleopInit(){
		driveState = JOYSTICK_DRIVE;
		gyro->UpdateGyro();
	}

	void TeleopPeriodic(){
		MainRoutine();
	}

	// -~-~-~-~-~-~-~-~-~-~- Routines -~-~-~-~-~-~-~-~-~-~- //

	void Initialize(){	//Setup motor controllers, PID, & reset sensors
		printf("Starting Initialization\n");

		driveTrain->SetSafetyEnabled(false);

		compressor->SetClosedLoopControl(false);
		compressor->Start();

		//Hanger run on VictorSP, must set Brake/Coast physically, no internal PID
		hangerB->SetInverted(true);

		/* Need to set each of these...
			-Control Mode
			-Reverse CL Output?

			-Feedback Sensor
			-Reverse Sensor?

			-Voltage Ramp Rate
			-Brake/Coast
		 */


		// -~-~-~-~-~-~-~-~-~-~- Control Mode Setup -~-~-~-~-~-~-~-~-~-~- //
		driveLMaster->SetControlMode(CANSpeedController::kSpeed);
		driveRMaster->SetClosedLoopOutputDirection(false);

		driveLFollowA->SetControlMode(CANSpeedController::kFollower);
		driveLFollowB->SetControlMode(CANSpeedController::kFollower);
		driveLFollowA->Set(98);		//Follow Left Master (driveLMaster)
		driveLFollowB->Set(98);		//Follow Left Master (driveLMaster)

		driveRMaster->SetControlMode(CANSpeedController::kSpeed);
		driveRMaster->SetClosedLoopOutputDirection(true);

		driveRFollowA->SetControlMode(CANSpeedController::kFollower);
		driveRFollowB->SetControlMode(CANSpeedController::kFollower);
		driveRFollowA->Set(99);		//Follow Right Master (driveRMaster)
		driveRFollowB->Set(99);		//Follow Right Master (driveRMaster)

		intake->SetControlMode(CANSpeedController::kSpeed);
		intake->SetClosedLoopOutputDirection(false);

		conveyor->SetControlMode(CANSpeedController::kSpeed);
		conveyor->SetClosedLoopOutputDirection(false);

		shooterA->SetControlMode(CANSpeedController::kSpeed);
		shooterA->SetClosedLoopOutputDirection(false);

		shooterB->SetControlMode(CANSpeedController::kFollower);
		shooterB->SetClosedLoopOutputDirection(true);
		shooterB->Set(97); 			//Follow to shooterA


		// -~-~-~-~-~-~-~-~-~-~- Feedback Sensor Setup -~-~-~-~-~-~-~-~-~-~- //
		driveLMaster->SetFeedbackDevice(CANTalon::CtreMagEncoder_Relative);
		driveRMaster->SetFeedbackDevice(CANTalon::CtreMagEncoder_Relative);
		driveLMaster->SetSensorDirection(false);
		driveRMaster->SetSensorDirection(false);

		intake->SetFeedbackDevice(CANTalon::CtreMagEncoder_Relative);
		intake->SetSensorDirection(false);

		conveyor->SetFeedbackDevice(CANTalon::CtreMagEncoder_Relative);
		conveyor->SetSensorDirection(false);

		shooterA->SetFeedbackDevice(CANTalon::CtreMagEncoder_Relative);
		shooterA->SetSensorDirection(false);


		// -~-~-~-~-~-~-~-~-~-~- Brake/Coast Setup -~-~-~-~-~-~-~-~-~-~- //
		driveLMaster->ConfigNeutralMode(CANTalon::kNeutralMode_Coast);
		driveLFollowA->ConfigNeutralMode(CANTalon::kNeutralMode_Coast);
		driveLFollowB->ConfigNeutralMode(CANTalon::kNeutralMode_Coast);

		driveRMaster->ConfigNeutralMode(CANTalon::kNeutralMode_Coast);
		driveRFollowA->ConfigNeutralMode(CANTalon::kNeutralMode_Coast);
		driveRFollowB->ConfigNeutralMode(CANTalon::kNeutralMode_Coast);

		intake->ConfigNeutralMode(CANTalon::kNeutralMode_Coast);
		conveyor->ConfigNeutralMode(CANTalon::kNeutralMode_Brake);

		shooterA->ConfigNeutralMode(CANTalon::kNeutralMode_Coast);
		shooterB->ConfigNeutralMode(CANTalon::kNeutralMode_Coast);


		// -~-~-~-~-~-~-~-~-~-~- PID Constant Initialization -~-~-~-~-~-~-~-~-~-~- //
		// -~- Simbotics PID: Magnitude -~- //
		pidDrive->setConstants(KP_MAGNITUDE, KI_MAGNITUDE, KD_MAGNITUDE);
		pidDrive->setMinDoneCycles(3);
		pidDrive->setErrorEpsilon(EPS_DRIVE);
		//pidDrive->setIzone(5);
		//pidDrive->setDzone(5);
		//pidDrive->setErrorIncrement(3);
		//pidDrive->setMaxOutput(0.6);

		// -~- Simbotics PID: Turn -~- //
		pidTurn->setConstants(KP_TURN, KI_TURN, KD_TURN);
		pidTurn->setMinDoneCycles(3);
		pidTurn->setErrorEpsilon(EPS_TURN);
		//pidTurn->setIzone(5);
		//pidTurn->setDzone(5);
		//pidTurn->setErrorIncrement(3);
		//pidTurn->setMaxOutput(0.6);

		// -~- Talon PID -~- //
		//Velocity Control for Drivetrain
		driveLMaster->SetPID(KP_DRIVE, KI_DRIVE, KD_DRIVE, KF_DRIVE);
		//driveLMaster->SetAllowableClosedLoopErr(0);
		//driveLMaster->SetIzone(RPMtoTicks(250));
		//driveLMaster->SetVoltageRampRate(12);

		driveRMaster->SetPID(KP_DRIVE, KI_DRIVE, KD_DRIVE, KF_DRIVE);
		//driveRMaster->SetAllowableClosedLoopErr(0);
		//driveRMaster->SetIzone(RPMtoTicks(250));
		//driveRMaster->SetVoltageRampRate(12);

		//Intake
		intake->SetPID(KP_INTAKE, KI_INTAKE, KD_INTAKE, KF_INTAKE);
		intake->SetAllowableClosedLoopErr(0);
		intake->SetIzone(RPMtoTicks(500));
		intake->SetVoltageRampRate(36);

		//Conveyor
		conveyor->SetPID(KP_CONVEYOR, KI_CONVEYOR, KD_CONVEYOR, KF_CONVEYOR);
		conveyor->SetAllowableClosedLoopErr(0);
		conveyor->SetIzone(RPMtoTicks(500));
		conveyor->SetVoltageRampRate(36);


		//Shooter
		shooterA->SetPID(KP_SHOOTER, KI_SHOOTER, KD_SHOOTER, KF_SHOOTER);
		shooterA->SetAllowableClosedLoopErr(0);
		shooterA->SetIzone(RPMtoTicks(120));
		shooterA->SetVoltageRampRate(120);

		SensorReset();

		printf("Initialization Complete.\n");
	}	//Initialize

	void SensorReset(){	//Resets Sensors and X/Y position
		xPos = yPos = 0;
		autoState = 0;
		gyro->Reset();

		driveLMaster->SetPosition(0);
		driveRMaster->SetPosition(0);

		intake->SetPosition(0);
		conveyor->SetPosition(0);
		shooterA->SetPosition(0);

		printf("Sensors Reset!\n");
	}	//SensorReset

	void MainRoutine(){ //Call State Machines and Control Compressor
		driveStateMachine();
		intakeStateMachine();
		conveyorStateMachine();
		shooterStateMachine();
		hopperStateMachine();
		hangerStateMachine();

		if (shooterState == SHOOTER_ON || hangerValue > HANGER_DEADBAND)
		{
			compressor->Stop();
		} else {
			compressor->Start();
		}
	}

	// -~-~-~-~-~-~-~-~-~-~- State Machines -~-~-~-~-~-~-~-~-~-~- //

	void driveStateMachine(){
		switch (driveState) {
			case NO_DRIVE: //No motor outputs
				ArcadeDrive(0, 0, false, false);
				SmartDashboard::PutString("Drive", "No Drive");
				break;

			case JOYSTICK_DRIVE: // Drive using joystick
				ArcadeDrive(controller->GetLeftX(), controller->GetLeftY());
				SmartDashboard::PutString("Drive", "Manual");
				break;

			case AUTO_DRIVE: //Driving in autonomous
				SmartDashboard::PutString("Drive", "Auto");
				break;

			case BASELOCK_DRIVE: //Hold position automatically
				DriveDefinedHeading(0,0,0);
				SmartDashboard::PutString("Drive", "Baselock");
				break;
		}	//switch

		if(controller->GetNumberedButton(8)){
			shiftHigh();
		} else {
			shiftLow();
		}

		if(controller->GetNumberedButton(10)){
			driveState = BASELOCK_DRIVE;
		} else{
			driveState = JOYSTICK_DRIVE;
		}

	}	//driveStateMachine

	void intakeStateMachine(){
		switch (intakeState){
			case INTAKE_OFF:
				setVoltagePct(intake, 0);
				SmartDashboard::PutString("Intake", "OFF");
				break;

			case INTAKE_IN:
				setTalonTargetRPM(intake, 2000);
				SmartDashboard::PutString("Intake", "IN");
				break;

			case INTAKE_OUT:
				setTalonTargetRPM(intake, -1000);
				SmartDashboard::PutString("Intake", "OUT");
				break;
		}	//switch

		if(controller->GetNumberedButton(1)){
			intakeState = INTAKE_IN;
		} else if(controller->GetNumberedButton(2)){
			intakeState = INTAKE_OFF;
		}else if(controller->GetNumberedButton(9)){
			 intakeState = INTAKE_OUT;
		}
	}	//intakeStateMachine

	void conveyorStateMachine(){
		switch (conveyorState){
			case CONVEYOR_OFF:
				setVoltagePct(conveyor, 0);
				SmartDashboard::PutString("Conveyor", "OFF");

				if(controller->GetNumberedButton(6)){
					conveyorState = CONVEYOR_IN;
				} else if(controller->GetNumberedButton(5)){
					conveyorState = CONVEYOR_OUT;
				}
				break;

			case CONVEYOR_IN:
				setTalonTargetRPM(conveyor, 2000);
				SmartDashboard::PutString("Conveyor", "IN");

				if(!controller->GetNumberedButton(6)){
					conveyorState = CONVEYOR_OFF;
				}
				break;

			case CONVEYOR_OUT:
				setTalonTargetRPM(conveyor, -1000);
				SmartDashboard::PutString("Conveyor", "OUT");

				if(!controller->GetNumberedButton(5)){
					conveyorState = CONVEYOR_OFF;
				}
				break;
		}	//switch
	}	//conveyorStateMachine

	void shooterStateMachine(){
		switch (shooterState){
			case SHOOTER_OFF:
				setVoltagePct(shooterA, 0);
				SmartDashboard::PutString("Shooter", "OFF");
				SmartDashboard::PutString("ShooterReady", "NO");
				break;

			case SHOOTER_ON:
				SmartDashboard::PutString("Shooter", "ON");

				if(fabs(setTalonTargetRPM(shooterA, 3500)) < EPS_SHOOTER){
					SmartDashboard::PutString("ShooterReady","YES");
				} else{
					SmartDashboard::PutString("ShooterReady","NO");
				}
				break;
		}	//switch

		if(controller->GetNumberedButton(4)){
			shooterState = SHOOTER_ON;
		} else if(controller->GetNumberedButton(3)){
			shooterState = SHOOTER_OFF;
		}

		SmartDashboard::PutNumber("ShooterSpeed", shooterA->GetSpeed());
	}	//shooterStateMachine

	void hopperStateMachine(){
		switch (hopperState){
			case HOPPER_IN:
				hopperClosed();
				SmartDashboard::PutString("Hopper", "CLOSED");
				break;
			case HOPPER_OUT:
				hopperOpen();
				SmartDashboard::PutString("Hopper", "OPEN");
				break;
		}	//switch

		if(controller->GetNumberedButton(7)){
			hopperState = HOPPER_OUT;
		} else{
			hopperState = HOPPER_IN;
		}
	}	//hopperStateMachine

	void hangerStateMachine(){
		hangerValue = fabs(controller->GetRightY());

		if(hangerValue > HANGER_DEADBAND){
			hangerA->Set(hangerValue);
			hangerB->Set(hangerValue); //Value is inverted during initialization
			SmartDashboard::PutString("Hanger", "RUNNING");
		} else{
			SmartDashboard::PutString("Hanger", "OFF");
		}
	}	//hangerStateMachine

	// -~-~-~-~-~-~-~-~-~-~- Control Functions -~-~-~-~-~-~-~-~-~-~- //

	void setVoltagePct(CANTalon* talon, float pctVoltage = 0.0){

		talon->SetControlMode(CANSpeedController::kPercentVbus);
		talon->Set(LimitOutput(pctVoltage, 1.0));
	}

	int setTalonTargetRPM(CANTalon* talon, float RPM){
		float ticks = RPMtoTicks(RPM);
		talon->SetControlMode(CANSpeedController::kSpeed);
		talon->Set(ticks);
		return talon->GetClosedLoopError();
	}

	// -~-~-~-~-~-~-~-~-~-~- Location Processing -~-~-~-~-~-~-~-~-~-~- //

	void UpdatePos(){
		long encoders = EncoderAvg();
		yPos += (encoders - encoderOld) * cos(DegToRad(gyro->GetAngle()));
		xPos += (encoders - encoderOld) * sin(DegToRad(gyro->GetAngle()));
		encoderOld = encoders;
		SmartDashboard::PutNumber("xPos", xPos);
		SmartDashboard::PutNumber("yPos", yPos);
	}			//UpdatePos

	long EncoderAvg(){
		return ((driveLMaster->GetPosition() + driveRMaster->GetPosition()) / 2);
	}

	// -~-~-~-~-~-~-~-~-~-~- Driving Functions -~-~-~-~-~-~-~-~-~-~- //

	void ArcadeDrive(float rotateAxis, float magnitudeAxis, bool sqRotateInput = true, bool sqMagnitudeInput = true){
		float leftOutput;
		float rightOutput;

		if (sqRotateInput) {
			rotateAxis = SignSquare(rotateAxis);
		}
		if (sqMagnitudeInput) {
			magnitudeAxis = SignSquare(magnitudeAxis);
		}

		leftOutput = LimitOutput(magnitudeAxis + rotateAxis);
		rightOutput = LimitOutput(magnitudeAxis - rotateAxis);

		driveTrain->TankDrive(leftOutput, rightOutput, false);

	}			//ArcadeDrive

	float SignSquare(float input) {
		if (input < 0) {
			input *= (-input);
		} else {
			input *= input;
		}
		return input;
	}			//SignSquare

	float LimitOutput(float input, const float LIMIT = 1.0) {
		if(input > LIMIT){
			input = LIMIT;
		} else if(input < -LIMIT){
			input = -LIMIT;
		}
		return input;
	}	//LimitOutput


	bool simplePIDdrive(float heading, float distance) {
		pidTurn->setIzone(0);
		pidDrive->setDesiredValue(distance);
		pidTurn->setDesiredValue(heading);
		float y = pidDrive->calcPID(EncoderAvg());
		float x = pidTurn->calcPID(gyro->GetAngle());
		ArcadeDrive(x, y, false, false);
		return (pidDrive->isDone() && pidTurn->isDone());
	}	//simplePIDdrive

	bool simplePIDturn(float heading) {
		pidTurn->setIzone(0);
		pidTurn->setDesiredValue(heading);
		float y = 0;
		float x = pidTurn->calcPID(gyro->GetAngle());
		//printf("Turn Output: %f\n", x);
		ArcadeDrive(x, y, false, false);
		return (pidTurn->isDone());
	}	//simplePIDturn




	double IPStoRPM(float ips, bool lowGear){
		//Convert speed in feet per second to sensor RPM
		double ratio = (36/12) * (36/18);

		//RPM = (inches per min.)/(inches per wheel rev.)*(sensor rev. per wheel rev.)
		return (ips * 60)/(4*pi)*ratio;

		double f = 1023/RPMtoTicks(5300);

		float wheelBase = 25.875;
		float rot, mag, left, right;
		//assuming left and right are given in inches per second
		rot = (left - right) / wheelBase; //rotation in radians per second
		mag = (left + right) / 2; //distance forward in inches per second

		//need to find inverse, given rotValue and magValue as a percentage of 100% to -100%
		//define max velocity for left and right
		// right = mag - wB*rot/2
		// left = mag + wb*rot/2

	}


	bool DriveDefinedHeading(float targetX, float targetY, float desiredHeading)
	/*
	Desc:	Drives Robot to an (x,y) target along the initial 'DesiredHeading'
	Pre:	targetX - distance to travel in the X direction
			targetY - distance to travel in the Y direction
			desiredHeading - initial heading to travel in
	Post: 	When Robot sucessfully moves to desired location, function returns true
	Note: 	This drive method WILL NOT propogate PID Errors if used in a sequence
	*/
	{
		pidTurn->setIzone(0.001); //Same as removing I term without resetting PID terms globally
		const float MIN_DISTANCE = 1000;
		const float SCALE_X = 1/200;
		const float HEADING_CAP = 45;

		float c = Pyth(targetX - xPos, targetY - yPos);	//absolute distance to waypoint
		float theta = RadToDeg(atan2(targetX - xPos, targetY - yPos));//angle to waypoint (-180 to 180 degrees)

		//	 _B__
		//  |   /
		// A|  / C
		//	| /
 	 	// 	|/

		float a = c * cos(DegToRad(desiredHeading - theta));//y component of distance
		float b = c * sin(DegToRad(desiredHeading - theta));//x component of distance

		//Only Set Lane Direction when y component of distance is a significant amount away
		float laneDirection;
		if (a > MIN_DISTANCE) {
			laneDirection = 1;
		} else {
			laneDirection = -1;
		}

		//set current heading to an incremental change to allow PID to scale rotation
		float heading = desiredHeading - (LimitOutput((SCALE_X * b), HEADING_CAP) * laneDirection);

		//Desired Distance is always 0, since distance to target always decreases
		pidDrive->setDesiredValue(0);
		pidTurn->setDesiredValue(heading);

		//Set Current Drive Ouptut Based on PID of current y distance to target
		float y = pidDrive->calcPID(a);
		float headingError = heading - gyro->GetAngle();

		//Scale output in order to allow for large turns to be completed before moving forward
		y *= DriveScaling(headingError);

		//Set Rotation Output Based on PID of current Angle
		float x = pidTurn->calcPID(gyro->GetAngle());

		ArcadeDrive(x, -y, false, false);

		//returns true after sucessfully reaching its location
		if (pidDrive->isDone()) {
			return true;
		} else {
			return false;
		}
	}			//DriveDefinedHeading

	float DriveScaling(float x)
	/*Desc: Used in combination with DriveDefinedLane() to Scale Drive Output
	 based upon the level of turning required (high heading means low drive output
	 and vice versa)
	 Pre:	x - Heading Input
	 Post:	returns a scale factor for driving output
	 */
	{
		float y = 0;
		const int MIN_HEADING = 5;//minimum heading where drive scaling is applied
		const int MAX_HEADING = 25;	//maximum heading where drive scaling is applied

		if (fabs(x) < MIN_HEADING)
			y = 1;
		else if (fabs(x) > MAX_HEADING)
			y = 0;
		else {
			//linearly scale output between 1 and 0 from MIN_HEADING to MAX_HEADING
			y = (-fabs(x) + MAX_HEADING) / (MAX_HEADING - MIN_HEADING);
		}
		return y;
	}	//DriveScaling

	// -~-~-~-~-~-~-~-~-~-~- Math / Conversions -~-~-~-~-~-~-~-~-~-~- //

	float Pyth(float x, float y) {
		return (sqrt(x * x + y * y));
	}	//Pyth

	float DegToRad(float input) {
		return ((input * pi) / 180);
	}	//DegToRad

	float RadToDeg(float input) {
		return ((input * 180) / pi);
	}	//RadToDeg

	float RPMtoTicks(float RPM){	//Speed converted from RPM to ticks/100ms
		float ticks = RPM * 4096 / 600;
		return ticks;
	}	//RPMtoTicks

	float TickstoRPM(float ticks){	//Speed converted from ticks/100ms to RPM
		float RPM = ticks / 4096 * 600;
		return RPM;
	}	//TickstoRPM

	// -~-~-~-~-~-~-~-~-~-~- Wrapper Functions -~-~-~-~-~-~-~-~-~-~- //

	void shiftHigh(){		//Shift to High Gear
		shifter->Set(DoubleSolenoid::kForward);
	}	//shiftHigh

	void shiftLow(){		//Shift to Low Gear
		shifter->Set(DoubleSolenoid::kReverse);
	}	//shiftLow

	void hopperOpen(){		//Open Gear Hopper
		hopper->Set(true);
	}

	void hopperClosed(){	//Close Gear Hopper
		hopper->Set(false);
	}
};

START_ROBOT_CLASS(OPR2017)
