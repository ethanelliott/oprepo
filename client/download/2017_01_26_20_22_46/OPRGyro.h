/*
 * OPRGyro.h
 *
 *  Created on: Feb 12, 2016
 *      Author: Ethan Elliott
 */

#ifndef SRC_OPRGYRO_H_
#define SRC_OPRGYRO_H_

#include "AHRS.h"

enum Direction {
	kPositive, kNegative, kNone
};

class OPRGyro {
public:
	OPRGyro(void);
	~OPRGyro(void);
	float GetAngle();
	void Reset();
	void UpdateGyro();
	Direction GetDirection();

	float GetYaw();
	float GetPitch();
	float GetRoll();

	float GetTemp();

	bool isRotating();
	bool isTurning(float threshold);
	float GetRate();
private:
	Direction direction;
	float fullAngle;
	int rotationCount;
	double oldGyroVal;
	AHRS *ahrs;

};

#endif /* SRC_OPRGYRO_H_ */
