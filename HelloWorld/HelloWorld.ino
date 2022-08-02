#include <avr/sleep.h>
void setup()
{
	Serial.begin(115200);
	pinMode(13, OUTPUT);
}

void loop()
{
	Serial.println("Hello World");
	digitalWrite(13, HIGH);
    delay(100);
    digitalWrite(13, LOW);
	delay(100);
	noInterrupts();
   	sleep_enable();
    sleep_cpu();
}


