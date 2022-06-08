#include "EMailSenderKey.h"
#include <EMailSender.h>
#include <DallasTemperature.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <SPI.h>
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte ip[] = {0, 0, 0, 0};             // ip in lan assigned to arduino

EthernetServer server(80);
/*-----( Declare Constants and Pin Numbers )-----*/
#define ONE_WIRE_BUS_PIN 2

/*-----( Declare objects )-----*/
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS_PIN);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// Email instance
EMailSender emailSend("test@test.com", "password");

/*-----( Declare Variables )-----*/
// Assign the addresses of your 1-Wire temp sensors.

#define ALERT_TEMPERATURE 50

DeviceAddress probes[] = {
		{0x28, 0xD9, 0x78, 0xC3, 0x06, 0x00, 0x00, 0x7F},
		{0x28, 0xFE, 0x3F, 0xC5, 0x06, 0x00, 0x00, 0xFD},
		{0x28, 0xC9, 0x51, 0xC4, 0x06, 0x00, 0x00, 0xA9},
		{0x28, 0x0E, 0x7D, 0xC5, 0x06, 0x00, 0x00, 0xE5},
		{0x28, 0xE4, 0x4C, 0xC4, 0x06, 0x00, 0x00, 0xC5},
		{0x28, 0xC7, 0xBC, 0xFE, 0x05, 0x00, 0x00, 0x3B},
		{0x28, 0xDA, 0xD4, 0xC4, 0x06, 0x00, 0x00, 0xB2},
		{0x28, 0xC6, 0x56, 0xC4, 0x06, 0x00, 0x00, 0xDC},
		{0x28, 0x4D, 0xCA, 0xC4, 0x06, 0x00, 0x00, 0x58},
		{0x28, 0x7F, 0xAE, 0xC4, 0x06, 0x00, 0x00, 0xD5},
		{0x28, 0x19, 0xCB, 0xC3, 0x06, 0x00, 0x00, 0xE1},
		{0x28, 0x70, 0x4B, 0xC3, 0x06, 0x00, 0x00, 0x7F},
		{0x28, 0xE9, 0x9B, 0xC4, 0x06, 0x00, 0x00, 0x80},
		{0x28, 0xFE, 0xCE, 0xC3, 0x06, 0x00, 0x00, 0x9F},
		{0x28, 0xCC, 0x5D, 0xC4, 0x06, 0x00, 0x00, 0x63},
		{0x28, 0x29, 0x88, 0xC4, 0x06, 0x00, 0x00, 0x2D},
		{0x28, 0x2A, 0x0D, 0xC4, 0x06, 0x00, 0x00, 0x6D},
		{0x28, 0x90, 0xA5, 0xC4, 0x06, 0x00, 0x00, 0xA8},
		{0x28, 0xAE, 0xC7, 0xC3, 0x06, 0x00, 0x00, 0x42},
		{0x28, 0xD9, 0x07, 0xFD, 0x05, 0x00, 0x00, 0xC4},
};

void setup()
{
	// start serial port to show results
	Serial.begin(115200);
	Serial.println(F("Init Temperature Control"));

	// Initialize the Temperature measurement library
	sensors.begin();

	// set the resolution to 10 bit (Can be 9 to 12 bits .. lower is faster)
	for (int i = 0; i < sizeof(probes) / sizeof(probes[0]); i++) {
		sensors.setResolution(probes[i], 10);
	}

	Serial.print(F("Initialize Eth... "));
	Ethernet.begin(mac, ip);
	server.begin();
	Serial.println(Ethernet.localIP());
}


void loop() /****** LOOP: RUNS CONSTANTLY ******/
{
	delay(1000);
	Serial.println();
	Serial.print("Number of Devices found on bus = ");
	Serial.println(sensors.getDeviceCount());
	Serial.print("Getting temperatures... ");
	Serial.println();

	// Command all devices on bus to read temperature
	sensors.requestTemperatures();

	Serial.print("Probe 20 temperature is:   ");
	printTemperature(probes[19]);
	Serial.println();

	
	long sensors_over_limit = 0;
	for (int i = 0; i < sizeof(probes) / sizeof(probes[0]); i++) {
		if (sensors.getTempC(probes[i]) > ALERT_TEMPERATURE) {
			sensors_over_limit |= 1 << i;
		}
	}

	if (sensors_over_limit ) {
		send_email(sensors_over_limit);
	}
}

void send_email(long sensors_over_limit) {
	static int last_send = 0;
	if (millis() > last_send + 5 * 60 * 1000) {
		last_send = millis();
		EMailSender::EMailMessage message;
		message.subject = "ALARM";
		message.message = "Sensors failed: ";
		message.message += String(sensors_over_limit);

		EMailSender::Response resp = emailSend.send("email_to_receive@gmail.com", message);

		Serial.println("Sending status: ");

		Serial.println(resp.status);
		Serial.println(resp.code);
		Serial.println(resp.desc);
	}

}

/*-----( Declare User-written Functions )-----*/
void printTemperature(DeviceAddress deviceAddress) {
	float tempC = sensors.getTempC(deviceAddress);

	if (tempC == -127.00) {
		Serial.print("Error getting temperature  ");
	} else {
		Serial.print("C: ");
		Serial.print(tempC);
		Serial.print(" F: ");
		Serial.print(DallasTemperature::toFahrenheit(tempC));
	}

	// listen for incoming clients
	EthernetClient client = server.available();
	if (client) {
		Serial.println("new client");
		// an http request ends with a blank line
		boolean currentLineIsBlank = true;
		while (client.connected()) {
			if (client.available()) {
				char c = client.read();
				Serial.write(c);
				// if you've gotten to the end of the line (received a newline
				// character) and the line is blank, the http request has ended,
				// so you can send a reply
				if (c == '\n' && currentLineIsBlank) {
					// send a standard http response header
					client.println("HTTP/1.1 200 OK");
					client.println("Content-Type: text/html");
					client.println(
							"Connection: close"); // the connection will be closed after
																		// completion of the response
					//  client.println("Refresh: 5");  // refresh the page automatically
					//  every 5 sec
					client.println();
					client.println("<!DOCTYPE HTML>");
					client.println("<html>");
					client.println("<head>");

					client.println("<link href=\"https://xxxxx/arduino.css\" "
												 "rel=\"stylesheet\" type=\"text/css\"/>");

					client.println("</head>");
					client.println("<body>");
					client.println(" <table id=\"tableID\"> ");
					for (int i = 0; i < sizeof(probes) / sizeof(probes[0]); i++) {
						client.println("<tr>");
						client.print("<td>Probe: ");
						client.print(i);
						client.println("</td>");
						client.print("<td>");
						client.print(sensors.getTempC(probes[i]));
						client.println("</td>");
						client.println("</tr>");
					}

					client.println("</table>");
					client.println("<script type='text/javascript' "
												 "src='xxxxxxx/arduino.js'></script>");
					client.println("</body>");
					client.println("</html>");
					break;
				}
				if (c == '\n') {
					// you're starting a new line
					currentLineIsBlank = true;
				} else if (c != '\r') {
					// you've gotten a character on the current line
					currentLineIsBlank = false;
				}
			}
		}
		// give the web browser time to receive the data
		delay(1);
		// close the connection:
		client.stop();
		Serial.println("client disonnected");
	}

} // End printTemperature
	//*********( THE END )***********