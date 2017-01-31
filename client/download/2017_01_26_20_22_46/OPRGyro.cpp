/*
 * OPRGyro.cpp
 *
 *  Created on: Feb 12, 2016
 *      Author: Ethan Elliott
 */

#include "WPILib.h"
#include "OPRGyro.h"
#include "AHRS.h"
#include <iostream>

OPRGyro::OPRGyro(void) {
	direction = Direction::kNone;
	fullAngle = 0;
	rotationCount = 0;
	oldGyroVal = 0;
	ahrs = new AHRS(SPI::kMXP);
}

OPRGyro::~OPRGyro(void) {
	delete ahrs;
}

float OPRGyro::GetAngle() {
	return fullAngle;
}

void OPRGyro::Reset() {
	ahrs->ZeroYaw();
	direction = Direction::kNone;
	fullAngle = 0;
	rotationCount = 0;
	oldGyroVal = 0;
}

Direction OPRGyro::GetDirection() {
	return direction;
}

void OPRGyro::UpdateGyro() {
	double currentAngle = ahrs->GetAngle();
	double oldAngle = oldGyroVal;
	if (fabs(currentAngle - oldAngle) > 0.5) {
		if ((currentAngle - oldAngle) >= 0) {
			//Positive Rotation
			direction = Direction::kPositive;

		} else if ((currentAngle - oldAngle) <= 0) {
			//Negative Rotation
			direction = Direction::kNegative;
		} else {
			//No Rotation
			direction = Direction::kNone;
		}
	} else {
		//No Rotation
		direction = Direction::kNone;
	}

	if (currentAngle >= 340 && oldAngle <= 20) {
		rotationCount--;
	}
	if (currentAngle <= 20 && oldAngle >= 340) {
		rotationCount++;
	}
	oldGyroVal = currentAngle;
	fullAngle = (currentAngle + 360 * rotationCount);
}

float OPRGyro::GetYaw() {
	return ahrs->GetYaw();
}
float OPRGyro::GetPitch() {
	return ahrs->GetPitch();
}
float OPRGyro::GetRoll() {
	return ahrs->GetRoll();
}

float OPRGyro::GetTemp() {
	return ahrs->GetTempC();
}

bool OPRGyro::isRotating() {
	return ahrs->IsRotating();
}

bool OPRGyro::isTurning(float threshold) {
	if (fabs(ahrs->GetRate()) > threshold) {
		return true;
	} else {
		return false;
	}
}

float OPRGyro::GetRate() {
	return ahrs->GetRate();
}
