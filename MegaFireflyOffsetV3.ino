//MegaFireflyOffsetV3.ino

#define PERIOD 2000  //the duration of the blink in milliseconds
#define OFFSET 1000  //the initial wait time for the before the start of the algorithm
#define BUTTONPERSON 1 //this value will be 0 on all but one firefly, in order to initiate the start process. button person refers to a queen bee or ant, since the firefly with a 1 will serve tell all the other fireflies to start

long led = 13;  //declare the pin for the led
long button = 2;  //declare the pin for the start button, will only do something on a single firefly

long mod = 0;  //this variable will be used to shift the temporal position of the blinking
long totalMod = OFFSET; //for troubleshooting

long iteration1 = 1;  //keeps track of the number of times a value has been stored in the array. Also keeps track of the current position in the array for calculations
long iteration2 = 1;
long iteration3 = 1;
long myIteration = 1;  //keeps track of the same information, but for self

long offset1 = 0;  //stores the most recent offset value, used for calculations, calculated in the function updateOffset
long offset2 = 0;
long offset3 = 0;

long avgOffset = 0;  //the average offset value calculated in the function updateAvg

long input1[80][3];  //an array that logs a bunch of useful data for people to use
long input2[80][3];  //stores the time that a state change has occured, the new state, and the offset values
long input3[80][3];
long myInput[80][3];

long ledState = LOW;  //an intermediate variable used in the timeToBlink to set the led state
long previousMillis = 0;  //a variable that keeps track of the last time the led blinked, in order to properly space the next blink in timeToBlink
long previousMillis2 = 0;  //used for the same perpose, but in the shiftMod function

bool startButton = false;  //boolean variable that lets the program know when it has started based on the propagating start message

void setup() {

    pinMode(led, OUTPUT);
    pinMode(button, INPUT); //initiates the button as an input, again, will only function on a single firefly

    for(int i=0; i<80; i++){  //zeros the arrays
    	for(int j=0; j<3; j++){
  		    input1[i][j] = 0;
  		    input2[i][j] = 0;
  	    	input3[i][j] = 0;
  		    myInput[i][j] = 0;
    	}
    }

    Serial.begin(9600);  //initiates the serial monitor
    Serial1.begin(9600);  //initiates the other 3 serial ports to listen to the connected "fireflies"
    Serial2.begin(9600);
    Serial3.begin(9600);

    Serial.println("*****************************************************");  //to signal the start of the program
}

void loop() {

	if((digitalRead(button) == HIGH) && (BUTTONPERSON == 1)){  //if the firfly reads that the connected button has been pressed, and this firefly is a designated "button person" firefly, then it relays the start signal to all the connected firelfies
		Serial1.write(2);  //the number 2 is the designated start signal
		Serial2.write(2);
		Serial3.write(2);
		Serial.println("the firefly tried to initiate the start");
	}

	if(mod > PERIOD || mod < (-1*PERIOD)) Serial.println("MOD IS SPIRALLING OUT OF CONTROL!");  //a debugging check that prints whenever the mod value overtakes the period, which should never be necessary if the initial offset values are less that the period

	checkPort1();  //listen to the three ports on loop
	checkPort2();
	checkPort3();
	  //if it returns a 0 or 1, log it
	  //if it returns a 2, then initiate rest of the program with startbutton vriable

	//checks the 3 ports
	timeToBlink();//blinks if it is time to, IF THE STARTBUTTON BOOL IS TRUE

	calcOffset1();
	calcOffset2();
	calOffset3();

	updateAvg();//update the average value with new data
	shiftMod();//shift the wavelength based on the mod variable

	//if times out, then use last stored values...or overwrite them...?
}

