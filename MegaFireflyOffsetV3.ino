//MegaFireflyOffsetV3.ino

#define PERIOD 2000  //the duration of the blink in milliseconds
#define OFFSET 700  //the initial wait time for the before the start of the algorithm
#define BUTTONPERSON 0 //this value will be 0 on all but one firefly, in order to initiate the start process. button person refers to a queen bee or ant, since the firefly with a 1 will serve tell all the other fireflies to start

int led = 13;  //declare the pin for the led
int button = 2;  //declare the pin for the start button, will only do something on a single firefly

int mod = 0;  //this variable will be used to shift the temporal position of the blinking
int totalMod = OFFSET; //for troubleshooting

unsigned int iteration1 = 1;  //keeps track of the number of times a value has been stored in the array. Also keeps track of the current position in the array for calculations
//unsigned int iteration2 = 1;
//unsigned int iteration3 = 1;
unsigned int myIteration = 1;  //keeps track of the same information, but for self

int offset1 = 0;  //stores the most recent offset value, used for calculations, calculated in the function updateOffset
//int offset2 = 0;
//int offset3 = 0;

int avgOffset = 0;  //the average offset value calculated in the function updateAvg

unsigned long input1[80][3];  //an array that logs a bunch of useful data for people to use
//unsigned long input2[80][3];  //stores the time that a state change has occured, the new state, and the offset values
//unsigned long input3[80][3];
unsigned long myInput[80][3];

int ledState = LOW;  //an intermediate variable used in the timeToBlink to set the led state
unsigned long previousMillis = 0;  //a variable that keeps track of the last time the led blinked, in order to properly space the next blink in timeToBlink
unsigned long previousMillis2 = 0;  //used for the same perpose, but in the shiftMod function

bool startButton = false;  //boolean variable that lets the program know when it has started based on the propagating start message

void setup() {

    pinMode(led, OUTPUT);
    pinMode(button, INPUT); //initiates the button as an input, again, will only function on a single firefly

    for(int i=0; i<80; i++){  //zeros the arrays
    	for(int j=0; j<3; j++){
  		    input1[i][j] = 0;
  		   // input2[i][j] = 0;
  	    	//input3[i][j] = 0;
  		    myInput[i][j] = 0;
    	}
    }

    Serial.begin(9600);  //initiates the serial monitor
    Serial1.begin(9600);  //initiates the other 3 serial ports to listen to the connected "fireflies"
  //  Serial2.begin(9600);
   // Serial3.begin(9600);

    Serial.println("*****************************************************");  //to signal the start of the program
}

void loop() {

	if((digitalRead(button) == HIGH) && (BUTTONPERSON == 1)){  //if the firfly reads that the connected button has been pressed, and this firefly is a designated "button person" firefly, then it relays the start signal to all the connected firelfies
		Serial1.write(2);  //the number 2 is the designated start signal
		//Serial2.write(2);
		//Serial3.write(2);
		Serial.println("the firefly tried to initiate the start");
	}

	if(mod > PERIOD || mod < (-1*PERIOD)) Serial.println("MOD IS SPIRALLING OUT OF CONTROL!");  //a debugging check that prints whenever the mod value overtakes the period, which should never be necessary if the initial offset values are less that the period

	checkPort1();  //listen to the three ports on loop
	  //if it returns a 0 or 1, log it
	  //if it returns a 2, then initiate rest of the program with startbutton vriable

	//checks the 3 ports
	timeToBlink();//blinks if it is time to, IF THE STARTBUTTON BOOL IS TRUE
	calcOffset1();
	updateAvg();//update the average value with new data
	shiftMod();//shift the wavelength based on the mod variable

	//if times out, then use last stored values...or overwrite them...?
}

