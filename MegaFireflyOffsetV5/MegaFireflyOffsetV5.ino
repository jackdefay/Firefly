//MegaFireflyOffsetV5.ino

#define PERIOD 2000  //the duration of the blink in milliseconds
//#define OFFSET 400  //the initial wait time for the before the start of the algorithm - OLD, only use this if need to control the initial offset value
//#define MAXDIVISOR 10  //the max divisor value if one uses random divisors, would make for an interesting test
#define DIVISOR 2

long led = 13;  //declare the pin for the led
long button = 2;  //declare the pin for the start button, will only do something on a single firefly
long syncButtonOn = 3;
long syncButtonOff = 4;

long mod = 0;  //this variable will be used to shift the temporal position of the blinking

long iteration1 = 1;  //keeps track of the number of times a value has been stored in the array. Also keeps track of the current position in the array for calculations
long iteration2 = 1;
long iteration3 = 1;
long myIteration = 1;  //keeps track of the same information, but for self

long offset1 = 0;  //stores the most recent offset value, used for calculations, calculated in the function updateOffset
long offset2 = 0;
long offset3 = 0;

long avgOffset = 0;  //the average offset value calculated in the function updateAvg

long input1[100][3];  //an array that logs a bunch of useful data for people to use
long input2[100][3];  //stores the time that a state change has occured, the new state, and the offset values
long input3[100][3];
long myInput[100][3];

long ledState = LOW;  //an intermediate variable used in the timeToBlink to set the led state
long previousMillis = 0;  //a variable that keeps track of the last time the led blinked, in order to properly space the next blink in timeToBlink
//long previousMillis2 = 0;  //used for the same perpose, but in the shiftMod function

bool startRelay = false;  //boolean variable that lets the program know when it has started based on the propagating start message
bool thisFireflyHasStarted = false;

bool syncRelayOn = false;
bool syncRelayOff = false;
bool inSynchronizingMode = false;

void setup() {

    pinMode(led, OUTPUT);
    pinMode(button, INPUT); //initiates the button as an input, again, will only function on a single firefly
    pinMode(syncButtonOn, INPUT);
    pinMode(syncButtonOff, INPUT);

    for(int i=0; i<100	; i++){  //zeros the arrays
    	for(int j=0; j<3; j++){
  		    input1[i][j] = 0;
  		    input2[i][j] = 0;
  	    	input3[i][j] = 0;
  		    myInput[i][j] = 0;
    	}
    }

    randomSeed(analogRead(0));  //uses the natural noise of an unconnected pin as a randomizer, sets that noise as the randomizing factor to set the initial offset

    Serial.begin(115200);  //initiates the serial monitor
    Serial1.begin(115200);  //initiates the other 3 serial ports to listen to the connected "fireflies"
    Serial2.begin(115200);
    Serial3.begin(115200);

    Serial.println("*****************************************************");  //to signal the start of the program
}

void loop() {

	static long initialOffset = (long) random(PERIOD);  //randomizes the initial offset of the system, this fuction was previously done by "OFFSET"
	//staic long initialOffset = OFFSET;  //old strat, leave commented out unless want to control the initial offset
	static long divisor = DIVISOR;  //sets the divisor value, which is then passed into the shiftMod callbackFunction
	//static long divisor = (long) random(MAXDIVISOR);  //where the divisor value is calculated if random divisors are used

	long systemTime = (long) millis();  //takes the time at the beginning of each loop of "void loop()" to pass to the funcions, so every function uses the same time each loop

	ifButtonsHaveBeenPressed(initialOffset, systemTime);

	if((mod > PERIOD) || (mod < (-1*PERIOD))) Serial.println("MOD IS SPIRALLING OUT OF CONTROL!");  //a debugging check that prints whenever the mod value overtakes the period, which should never be necessary if the initial offset values are less that the period

	checkPort1(initialOffset, systemTime);  //listen to the three ports on loop
	checkPort2(initialOffset, systemTime);
	checkPort3(initialOffset, systemTime);

	timeToBlink(systemTime);//blinks if it is time to

	calcOffset1();  //calculates the offset of each connected firefly to self
	calcOffset2();
	calcOffset3();

	updateAvg();//update the average value with new data

	shiftMod(divisor);//shift the wavelength based on the mod variable

	//if times out, then use last stored values...or overwrite them...?
}

