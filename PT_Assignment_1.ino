/*
* Demo line-following code for the Pololu Zumo Robot
*
* This code will follow a black line on a white background, using a
* PID-based algorithm.  It works decently on courses with smooth, 6"
* radius curves and has been tested with Zumos using 30:1 HP and
* 75:1 HP motors.  Modifications might be required for it to work
* well on different courses or with different motors.
*
* http://www.pololu.com/catalog/product/2506
* http://www.pololu.com
* http://forum.pololu.com
*/

#include <QTRSensors.h>
#include <ZumoReflectanceSensorArray.h>
#include <ZumoMotors.h>
#include <ZumoBuzzer.h>
#include <Pushbutton.h>


ZumoBuzzer buzzer;
ZumoReflectanceSensorArray reflectanceSensors;
ZumoMotors motors;
Pushbutton button(ZUMO_BUTTON);

// This is the maximum speed the motors will be allowed to turn.
// (400 lets the motors go at top speed; decrease to impose a speed limit)
const int MAX_SPEED = 250;
const int RUN_SPEED = MAX_SPEED / 2;

const int NUM_SENSORS = 6;

const int PRINT_MOD = 120;

bool run = true;

bool runReflectanceArray = true;

int leftMotorSpeed = 0;
int rightMotorSpeed = 0;

int printCounter = 0;

unsigned int sensorArray[NUM_SENSORS];

void setup()
{
	//Begin Serial communication
	Serial.begin(9600);

	// Play a little welcome song
	buzzer.play(">g32>>c32");
	buzzer.play(">g32>>c32");


	// Initialize the reflectance sensors module
	reflectanceSensors.init();

	// Wait for the user button to be pressed and released
	button.waitForButton();

	// Turn on LED to indicate we are in calibration mode
	pinMode(13, OUTPUT);
	digitalWrite(13, HIGH);

	// Wait 1 second and then begin automatic sensor calibration
	// by rotating in place to sweep the sensors over the line
	delay(1000);
	int i;
	for (i = 0; i < 80; i++)
	{
		if ((i > 10 && i <= 30) || (i > 50 && i <= 70))
			motors.setSpeeds(-200, 200);
		else
			motors.setSpeeds(200, -200);
		reflectanceSensors.calibrate();

		// Since our counter runs to 80, the total delay will be
		// 80*20 = 1600 ms.
		delay(20);
	}
	motors.setSpeeds(0, 0);

	// Turn off LED to indicate we are through with calibration
	digitalWrite(13, LOW);
	buzzer.play(">g32>>c32");

	// Wait for the user button to be pressed and released
	button.waitForButton();

	// Play music and wait for it to finish before we start driving.
	buzzer.play("L16 cdegreg4");

	//Write a message to the console
	Serial.println("Starting....");
}

//Main Loop
void loop()
{
	if (run == true)
	{
		//Read input from user through Serial connection
		readInput();

		if (runReflectanceArray)
		{
			handleReflectanceArray();
		}
	}
	else
	{
		readStartStopInput();
	}
}

void handleReflectanceArray()
{
	reflectanceSensors.readLine(sensorArray);

	//    if(printCounter > PRINT_MOD)
	//    {
	//      displayArrayData(); 
	//      printCounter = 0;  
	//    }

	//    printCounter++;

	//Check to see if we've hit a wall
	int wallHitCounter = 0;
	//Loop through all the sensors
	for (int i = 0; i < NUM_SENSORS; i++)
	{
		//If they have a slight bit of darkness we will count that as a hit
		if (sensorArray[i] > 200)
		{
			//Increment counter
			wallHitCounter++;
		}
	}

	//If all the sensors bar 1 have been hit then stop
	if (wallHitCounter == NUM_SENSORS - 1)
	{
		//We've hit a wall
		runReflectanceArray = false;
		motors.setSpeeds(0, 0);
		Serial.println("Wall hit ! ");

		return;
	}

	//If we detect darkness on the left two sensors then turn right
	//Need to make these values variables for easier calibration
	//Maybe use calibrated sensor value instead?
	if (sensorArray[0] > 500 || sensorArray[1] > 250)
	{
		//Turn right away from wall
		Turn(1, 50);
	}

	//If we detect darkness on the right two sensors then turn left
	if (sensorArray[NUM_SENSORS] > 500 || sensorArray[NUM_SENSORS - 1] > 250)
	{
		//Turn left away from wall
		Turn(-1, 50);
	}
}

