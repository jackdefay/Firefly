//FireflyStrangerTestingV1.ino
//based off FireflyStrangerV2.ino
//now updated to current stable version

#define PERIOD 1600  //the duration of the blink in milliseconds
#define DIVISOR 1 //this is conversionMult now

long led = 13;  //declare the pin for the led
long button = 2;  //declare the pin for the start button, will only do something on a single firefly
long noiseReductionSwitch = 3;

long ledState = LOW;  //an intermediate variable used in the timeToBlink to set the led state
long previousMillis = 0;  //a variable that keeps track of the last time the led blinked, in order to properly space the next blink in timeToBlink

bool startRelay = false;  //boolean variable that tells the program to relay the start message
bool thisFireflyHasStarted = false;  //boolean that allows the firefly to recieve incoming serial communication that isn't a "2"
bool noiseReductionRelay = false;
bool noiseReduction = false;

bool DataCollected = false;
long lateststart = 0;

void setup() {

    pinMode(led, OUTPUT);
    pinMode(button, INPUT); //initiates the button as an input, again, will only function on a single firefly
    pinMode(noiseReductionSwitch, INPUT);

    randomSeed(analogRead(0));  //uses the natural noise of an unconnected pin as a randomizer, sets that noise as the randomizing factor to set the initial offset

    Serial.begin(115200);  //initiates the serial monitor
    Serial1.begin(115200);  //initiates the other 3 serial ports to listen to the connected "fireflies"
    Serial2.begin(115200);
    Serial3.begin(115200);

    //Serial.println("*****************************************************");  //to signal the start of the program
}

void loop() {

	static long initialOffset = (long) random(PERIOD);  //randomizes the initial offset of the system, this was previously done by "OFFSET"
	static double divisor = DIVISOR;  //sets the divisor value, which is then passed into the shiftMod function

	long systemTime = (long) millis();  //takes the time at the beginning of each loop of "void loop()" to pass to the funcions, so every function uses the same time each loop

	buttonCheck();  //a function that checks if the button has been pressed
	systemTime = relay(initialOffset, systemTime);  //relays the start signal if the boolean is true, and initiates the start process

	//if((mod > PERIOD) || (mod < (-1*PERIOD))) Serial.println("MOD IS SPIRALLING OUT OF CONTROL!");  //a debugging check that prints whenever the mod value overtakes the period, which should never be necessary if the initial offset values are less that the period

	checkPort1(initialOffset, systemTime);  //listen to the three ports on loop
	checkPort2(initialOffset, systemTime);
	checkPort3(initialOffset, systemTime);

	timeToBlink(systemTime);//blinks if it is time to

	if(DataCollected == true){
		Serial1.write(4);  //the number 3 is the designated restart signal
		Serial2.write(4);
		Serial3.write(4);

		digitalWrite(led, LOW);

		Serial1.end();
		Serial2.end(); 
		Serial3.end();

		Serial.print(divisor);
		Serial.print(", ");
		Serial.println(((long) (millis()) - lateststart));
		
		delay(4000);
		asm volatile ("  jmp 0");
	}
}

void checkPort1(long initialOffset, long systemTime){
	if(Serial1.available()){  //only reads from the port if there is available data in the buffer
		long readValue = (long) Serial1.read();  //reads the first value stored in the serial buffer

		if(readValue==2){  //an if case that relays the start command to all connected fireflies the first time it recieves the start command itself
			startRelay = true;  //finally sets the startRelay variable to true, initiating its other processes and locking down the loop with the (startbutton==false) if case
		}

		else if(readValue==3){
			noiseReductionRelay = true;
		}

		else if((readValue==4) && (DataCollected == false)){  //an if case that relays the start command to all connected fireflies the first time it recieves the start command itself
			DataCollected = true;
		}
	}
}

void checkPort2(long initialOffset, long systemTime){
	if(Serial2.available()){  //only reads from the port if there is available data in the buffer
		long readValue = (long) Serial2.read();  //reads the first value stored in the serial buffer

		if(readValue==2){  //an if case that relays the start command to all connected fireflies the first time it recieves the start command itself
			startRelay = true;  //finally sets the startRelay variable to true, initiating its other processes and locking down the loop with the (startbutton==false) if case
		}

		else if(readValue==3){
			noiseReductionRelay = true;
		}

		else if((readValue==4) && (DataCollected == false)){  //an if case that relays the start command to all connected fireflies the first time it recieves the start command itself
			DataCollected = true;
		}
	}
}

