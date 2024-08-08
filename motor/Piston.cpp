#include <iostream>
#include <unistd.h>
#include <thread>
#include <stdexcept>    
#include "MotorManager.cpp"

using namespace std;

struct PistonException : public exception {
   const char * what () const throw () {
      return "The piston is unavailble. It's busy with another task at the moment.";
   }
};

class Piston
{
    private:

    bool active;
    int index;
    int ID;

    void Pull() 
    {
        if (ID == 2) delay(500);
        setDir(PISTON, BACKWARD);
        delay(600);
        setDir(PISTON, STOP);
        Logger::getInstance().log(INFO, "The piston finished pulling and starts pushing"); 
    }

    void Push() 
    {
        setDir(PISTON, FORWARD);
        delay(500);
        setDir(PISTON, STOP);
        Logger::getInstance().log(INFO, "The piston finished pushing"); 
    }

    public:
    static int counter;
    static int counterW;
    static int counterB;
    static int counterC;

    Piston()
    {
        index = 2;
        active = false;
        ID = -1;
    }
    
    bool pistonIsActive()
    {
        return active;
    }

    void SortDisk(int colourID)
    {
        // Colour check:
        if (colourID == 0) // The disk is black, so the piston does nothing
        {
            digitalWrite(LED_W,HIGH);
            digitalWrite(LED_B,HIGH);
            Logger::getInstance().log(INFO, "The disk is black, so the piston does nothing");
            counterB++;
            delay(2000);
            digitalWrite(LED_W,LOW);
            digitalWrite(LED_B,LOW);
            return;
        }
        if (active)
        {
            Logger::getInstance().log(ERROR, "The piston is unavailble. It's busy with another task at the moment"); 
            throw PistonException();
        }
        active = true;
        ID = colourID;
        if (colourID == 1) // White disk, so we push
        {
            digitalWrite(LED_W,HIGH);
            digitalWrite(LED_B,LOW);
            Logger::getInstance().log(INFO, "The disk is white, so the piston should push it");
            counterW++;
        }
        else               // Coloured disk, so we pull
        {
            digitalWrite(LED_W,LOW);
            digitalWrite(LED_B,HIGH);
            Logger::getInstance().log(INFO, "The disk is neither black nor white, so the piston should pull it"); 
            counterC++;
        }
        Logger::getInstance().log(INFO, "The piston starts pulling"); 
        Pull(); // Pull the piston. We move the piston from it's idle position
        Push(); // Push the piston. We move the piston back to it's idle position
        active = false;
        Logger::getInstance().log(INFO, "The disk is supposed to be correctly sorted"); 
        digitalWrite(LED_W,LOW);
        digitalWrite(LED_B,LOW);
    }

};

