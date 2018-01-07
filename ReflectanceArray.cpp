#include "ReflectanceArray.h"

ReflectanceArrayClass::~ReflectanceArrayClass()
{
}

void ReflectanceArrayClass::Init()
{
	reflectanceSensors.init();

	for (int i = 0; i < NUM_SENSORS; i++)
	{
		m_aSensorArray[i] = 0;
	}
}

void ReflectanceArrayClass::Calibrate()
{
	// Wait 1 second and then begin automatic sensor calibration
	// by rotating in place to sweep the sensors over the line
	delay(1000);

	SPRINT("Calibrating Reflectance Array...");
	int i;
	for (i = 0; i < 80; i++)
	{
		if ((i > 10 && i <= 30) || (i > 50 && i <= 70))
			MotorsClass::GetMotorInstance().SetMotorSpeeds(-200, 200);
		else
			MotorsClass::GetMotorInstance().SetMotorSpeeds(200, -200);
		reflectanceSensors.calibrate();

		// Since our counter runs to 80, the total delay will be
		// 80*20 = 1600 ms.
		delay(20);
	}
}

ReflectanceData ReflectanceArrayClass::HandleReflectanceArray()
{
	ReadSensorData();

	ReflectanceData hitData;

	//Check to see if we've hit a wall
	int wallHitCounter = 0;
	//Loop through all the sensors
	for (int i = 0; i < NUM_SENSORS; i++)
	{
		//If they have a slight bit of darkness we will count that as a hit
		if (m_aSensorArray[i] > 100)
		{
			//Increment counter
			wallHitCounter++;
		}
	}

	//If all the sensors bar 1 have been hit then stop
	if (wallHitCounter == NUM_SENSORS - 2)
	{
		//We've hit a wall
		//Stop moving and tell user
		MotorsClass::GetMotorInstance().SetMotorSpeeds(0, 0);
		SPRINT("Wall hit!");

		//Switch to USER state
		//m_eZumoState = ZState::ZUMO_STATES::USER;
		
		hitData.hit = true;
		hitData.direction = 0;
		hitData.sensorsHit = wallHitCounter;

		return hitData;
	}

	//If we detect darkness on the left two sensors then turn right
	//Need to make these values variables for easier calibration
	//Maybe use calibrated sensor value instead?

	if (m_aSensorArray[0] > 900 || m_aSensorArray[1] > 900)
	{
		//Turn right away from wall
#if PRINT_WALL_HIT_DATA
		SPRINT("Wall hit on left!");
#endif

		hitData.hit = true;
		hitData.direction = -1;
		hitData.sensorsHit = 1;
		//m_gMotors.Turn(1, 50, true);

		return hitData;
	}
	//If we detect darkness on the right two sensors then turn left
	else if (m_aSensorArray[NUM_SENSORS - 1] > 900 || m_aSensorArray[NUM_SENSORS - 2] > 900)
	{
		//Turn left away from wall
#if PRINT_WALL_HIT_DATA
		SPRINT("Wall hit on right!");
#endif

		hitData.hit = true;
		hitData.direction = 1;
		hitData.sensorsHit = 1;

		return hitData;
		//m_gMotors.Turn(-1, 50, true);
	}

	return hitData;
}

void ReflectanceArrayClass::ReadSensorData()
{
	//Get new data from reflectance sensors and put in array
	reflectanceSensors.readLine(m_aSensorArray);

#if PRINT_ARRAY_DATA
	DisplayArrayData();
#endif
}


ReflectanceArrayClass ReflectanceArray;