void checkPort3(long initialOffset, long systemTime){
	if(Serial3.available()){  //only reads from the port if there is available data in the buffer
		long readValue = (long) Serial3.read();  //reads the first value stored in the serial buffer

		if(readValue==2){  //an if case that relays the start command to all connected fireflies the first time it recieves the start command itself
			startRelay = true;  //finally sets the startRelay variable to true, initiating its other processes and locking down the loop with the (startbutton==false) if case
		}

		else if(readValue==3){
			noiseReductionRelay = true;
		}

		else if((readValue==4) && (DataCollected == false)){  //an if case that relays the start command to all connected fireflies the first time it recieves the start command itself
			DataCollected = true;
		}
	}
}

void timeToBlink(long systemTime){
	long currentMillis = (long) systemTime;  //allows the function to compare the current time to the time recorded in previousMillis

	//this next step is approximated to the nearest millisecond, even though the calulations may be more precise
	if(((currentMillis - previousMillis) >= ((long) PERIOD)) && (thisFireflyHasStarted)){  //if the difference in time between the previousMillis and current time, is greater than or equal to the period of the firefly plus its modifier value; and the program has "started"
		previousMillis = (long) currentMillis;  //if the criteria are met, then resets the previousMillis time to the current one, in order to prep for the next cycle
		long valueToSend = 0;  //initiates a intermediate variable for the value that will be sent across the serial ports to the other fireflies

		if(ledState == LOW){  //if the current state of the led is off
			valueToSend = 1;  //then tells the other fireflies that it is turning its led on
			ledState = HIGH;  //and sets its led to HIGH
		}
		else{  //if the firefly isn't off ==> it is on
			valueToSend = 0;  //then it tells the other firelfies it is turning its led on
			ledState = LOW;  //and turns its led on
		}

		digitalWrite(led, ledState);  //the actual action of turning the led on or off
		Serial1.write(valueToSend);  //the action of sending the new value
		Serial2.write(valueToSend);
		Serial3.write(valueToSend);
	}
}

void buttonCheck(){
	if((digitalRead(button) == HIGH) && (digitalRead(noiseReductionSwitch) == LOW)) startRelay = true;  //checks if the button is pressed
	else if((digitalRead(button) == HIGH) && (digitalRead(noiseReductionSwitch) == HIGH)) noiseReductionRelay = true;
}

long relay(long initialOffset, long systemTime){  //propogates the start signal throughout the swarm
	if((startRelay) && (thisFireflyHasStarted == false)){  //if the firfly reads that the connected button has been pressed, and this firefly is a designated "button person" firefly, then it relays the start signal to all the connected firelfies
		Serial1.write(2);  //the number 2 is the designated start signal
		Serial2.write(2);
		Serial3.write(2);

		//Serial.println("started");  //gives an indicator in the serial monitor
		//Serial.flush();
		
		delay(initialOffset);  //waits an additional time for offset. this is how the offset variable is introduced into the system

		thisFireflyHasStarted = true;  //sets the boolean that allows the firefly to recieve values from the serial ports that aren't a 2

		systemTime = (long) millis();
		previousMillis = (long) systemTime;  //resets the previousMillis variables to prevent things from piling up

		lateststart = millis();
	}

	else if((noiseReductionRelay) && (thisFireflyHasStarted == false) && (noiseReduction == false)){  //if the firfly reads that the connected button has been pressed, and this firefly is a designated "button person" firefly, then it relays the start signal to all the connected firelfies
		Serial1.write(3);  //the number 2 is the designated start signal
		Serial2.write(3);
		Serial3.write(3);

		//Serial.println("noise reduction activated");  //gives an indicator in the serial monitor
		//Serial.flush();
		
		delay(initialOffset);  //waits an additional time for offset. this is how the offset variable is introduced into the system

		thisFireflyHasStarted = true;  //sets the boolean that allows the firefly to recieve values from the serial ports that aren't a 2
		noiseReduction = true;

		systemTime = (long) millis();
		previousMillis = (long) systemTime;  //resets the previousMillis variables to prevent things from piling up

		lateststart = millis();

		//Serial.println("the switch was on");
	}

	return systemTime;
}
