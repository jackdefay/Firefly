//FireflyDualAlgorithmV1.ino
//based off FireflyV7 and FireflyNoiseResistantV2
//now updated to current stable version

#define PERIOD 2000  //the duration of the blink in milliseconds
#define DIVISOR 1.75 //this is conversionMult now

long led = 13;  //declare the pin for the led
long button = 2;  //declare the pin for the start button, will only do something on a single firefly
long noiseReductionSwitch = 3;

double mod = 0;  //this variable will be used to shift the temporal position of the blinking

long iteration1 = 1;  //keeps track of the number of times a value has been stored in the array. Also keeps track of the current position in the array for calculations
long iteration2 = 1;
long iteration3 = 1;
long myIteration = 1;  //keeps track of the same information, but for self

long offset1 = 0;  //stores the most recent offset value, used for calculations, calculated in the function updateOffset
long offset2 = 0;
long offset3 = 0;

double avgOffset = 0;  //the average offset value calculated in the function updateAvg

long input1[100][3];  //an array that logs a bunch of useful data for people to use
long input2[100][3];  //stores the time that a state change has occured, the new state, and the offset values
long input3[100][3];
long myInput[100][3];

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

    //Serial.println("*****************************************************");  //to signal the start of the program
}

void loop() {

	static long initialOffset = (long) random(PERIOD);  //randomizes the initial offset of the system, this was previously done by "OFFSET"
	static long divisor = DIVISOR;  //sets the divisor value, which is then passed into the shiftMod function

	long systemTime = (long) millis();  //takes the time at the beginning of each loop of "void loop()" to pass to the funcions, so every function uses the same time each loop

	buttonCheck();  //a function that checks if the button has been pressed
	systemTime = relay(initialOffset, systemTime);  //relays the start signal if the boolean is true, and initiates the start process

	//if((mod > PERIOD) || (mod < (-1*PERIOD))) Serial.println("MOD IS SPIRALLING OUT OF CONTROL!");  //a debugging check that prints whenever the mod value overtakes the period, which should never be necessary if the initial offset values are less that the period

	checkPort1(initialOffset, systemTime);  //listen to the three ports on loop
	checkPort2(initialOffset, systemTime);
	checkPort3(initialOffset, systemTime);

	timeToBlink(systemTime);//blinks if it is time to

	calcOffset1();  //calculates the offset of each connected firefly to self
	calcOffset2();
	calcOffset3();

	updateAvg();//update the average value with new data

	shiftMod(divisor);//shift the period based on the mod variable

	//if times out, then use last stored values...or overwrite them...?

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
			noiseReductionRelay = true;
		}

		else if((readValue==4) && (DataCollected == false)){  //an if case that relays the start command to all connected fireflies the first time it recieves the start command itself
			DataCollected = true;
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
			startRelay = true;  //finally sets the startRelay variable to true, initiating its other processes and locking down the loop with the (startbutton==false) if case
		}

		else if(readValue==3){
			noiseReductionRelay = true;
		}

		else if((readValue==4) && (DataCollected == false)){  //an if case that relays the start command to all connected fireflies the first time it recieves the start command itself
			DataCollected = true;
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

	//this next step is approximated to the nearest millisecond, even though the calulations may be more precise
	if(((currentMillis - previousMillis) >= ((long) (PERIOD + mod))) && (thisFireflyHasStarted)){  //if the difference in time between the previousMillis and current time, is greater than or equal to the period of the firefly plus its modifier value; and the program has "started"
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
	double tempSum = 0;  //a temporary variable for the sum of the values
	double numberOn = 0;  //another temporary variable to store the number of "online" fireflies for the calculation of the average

	long distSelf = 0;  //distance from the average to self
	long dist1 = 0; //distance from the average to the first connected firefly
	long dist2 = 0;
	long dist3 = 0;

	long tempDist1 = 0;
	long tempDist2 = 0;
	long tempDist3 = 0;

	if(myIteration > 3){
		distSelf = (long) abs(avgOffset);
		//tempSum stays the same since offset to self is 0
		numberOn++;
	}

	if(iteration1 > 3){  //ensures that the firefly is "alive"
		tempDist1 = offset1 - avgOffset;
		dist1 = (long) abs(tempDist1);
		tempSum += (double) offset1;  //adds the most recently calculated offset value
		numberOn++;  //adds one to the tally of how many values are being averaged, in order to divide by the correct number
	}

	if(iteration2 > 3){
		tempDist2 = offset2 - avgOffset;
		dist2 = (long) abs(tempDist2);
		tempSum += (double) offset2;
		numberOn++;
	}

	if(iteration3 > 3){
		tempDist3 = offset3 - avgOffset;
		dist3 = (long) abs(tempDist3);
		tempSum += (double) offset3;
		numberOn++;
	}

	if(numberOn > 0){
		avgOffset = (double) tempSum/(numberOn);  //calculates the average, based on the sum variable and the number of fireflies variable
		if(noiseReduction == true) restrictNoise(tempSum, numberOn, distSelf, dist1, dist2, dist3);  //although avgOffset is not passed into this function, avgOffset is used in and recalculated in this function
	}
}

void shiftMod(long divisor){
	mod = (double) (avgOffset/divisor);

		/*Serial.print(millis());  //outputs the millisecond time, the three offset values, and the average offset
		Serial.print(",");		 //remember self has an offset of 0, so the variance from the mean can still be determined
		Serial.print(offset1);	 //next copy the serial monitor data to a text file in comma format
		Serial.print(",");		 //then upload that text file to a google sheet to interpret and graph the data
		Serial.print(offset2);
		Serial.print(",");
		Serial.print(offset3);
		Serial.print(",");
		Serial.println(avgOffset);*/

	if((1 >= avgOffset) && (avgOffset >= -1) && (iteration1>3) && (iteration2>3) && (iteration3>3) && (DataCollected == false)){
		DataCollected = true;
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
		Serial.flush();
		
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
		Serial.flush();
		
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

void restrictNoise(double tempSum, double numberOn, long distSelf, long dist1, long dist2, long dist3){

	long tempMax1 = (long) max(distSelf, dist1);
	long tempMax2 = (long) max(dist2, dist3);
	long tempMaxTotal = (long) max(tempMax1, tempMax2);

	bool noGo = false;

	if(tempMaxTotal == distSelf);  //then tempSum stays the same since offset from self is 0
	else if(tempMaxTotal == dist1) tempSum -= (double) (offset1);
	else if(tempMaxTotal == dist2) tempSum -= (double) (offset2);
	else if(tempMaxTotal == dist3) tempSum -= (double) (offset3);  //this also works if two of the offsets are equal, because then the value subtracted is the same too
	else noGo = true;

	if((numberOn > 1) && (noGo == false)){
		avgOffset = (double) tempSum/(numberOn-1);  //calculates the average, based on the sum variable and the number of fireflies variable
	} 

	//else, average offset stays the same
}
