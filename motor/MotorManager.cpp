#include <thread>
#include <stdint.h>
#include <wiringPi.h>
#include "../Logs/Logger.cpp"

#define MOT1a 28 // Conveyor belt 1
#define MOT1b 27
#define MOT1PWM 29
#define MOT2a 16 // Conveyor belt 2
#define MOT2b 1
#define MOT2PWM 15
#define MOT3a 4 // Piston
#define MOT3b 5
#define MOT3PWM 6
#define MOT4a 26 // Rotating Disk
#define MOT4b 11
#define MOT4PWM 10
#define LED_W 25 
#define LED_B 24
#define SWITCH 23

// defining constants
#define FORWARD 2  // 0b10
#define BACKWARD 1 // 0b01
#define STOP 0     // 0b00

// defining identifiers
#ifndef BELT_1
    #define BELT_1 1
#endif

#ifndef BELT_2
    #define BELT_2 2
#endif

#ifndef PISTON
    #define PISTON 3
#endif

#ifndef DISK
    #define DISK 4
#endif

double speed[4] = {3, 6, 4, 3};
//double speed[4] = {0, 0, 0, 0};

void pwm(int pin, int index)
{	
	while(true) 
	{
		digitalWrite(pin,HIGH);
		delay(speed[index]);
		digitalWrite(pin,LOW);
		delay(10 - speed[index]);
	}
}

inline void initSpeed()
{
	Logger::getInstance().log(INFO, "Initialising the speed of motors");
	std::thread l(pwm,MOT1PWM,0);
	std::thread p(pwm,MOT2PWM,1);
	std::thread k(pwm,MOT3PWM,2);
	std::thread m(pwm,MOT4PWM,3);
	l.detach();
	p.detach();
	k.detach();
	m.detach();
}

void motInit()
{
	// Initialise logger
	Logger::getInstance().log(INFO, "Logger was initilised. Starting general initialisation.");
	// Initialize WiringPi
	wiringPiSetup();
	Logger::getInstance().log(INFO, "WiringPi initialisation SUCCESSFUL! Initialising pins");
	// Set the pin mode to output
	pinMode(MOT1a,OUTPUT);
	pinMode(MOT1b,OUTPUT);
	pinMode(MOT1PWM,OUTPUT);
	pinMode(MOT2a,OUTPUT);
	pinMode(MOT2b,OUTPUT);
	pinMode(MOT2PWM,OUTPUT);
	pinMode(MOT3a,OUTPUT);
	pinMode(MOT3b,OUTPUT);
	pinMode(MOT3PWM,OUTPUT);
	pinMode(MOT4a,OUTPUT);
	pinMode(MOT4b,OUTPUT);
	pinMode(MOT4PWM,OUTPUT);
	pinMode(LED_W,OUTPUT);
	pinMode(LED_B,OUTPUT);
	Logger::getInstance().log(INFO, "Motor pins initialised successfully!");
	// Turn off the LEDs
	digitalWrite(LED_W,LOW);
	digitalWrite(LED_B,LOW);
}

void setSpeed(int index, uint8_t s)
{
	Logger::getInstance().log(INFO, "Setting speed " + to_string(s) + " to motor no. " + to_string(index + 1));
	speed[index] = s;
}

void changeDir(uint8_t mot, uint8_t pinA, uint8_t pinB, uint8_t dir)
{
	if (dir == FORWARD)
	{
		Logger::getInstance().log(INFO, "Changing direction of motor no. " + to_string(mot) + " to FORWARD");
		digitalWrite(pinA,HIGH);
		digitalWrite(pinB,LOW);
	}
	else if (dir == BACKWARD)
	{
		Logger::getInstance().log(INFO, "Changing direction of motor no. " + to_string(mot) + " to BACKWARD");
		digitalWrite(pinA,LOW);
		digitalWrite(pinB,HIGH);
	}
	else if (dir == STOP)
	{
		Logger::getInstance().log(INFO, "Changing direction of motor no. " + to_string(mot) + " to STOP");
		digitalWrite(pinA,LOW);
		digitalWrite(pinB,LOW);
	}
	
}

void setDir(uint8_t mot, uint8_t dir)
{
	if (mot == BELT_1) changeDir(mot, MOT1a, MOT1b, dir);
	else if (mot == BELT_2) changeDir(mot, MOT2a, MOT2b, dir);
	else if (mot == PISTON) changeDir(mot, MOT3a, MOT3b, dir);
	else if (mot == DISK) changeDir(mot, MOT4a, MOT4b, dir);
}

void stop()
{
	Logger::getInstance().log(INFO, "Stopping all motors!");
	setDir(BELT_1, STOP);
	setDir(BELT_2, STOP);
	setDir(PISTON, STOP);
	setDir(DISK, STOP);
}