void readInput()
{
	if (Serial.available()>0) {
		int input = Serial.read();
		switch (input)
		{
		case 'W':
		case 'w':
			SetMotorSpeeds(RUN_SPEED, RUN_SPEED);
			break;
		case 'S':
		case 's':
			if (leftMotorSpeed != 0 && rightMotorSpeed != 0)
			{
				SetMotorSpeeds(0, 0);
			}
			else
			{
				SetMotorSpeeds(-RUN_SPEED, -RUN_SPEED);
			}
			break;

		case 'A':
		case 'a':
			Turn(-1, 50);
			break;
		case 'D':
		case 'd':
			Turn(1, 50);
			break;
		case 'C':
		case 'c':
			runReflectanceArray = true;
			SetMotorSpeeds(RUN_SPEED, RUN_SPEED);
			break;

		case 'p':
		case 'P':
			run = !run;
			break;

		default:
			break;
		}
	}
}

void readStartStopInput()
{
	if (Serial.available()>0) {
		int input = Serial.read();
		switch (input)
		{
		case 'p':
		case 'P':
			run = !run;
			break;
		}
	}
}


//A function to handle turning the Zumo left or right
//Parameters : Pass it -1 for left, 1 for right. How long to turn for
void Turn(int direction, int delayMs)
{
	switch (direction)
	{
	case -1:
		motors.setSpeeds(-MAX_SPEED, MAX_SPEED);
		break;

	case 1:
		motors.setSpeeds(MAX_SPEED, -MAX_SPEED);
		break;
	}


	delay(delayMs);
	//motors.setSpeeds(RUN_SPEED, RUN_SPEED);
}

//Function to set speeds of both motors independantly
//Parameters : Left motor speed , Right motor speed
void SetMotorSpeeds(int pLeftSpeed, int pRightSpeed)
{
	ClampMotorSpeed(pLeftSpeed);
	ClampMotorSpeed(pRightSpeed);

	motors.setSpeeds(pLeftSpeed, pRightSpeed);
	leftMotorSpeed = pLeftSpeed;
	rightMotorSpeed = pRightSpeed;
}

//Function to set left motor speed only
//Parameters: Left motor speed
void SetLeftMotorSpeed(int pLeftSpeed)
{
	ClampMotorSpeed(pLeftSpeed);

	motors.setSpeeds(pLeftSpeed, rightMotorSpeed);
	leftMotorSpeed = pLeftSpeed;
}

//Function to set right motor speed only
//Parameters: Right motor speed
void SetRightMotorSpeed(int pRightSpeed)
{
	ClampMotorSpeed(pRightSpeed);

	motors.setSpeeds(leftMotorSpeed, pRightSpeed);
	rightMotorSpeed = pRightSpeed;
}

//Function to clamp any motor speed between it's maximum value (MAX_SPEED)
//Paramaters : Reference to speed being clamped
void ClampMotorSpeed(int& pSpeed)
{
	if (pSpeed > MAX_SPEED)
	{
		pSpeed = MAX_SPEED;
	}
	else if (pSpeed < -MAX_SPEED)
	{
		pSpeed = -MAX_SPEED;
	}
}

//Print out the sensor array data all in one line with tabs in between
void displayArrayData()
{
	for (int i = 0; i < NUM_SENSORS; i++)
	{
		Serial.print(sensorArray[i]);
		Serial.print("\t");
	}
	Serial.print("\n");
}