void checkPort1(){
	if(Serial1.available()){  //only reads from the port if there is available data in the buffer
		long readValue = (long) Serial1.read();  //reads the first value stored in the serial buffer

		if(readValue==2){  //an if case that relays the start command to all connected fireflies the first time it recieves the start command itself
			if(startButton==false){  //to prevent the command from looping indefinitely
				Serial1.write(2);  //then relays the 2 value across all of its ports
				Serial2.write(2);
				Serial3.write(2);
				delay(PERIOD+OFFSET);  //waits the set period length, then an additional time for offset. this is how the offset variable is introduced into the system
				previousMillis = (long) millis();  //resets the previousMillis and previousMillis2 variables to prevent things from piling up
				previousMillis2 = (long) millis();
				Serial.println("started");  //gives an indicator in the serial monitor
			}

			startButton = true;  //finally sets the startButton variable to true, initiating its other processes and locking down the loop with the (startbutton==false) if case
		}

		else if((readValue != input1[iteration1-1][1]) && (startButton)){  //if the reieved value is distinct from the last recieved value, and the program has "started" from recieving a 0
			input1[iteration1][0] = (long) millis();  //then sets the first row value of the array to the current time
			input1[iteration1][1] = (long) readValue;  //and the second row value to the new state of the led
			iteration1++;  //increases the tally for the number of datapoints logged

			//if(millis()%1000 == 0){
			//Serial.print("input1[iteration1-1][0] = ");
			//Serial.println(input1[iteration1-1][0]);
			//}

			//Serial.print("iteration1 = ");  //print statements for debugging
			//Serial.println(iteration1);
			//Serial.print("the received value is: ");
			//Serial.println(input1[iteration1-1][1]);
		}
	}
}

void checkPort2(){
	if(Serial2.available()){  //only reads from the port if there is available data in the buffer
		long readValue = (long) Serial2.read();  //reads the first value stored in the serial buffer

		if(readValue==2){  //an if case that relays the start command to all connected fireflies the first time it recieves the start command itself
			if(startButton==false){  //to prevent the command from looping indefinitely
				Serial1.write(2);  //then relays the 2 value across all of its ports
				Serial2.write(2);
				Serial3.write(2);
				delay(PERIOD+OFFSET);  //waits the set period length, then an additional time for offset. this is how the offset variable is introduced into the system
				previousMillis = (long) millis();  //resets the previousMillis and previousMillis2 variables to prevent things from piling up
				previousMillis2 = (long) millis();
				Serial.println("started");  //gives an indicator in the serial monitor
			}

			startButton = true;  //finally sets the startButton variable to true, initiating its other processes and locking down the loop with the (startbutton==false) if case
		}

		else if((readValue != input2[iteration2-1][1]) && (startButton)){  //if the reieved value is distinct from the last recieved value, and the program has "started" from recieving a 0
			input2[iteration2][0] = (long) millis();  //then sets the first row value of the array to the current time
			input2[iteration2][1] = (long) readValue;  //and the second row value to the new state of the led
			iteration2++;  //increases the tally for the number of datapoints logged
		}
	}
}

void checkPort3(){
	if(Serial3.available()){  //only reads from the port if there is available data in the buffer
		long readValue = (long) Serial3.read();  //reads the first value stored in the serial buffer

		if(readValue==2){  //an if case that relays the start command to all connected fireflies the first time it recieves the start command itself
			if(startButton==false){  //to prevent the command from looping indefinitely
				Serial1.write(2);  //then relays the 2 value across all of its ports
				Serial2.write(2);
				Serial3.write(2);
				delay(PERIOD+OFFSET);  //waits the set period length, then an additional time for offset. this is how the offset variable is introduced into the system
				previousMillis = (long) millis();  //resets the previousMillis and previousMillis2 variables to prevent things from piling up
				previousMillis2 = (long) millis();
				Serial.println("started");  //gives an indicator in the serial monitor
			}

			startButton = true;  //finally sets the startButton variable to true, initiating its other processes and locking down the loop with the (startbutton==false) if case
		}

		else if((readValue != input3[iteration3-1][1]) && (startButton)){  //if the reieved value is distinct from the last recieved value, and the program has "started" from recieving a 0
			input3[iteration3][0] = (long) millis();  //then sets the first row value of the array to the current time
			input3[iteration3][1] = (long) readValue;  //and the second row value to the new state of the led
			iteration3++;  //increases the tally for the number of datapoints logged
		}
	}
}

