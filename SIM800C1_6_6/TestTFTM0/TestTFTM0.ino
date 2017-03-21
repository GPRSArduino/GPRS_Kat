



#if defined(ARDUINO_SAMD_ZERO) && defined(SERIAL_PORT_USBVIRTUAL)
// Required for Serial on Zero based boards
#define Serial SERIAL_PORT_USBVIRTUAL
#endif


const char str[] = "My very long string";
extern "C" char *sbrk(int i);







int count = 0;
// constants won't change. Used here to set a pin number :
const int ledPin =  LED_BUILTIN;// the number of the LED pin

// Variables will change :
int ledState = LOW;             // ledState used to set the LED

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change :
const long interval = 1000;           // interval at which to blink (milliseconds)

int FreeRam () {
	char stack_dummy = 0;
	return &stack_dummy - sbrk(0);
}

void setup()
{
	Serial.begin(115200);
	delay(1000);
	
	Serial.print( "FreeRam");
	Serial.println( FreeRam ());
	Serial.print("Address of str $"); Serial.println((int)&str, HEX);
	Serial.println("\r\n==================");
	
}

void loop()
{

	unsigned long currentMillis = millis();

	if (currentMillis - previousMillis >= interval) {
		// save the last time you blinked the LED
		previousMillis = currentMillis;
		Serial.print( "count = ");
		Serial.println(count);
		
		count++;



		// if the LED is off turn it on and vice-versa:
		if (ledState == LOW) {
			ledState = HIGH;
			} else {
			ledState = LOW;
		}

		// set the LED with the ledState of the variable:
		digitalWrite(ledPin, ledState);
	}

	
}
