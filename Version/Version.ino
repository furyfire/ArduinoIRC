#include "Version.h"

const 
Version ver(0,1,0);

void setup()
{
	Serial.begin(9600);
	Serial.println("Powerup");
}

void loop()
{
	delay(5000);
	Serial.println("heartbeat");
}