void timeToBlink(){
	long currentMillis = (long) millis();  //allows the function to compare the current time to the time recorded in previousMillis
//Serial.print("previousMillis = ");
//Serial.println(previousMillis);
	if(((currentMillis - previousMillis) >= (PERIOD + mod)) && (startButton)){  //if the difference in time between the previousMillis and current time, is greater than or equal to the period of the firefly plus its modifier value; and the program has "started"
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

		myInput[myIteration][0] = (long) millis();  //logs data in the same format as the other arrays, but for self
		myInput[myIteration][1] = (long) valueToSend;
		myIteration++;

		//Serial.print("myInput[myIteration-1][0] = ");
		//Serial.println(myInput[myIteration-1][0]);
	}
}

void calcOffset1(){
	if(iteration1 > 3){
		long commonIteration = (long) min(iteration1, myIteration);  //finds the least common itteration of connected firelfy1 and self, in order to find the most recent data point that may be used in calculation

		offset1 = (long) (input1[commonIteration-1][0] - myInput[commonIteration-1][0]);
		input1[commonIteration-1][2] = offset1;  //logs the offset value in the third row of the array

		if(millis()%1000 == 0){
			//Serial.print("common iteration = ");
			//Serial.println(commonIteration);
			Serial.print("offset1 = ");
			Serial.println(offset1);
		}
	}
}

void calcOffset2(){
	if(iteration2 > 3){
		long commonIteration = (long) min(iteration2, myIteration);  //finds the least common itteration of connected firelfy1 and self, in order to find the most recent data point that may be used in calculation

		offset2 = (long) (input2[commonIteration-1][0] - myInput[commonIteration-1][0]);
		input2[commonIteration-1][2] = offset2;  //logs the offset value in the third row of the array

		if(millis()%1000 == 0){
			Serial.print("offset2 = ");
			Serial.println(offset2);
		}
	}
}

void calcOffset3(){
	if(iteration3 > 3){
		long commonIteration = (long) min(iteration3, myIteration);  //finds the least common itteration of connected firelfy1 and self, in order to find the most recent data point that may be used in calculation

		offset3 = (long) (input3[commonIteration-1][0] - myInput[commonIteration-1][0]);
		input3[commonIteration-1][2] = offset3;  //logs the offset value in the third row of the array

		if(millis()%1000 == 0){
			Serial.print("offset3 = ");
			Serial.println(offset3);
		}
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

	if(numberOn > 0) avgOffset = (long) tempSum/numberOn;  //calculates the average, based on the sum variable and the number of fireflies variable

	if(millis()%1000 == 0){
		Serial.print("avgOffset= ");
		Serial.println(avgOffset);
	}
}

void shiftMod(){
	long currentMillis2 = (long) millis();  //checks the current time with currentMillis, in the same way as the timeToBlink function

	if(((currentMillis2 - previousMillis2) >= (1000)) && (avgOffset != 0)){  //again, like timeToBlink, checks if the difference in times has reached a certain magnitude, an arbitrary one second, then changes the mod value accordingly
		previousMillis2 = (long) currentMillis2;  //same as timeToBlink

		totalMod += (long) mod;  //keeps a tally of the total mod value, shouldn't be entirely accurate, just for debugging
		mod = (long) (avgOffset/2);  //sets the mod value to one half of the averageOffset, which is the average distance between self and the other fireflies at any given point, factoring in direction

		Serial.print("mod = ");
		Serial.println(mod);

		if(mod==0  && iteration1>3 /*&& iteration2>3 && iteration3>3*/) Serial.println("SYNCHRONIZED!");
	}
}
