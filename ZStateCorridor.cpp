#include "ZStateCorridor.h"

ZStateCorridor::ZStateCorridor()
{
}

ZStateCorridor::~ZStateCorridor()
{
}

//Purely virtual function for initialising state
void ZStateCorridor::InitState()
{
	//Make sure state isn't already finished
	m_bStateFinished = false;

	//Make sure state isn't waiting for state change input
	m_bWaitForStateChange = false;

	//Get reference to the reflectance array instance
	m_reflectanceArray = ReflectanceArrayClass::GetReflectanceArrayInstance();

	//Set motors to move robot forward
	m_motors.SetMotorSpeeds(RUN_SPEED, RUN_SPEED);

	//Get the current elapsed time
	m_fStartTime = millis();
}

//Purely virtual function for updating state (tick)
void ZStateCorridor::UpdateState()
{
	if (!m_bWaitForStateChange)
	{
		//Check any collisions with the wall
		CheckWallCollision();
		//Check for any specific user input
		CheckUserInput();
	}
	else
	{
		CheckStateChangeInput();
	}
	
}

//Purely virtual function for stopping state
void ZStateCorridor::StopState()
{
}

//Uses the reflectance array to check any wall collision
//and moves out the way
void ZStateCorridor::CheckWallCollision()
{
	//Get reflectance hit data from our reflectance array
	ReflectanceData hitData = m_reflectanceArray.HandleReflectanceArray();

	//If the reflectance array has detected a hit
	if (hitData.m_bHit == true)
	{
		//If we've hit a wall head on
		if (hitData.m_iSensorsHit > 1 && hitData.m_iDirection == 0)
		{
			//Stop the motors
			m_motors.SetMotorSpeeds(0, 0);

			//End the state and move onto USER state
			m_bStateFinished = true;
			m_eNextState = ZUMO_STATES::USER;

			//Building data stuff
			{
				//Get finishing time
				m_fFinishTime = millis();
				CalculateCorridorLength();
			}
			
		}
		//Otherwise if we've just clipped the side of a wall
		else if (hitData.m_iSensorsHit == 1)
		{
			//If we've hit something on the left, move to the right
			if (hitData.m_iDirection == -1)
			{
				m_motors.Turn(1, 30, true);
			}
			//If we've hit something on the right, move to the left
			else if (hitData.m_iDirection == 1)
			{
				m_motors.Turn(-1, 30, true);
			}
		}

	}
}

//Checks for certain key input
void ZStateCorridor::CheckUserInput()
{
	//If spacebar key is pressed or 's'
	if (InputManagerClass::IsKeyPressed(32) || InputManagerClass::IsKeyPressed('s'))
	{
		//Stop the robot
		m_motors.SetMotorSpeeds(0, 0);

		//Get finishing time
		m_fFinishTime = millis();
		
		//Trigger state to wait for key press
		m_bWaitForStateChange = true;

		SPRINT(Stopping corridor behaviour.);
		SPRINT(new [C]orridor or new[R]oom ? );
	}
}

//Checks for player input to change states
void ZStateCorridor::CheckStateChangeInput()
{
	if (InputManagerClass::IsKeyPressed('C'))
	{
		//Calculate length of corridor and store it in current corridor
		CalculateCorridorLength();

		//Move into user state
		m_eNextState = ZUMO_STATES::USER;
		m_bStateFinished = true;
	}
	if (InputManagerClass::IsKeyPressed('R'))
	{
		//Calculate overall time
		float overallTime = m_fFinishTime - m_fStartTime;
		//Add a new room on this corridor, setting the time
		m_pBuildingData->GetCurrentCorridor()->AddRoom(overallTime, DIRECTION::INVALID);

		//Move into user state
		m_eNextState = ZUMO_STATES::USER;
		m_bStateFinished = true;
	}
}

//Calculates the time in which it took to traverse a corridor
void ZStateCorridor::CalculateCorridorLength()
{
	float currentCorridorTime = m_fFinishTime - m_fStartTime;

	float overallCorridorTime = currentCorridorTime;

	//Get current corridor
	Corridor* corridor = m_pBuildingData->GetCurrentCorridor();
	
	//Loop through all rooms
	for (int i = 0; i < corridor->m_iNumRooms; i++)
	{
		//Add the room time onto the overall time
		overallCorridorTime += corridor->GetRoom(i)->m_fTimeDownCorridor;
	}


	//Set approximate time in building data
	m_pBuildingData->GetCurrentCorridor()->m_fApproxLength = overallCorridorTime;

	SPRINT(Approximate corridor time : );
	Serial.print(overallCorridorTime / 1000);
	Serial.print(" for corridor : ");
	Serial.print(m_pBuildingData->m_iCurrentCorridor);
}

