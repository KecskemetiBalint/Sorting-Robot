#include <wiringPi.h>
#include <iostream>
#include <thread>
#include <cmath>
#include <stdint.h>
#include "../motor/Piston.cpp"
#include "../ADC/Adafruit_ADS1015.cpp"

using namespace std;

class IR_Sensor
{
	private: 
	
	Adafruit_ADS1115 ads;
	double upperOut = 1.8;
	double lowerOut = 0.2;
	
	public:
	Piston piston;
	
	// Constructor 	
	IR_Sensor()
	{
		ads.setGain(GAIN_ONE);
		ads.begin();
		initIR();
	}

	// Computes the current input and returns it
	double getIR(uint8_t l)
	{
		//int arr[]={9,8};
		
		
		double adc;
		adc = ads.readADC_SingleEnded(l);
		adc -= 8000.0;
		adc = (double) adc / 10000.0;
		return adc;
		
		//if(digitalRead(arr[l]))
			//return 1.9;
		//else
			//return 0.7;
	}
	
	// Detects is a disk is in front of the IR Sensor l, depending on the output range
	// Since the IR sensors have a LED, if the light level is below a certain threashold, then something (a disk) is blocking the LED
	void detectDisk(uint8_t l) 
	{
		while (true)
		{
			double level = getIR(l);
			if (lowerOut < level && level < upperOut) 
			{
				if (l == 1)
				{
					Logger::getInstance().log(INFO, "A disk entered the system or a black disk exited the system");
					Piston::counter--;
					piston.SortDisk(0);
				}
				else 
				{
					Logger::getInstance().log(INFO, "A disk entered the system");
					Piston::counter += 2;
				}
			}
			delay(1);
		}
	}
	
	inline void initIR()
	{
		Logger::getInstance().log(INFO, "Initialising the IR sensors");
		std::thread s0([this]{ detectDisk(0); });
        std::thread s1([this]{ detectDisk(1); });
		s0.detach();
		s1.detach();
	}
};
