#include <wiringPi.h>
#include <iostream>
#include <algorithm>
#include <stdint.h>
#include "MotorManager.cpp"
//#include "../Logs/Logger.cpp"

using namespace std;

class Motor
{
	public:
	uint8_t mot;
	
	Motor() 
	{
		//logger.init("../Logs/logfile.txt");
	}

	void init(uint8_t motID)
	{
		mot = motID;
	}
	
	void changeDir(uint8_t pinA, uint8_t pinB, uint8_t dir)
	{
		if (dir == FORWARD)
		{
			digitalWrite(pinA,HIGH);
			digitalWrite(pinB,LOW);
		}
		else if (dir == BACKWARD)
		{
			digitalWrite(pinA,LOW);
			digitalWrite(pinB,HIGH);
		}
		else if (dir == STOP)
		{
			digitalWrite(pinA,LOW);
			digitalWrite(pinB,LOW);
		}
		
	}

	void setDir(int8_t dir)
	{
		if (mot == BELT_1) changeDir(MOT1a, MOT1b, dir);
		else if (mot == BELT_2) changeDir(MOT2a, MOT2b, dir);
		else if (mot == PISTON) changeDir(MOT3a, MOT3b, dir);
		else if (mot == DISK) changeDir(MOT4a, MOT4b, dir);
	}
};
