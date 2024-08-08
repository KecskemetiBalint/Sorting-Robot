#include "RobotManager.cpp"

using namespace std;

IR_Sensor IRSensors;

int Piston::counter = 0;
int Piston::counterW = 0;
int Piston::counterB = 0;
int Piston::counterC = 0;

void detectBlock()
{
    int last, curr;
    int start, end;
    curr = digitalRead(SWITCH);
    last = 1 - curr;
    while(true)     
    {
        curr = digitalRead(SWITCH);
        if (curr != last) 
        {
            //Reset timer and start timer
            start = now();
        }
        else 
        {
            end = now();
            if ((end - start)>= 4000) 
            {
                Logger::getInstance().log(CRITICAL, "The rotating disk is blocked in some way! Fix it before continuing");
                delay(5000);
            }
        }
        last = curr;
    }
}

void Initialise()
{
    motInit();
	// Set diraction
    setDir(BELT_1, FORWARD);
	setDir(BELT_2, FORWARD);
	setDir(PISTON, STOP);
	setDir(DISK, FORWARD);
    // Initialise speed
    initSpeed();
    // Initialise camera
    CamInit(0);
    // Fault detection
    std::thread a(detectBlock);
    a.detach();
}

int main()
{
    int exit;
	Initialise();
    while(true)     
    {
        cin >> exit;
        if (exit == 1) 
        {
            stop();
            Logger::getInstance().log(INFO, to_string(Piston::counter) + " enetered the system");
            Logger::getInstance().log(INFO, "We should have " + to_string(Piston::counterW) + " white disks");
            Logger::getInstance().log(INFO, "We should have " + to_string(Piston::counterB) + " black disks");
            Logger::getInstance().log(INFO, "We should have " + to_string(Piston::counterC) + " coloured disks");
            break;
        }
    }
}   	