void checkPort1(long initialOffset, long systemTime){
	if(Serial1.available()){  //only reads from the port if there is available data in the buffer
		long readValue = (long) Serial1.read();  //reads the first value stored in the serial buffer

		if(readValue==2){  //an if case that relays the start command to all connected fireflies the first time it recieves the start command itself
			startRelay = true;  //finally sets the startRelay variable to true, initiating its other processes and locking down the loop with the (startbutton==false) if case
		}

		else if(readValue==3){
			syncRelayOn = true;
		}

		else if(readValue==4){
			syncRelayOff = true;
		}

		else if((readValue != input1[iteration1-1][1]) && (thisFireflyHasStarted)){  //if the reieved value is distinct from the last recieved value, and the program has "started" from recieving a 0
			input1[iteration1][0] = (long) systemTime;  //then sets the first row value of the array to the current time
			input1[iteration1][1] = (long) readValue;  //and the second row value to the new state of the led
			iteration1++;  //increases the tally for the number of datapoints logged
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
			syncRelayOn = true;
		}

		else if(readValue==4){
			syncRelayOff = true;
		}

		else if((readValue != input2[iteration2-1][1]) && (thisFireflyHasStarted)){  //if the reieved value is distinct from the last recieved value, and the program has "started" from recieving a 0
			input2[iteration2][0] = (long) systemTime;  //then sets the first row value of the array to the current time
			input2[iteration2][1] = (long) readValue;  //and the second row value to the new state of the led
			iteration2++;  //increases the tally for the number of datapoints logged
		}
	}
}

void checkPort3(long initialOffset, long systemTime){
	if(Serial3.available()){  //only reads from the port if there is available data in the buffer
		long readValue = (long) Serial3.read();  //reads the first value stored in the serial buffer

		if(readValue==2){  //an if case that relays the start command to all connected fireflies the first time it recieves the start command itself
			startRelay= true;  //finally sets the startRelay variable to true, initiating its other processes and locking down the loop with the (startbutton==false) if case
		}

		else if(readValue==3){
			syncRelayOn = true;
		}

		else if(readValue==4){
			syncRelayOff = true;
		}

		else if((readValue != input3[iteration3-1][1]) && (thisFireflyHasStarted)){  //if the reieved value is distinct from the last recieved value, and the program has "started" from recieving a 0
			input3[iteration3][0] = (long) systemTime;  //then sets the first row value of the array to the current time
			input3[iteration3][1] = (long) readValue;  //and the second row value to the new state of the led
			iteration3++;  //increases the tally for the number of datapoints logged
		}
	}
}

void timeToBlink(long systemTime){
	long currentMillis = (long) systemTime;  //allows the function to compare the current time to the time recorded in previousMillis

	if(((currentMillis - previousMillis) >= (PERIOD + mod)) && (thisFireflyHasStarted)){  //if the difference in time between the previousMillis and current time, is greater than or equal to the period of the firefly plus its modifier value; and the program has "started"
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

		myInput[myIteration][0] = (long) systemTime;  //logs data in the same format as the other arrays, but for self
		myInput[myIteration][1] = (long) valueToSend;
		myIteration++;
	}
}

void calcOffset1(){
	if(iteration1 > 3){
		long commonIteration = (long) min(iteration1, myIteration);  //finds the least common itteration of connected firelfy1 and self, in order to find the most recent data point that may be used in calculation

		offset1 = (long) (input1[commonIteration-1][0] - myInput[commonIteration-1][0]);
		//input1[commonIteration-1][2] = offset1;  //logs the offset value in the third row of the array
	}
}

void calcOffset2(){
	if(iteration2 > 3){
		long commonIteration = (long) min(iteration2, myIteration);  //finds the least common itteration of connected firelfy1 and self, in order to find the most recent data point that may be used in calculation

		offset2 = (long) (input2[commonIteration-1][0] - myInput[commonIteration-1][0]);
		//input2[commonIteration-1][2] = offset2;  //logs the offset value in the third row of the array
	}
}