void checkPort1(){
	if(Serial1.available()){  //only reads from the port if there is available data in the buffer
		int readValue = Serial1.read();  //reads the first value stored in the serial buffer

		if(readValue==2){  //an if case that relays the start command to all connected fireflies the first time it recieves the start command itself
			if(startButton==false){  //to prevent the command from looping indefinitely
				Serial1.write(2);  //then relays the 2 value across all of its ports
				//Serial2.write(2);
				//Serial3.write(3);
				delay(PERIOD+OFFSET);  //waits the set period length, then an additional time for offset. this is how the offset variable is introduced into the system
				previousMillis = millis();  //resets the previousMillis and previousMillis2 variables to prevent things from piling up
				previousMillis2 = millis();
				Serial.println("started");  //gives an indicator in the serial monitor
			}

			startButton = true;  //finally sets the startButton variable to true, initiating its other processes and locking down the loop with the (startbutton==false) if case
		}

		else if((readValue != input1[iteration1-1][1]) && (startButton)){  //if the reieved value is distinct from the last recieved value, and the program has "started" from recieving a 0
			input1[iteration1][0] = millis();  //then sets the first row value of the array to the current time
			input1[iteration1][1] = readValue;  //and the second row value to the new state of the led
			iteration1++;  //increases the tally for the number of datapoints logged

			Serial.print("iteration1 = ");  //print statements for debugging
			Serial.println(iteration1);
			Serial.print("the received value is: ");
			Serial.println(input1[iteration1-1][1]);
		}
	}
}

//void checkPort2()  //will have to copy the checkPort function to work for ports 2 and 3
//void checkPort3()

void timeToBlink(){
	unsigned long currentMillis = millis();  //allows the function to compare the current time to the time recorded in previousMillis
//Serial.print("previousMillis = ");
//Serial.println(previousMillis);
	if(((currentMillis - previousMillis) >= (PERIOD + mod)) && (startButton)){  //if the difference in time between the previousMillis and current time, is greater than or equal to the period of the firefly plus its modifier value; and the program has "started"
		previousMillis = currentMillis;  //if the criteria are met, then resets the previousMillis time to the current one, in order to prep for the next cycle
		int valueToSend = 0;  //initiates a intermediate variable for the value that will be sent across the serial ports to the other fireflies

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
		//Serial2.write(valueToSend);
		//Serial3.write(valueToSend);

		myInput[myIteration][0] = millis();  //logs data in the same format as the other arrays, but for self
		myInput[myIteration][1] = valueToSend;
		myIteration++;
	}
}

void calcOffset1(){
	if(iteration1 > 3){
		unsigned int commonIteration = min(iteration1, myIteration);  //finds the least common itteration of connected firelfy1 and self, in order to find the most recent data point that may be used in calculation

		unsigned int posTemp = abs(myInput[commonIteration-1][0] - input1[commonIteration-1][0]);  //takes the distance between the two values

		//Serial.print("posTemp = ");
		//Serial.println(posTemp);

		if(myInput[commonIteration-1][0] > input1[commonIteration-1][0]){  //if the time value at the common data point is greater for self
			offset1 = (-1)*(posTemp);  //sets the offset value to the negative of the distance, because the connected firefly one must be behind self
		}
		else{  //if the time value for self isn't greater ==> the time value for connected firelfy1 is greater
			offset1 = posTemp;  //sets the offset value to the distance, because the connected firefly must be ahead of self
		}

		input1[commonIteration-1][3] = offset1;  //logs the offset value in the third row of the array

		//Serial.print("offset1 = ");
		//Serial.println(offset1);
	}
}

void updateAvg(){
	int tempSum = 0;  //a temporary variable for the sum of the values
	int numberOn = 0;  //another temporary variable to store the number of "online" fireflies for the calculation of the average

	if(iteration1 > 3){  //ensures that the firefly is "alive"
		tempSum += offset1;  //adds the most recently calculated offset value
		numberOn++;  //adds one to the tally of how many values are being averaged, in order to divide by the correct number
	}

	/*
	if(iteration2 > 3){
		tempSum += offset2;
		numberOn++;
	}

	if(iteration3 > 3){
		tempSum += offset3;
		numberOn++;
	}
	*/

	if(numberOn > 0) avgOffset = tempSum/numberOn;  //calculates the average, based on the sum variable and the number of fireflies variable
}

void shiftMod(){
	unsigned long currentMillis2 = millis();  //checks the current time with currentMillis, in the same way as the timeToBlink function

	if((currentMillis2 - previousMillis2) >= (1000)){  //again, like timeToBlink, checks if the difference in times has reached a certain magnitude, an arbitrary one second, then changes the mod value accordingly
		previousMillis2 = currentMillis2;  //same as timeToBlink

		totalMod += mod;  //keeps a tally of the total mod value, shouldn't be entirely accurate, just for debugging
		mod = (avgOffset/2);  //sets the mod value to one half of the averageOffset, which is the average distance between self and the other fireflies at any given point, factoring in direction

		Serial.print("mod = ");
		Serial.println(mod);

		if(mod==0  && iteration1>3 /*&& iteration2>3 && iteration3>3*/) Serial.println("SYNCHRONIZED!");
	}
}