void calcOffset3(){
	if(iteration3 > 3){
		long commonIteration = (long) min(iteration3, myIteration);  //finds the least common itteration of connected firelfy1 and self, in order to find the most recent data point that may be used in calculation

		offset3 = (long) (input3[commonIteration-1][0] - myInput[commonIteration-1][0]);
		//input3[commonIteration-1][2] = offset3;  //logs the offset value in the third row of the array
	}
}

void updateAvg(){
	long tempSum = 0;  //a temporary variable for the sum of the values
	long numberOn = 0;  //another temporary variable to store the number of "online" fireflies for the calculation of the average

	if(iteration1 > 3){  //ensures that the firefly is "alive"
		tempSum += (long) offset1;  //adds the most recently calculated offset value
		numberOn++;  //adds one to the tally of how many values are being averaged, in order to divide by the correct number
	}

	if(iteration2 > 3){
		tempSum += (long) offset2;
		numberOn++;
	}

	if(iteration3 > 3){
		tempSum += (long) offset3;
		numberOn++;
	}

	if((avgOffset != ((long) (tempSum/numberOn))) && ((iteration1>3) || (iteration2>3) || (iteration3>3))){
		Serial.print(millis());  //outputs the millisecond time, the three offset values, and the average offset
		Serial.print(",");		 //remember self has an offset of 0, so the variance from the mean can still be determined
		Serial.print(offset1);	 //next copy the serial monitor data to a text file in comma format
		Serial.print(",");		 //then upload that text file to a google sheet to interpret and graph the data
		Serial.print(offset2);
		Serial.print(",");
		Serial.print(offset3);
		Serial.print(",");
		Serial.println(avgOffset);
	}

	if(numberOn > 0) avgOffset = (long) tempSum/numberOn;  //calculates the average, based on the sum variable and the number of fireflies variable
}

void shiftMod(long divisor){
	if(inSynchronizingMode) mod = (long) (avgOffset/divisor);

	//if(mod==0  && iteration1>3 && iteration2>3 && iteration3>3) Serial.println("SYNCHRONIZED!");
}

void ifButtonsHaveBeenPressed(long initialOffset, long systemTime){
	if(((digitalRead(button) == HIGH) || (startRelay)) && (thisFireflyHasStarted == false)){  //if the firfly reads that the connected button has been pressed, and this firefly is a designated "button person" firefly, then it relays the start signal to all the connected firelfies
		Serial1.write(2);  //the number 2 is the designated start signal
		Serial2.write(2);
		Serial3.write(2);

		delay(PERIOD);  //waits the set period length
		previousMillis = (long) systemTime;  //resets the previousMillis and previousMillis2 variables to prevent things from piling up
		Serial.println("started");  //gives an indicator in the serial monitor

		thisFireflyHasStarted = true;

		delay(initialOffset);  //delays for an additional time for offset. this is how the offset variable is introduced into the system

		startRelay = false;
	}

	/*if(((digitalRead(syncButtonOn) == HIGH) || (syncRelayOn) ) && (inSynchronizingMode == false)){
		Serial1.write(3);
		Serial2.write(3);
		Serial3.write(3);

		Serial.println("This firefly is going to wait for the duration of one Period before starting to synchronize");
		//delay(PERIOD);
		//previousMillis = (long) systemTime;

		inSynchronizingMode = true;
		syncRelayOn = false;

		Serial1.end();
		Serial2.end();
		Serial3.end();
		Serial1.begin(115200);
		Serial2.begin(115200);
		Serial3.begin(115200);
	}*/

	/*if(((digitalRead(syncButtonOff) == HIGH) || (syncRelayOff)) && (inSynchronizingMode == true)){
		Serial1.write(4);
		Serial2.write(4);
		Serial3.write(4);

		Serial.println("This firefly is going to wait for the duration of one Period before resuming without synchronizing");
		delay(PERIOD);
		previousMillis = (long) systemTime;

		inSynchronizingMode = false;
		syncRelayOff = false;

		Serial1.end();
		Serial2.end();
		Serial3.end();
		Serial1.begin(115200);
		Serial2.begin(115200);
		Serial3.begin(115200);
	}*/
}