//MegaFireflyOffsetV3.ino

#define PERIOD 2000  //the duration of the blink in milliseconds
#define OFFSET 1000  //the initial wait time for the before the start of the algorithm
#define QUEEN 1 //this value will be 0 on all but one firefly, in order to initiate the start process. Queen refers to a queen bee or ant, since the firefly with a 1 will serve tell all the other fireflies to start

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

	if((digitalRead(button) == HIGH) && (QUEEN == 1)){  //if the firfly reads that the connected button has been pressed, and this firefly is a designated "Queen" firefly, then it relays the start signal to all the connected firelfies
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
	//calcOffset1();
	//updateAvg();//update the average value with new data
	//shiftMod();//shift the wavelength based on the mod variable

	//if times out, then use last stored values...or overwrite them...?

	//PRINT OUT A BUNCH OF THINGS TO FIND OUT WHY NOT SENDING A 1 OVER THE SERIAL PORTS
}

void checkPort1(){
	if(Serial1.available()){  //only reads from the port if there is available data in the buffer
		int readValue = Serial1.read();  //reads the first value stored in the serial buffer

		if(readValue==2){
			if(startButton==false){
				Serial1.write(2);  //then relays the 2 value across all of its ports
				//Serial2.write(2);
				//Serial3.write(3);
				delay(PERIOD+OFFSET);
				previousMillis = millis();
				Serial.println("started");
			}

			startButton = true;  //if the firefly recieves a 2, it takes this as a signal to initiates the other processes
		}

		else if((readValue != input1[iteration1-1][1]) && (startButton)){
			input1[iteration1][0] = millis();
			input1[iteration1][1] = readValue;
			iteration1++;

			Serial.print("iteration1 = ");
			Serial.println(iteration1);
			Serial.print("the received value is: ");
			Serial.println(input1[iteration1][0]);
		}
	}
}

void timeToBlink(){
	unsigned long currentMillis = millis();
//Serial.print("previousMillis = ");
//Serial.println(previousMillis);
	if(((currentMillis - previousMillis) >= (PERIOD + mod)) && (startButton)){
		previousMillis = currentMillis;
		int valueToSend = 0;

		if(ledState == LOW){
			valueToSend = 1;
			ledState = HIGH;
		}
		else{
			valueToSend = 0;
			ledState = LOW;
		}

		digitalWrite(led, ledState);
		Serial1.write(valueToSend);
		//Serial2.write(valueToSend);
		//Serial3.write(valueToSend);

		myInput[myIteration][0] = millis();
		myInput[myIteration][1] = valueToSend;
		myIteration++;
	}
}

void calcOffset1(){
	if(iteration1 > 3){
		unsigned int commonIteration = min(iteration1, myIteration);

		unsigned int posTemp = abs(myInput[commonIteration-1][0] - input1[commonIteration-1][0]);

		//Serial.print("posTemp = ");
		//Serial.println(posTemp);

		if(myInput[commonIteration-1][0] > input1[commonIteration-1][0]){
			offset1 = (-1)*(posTemp);
		}
		else{
			offset1 = posTemp;
		}

		input1[commonIteration-1][3] = offset1;

		//Serial.print("offset1 = ");
		//Serial.println(offset1);
	}
}

void updateAvg(){
	int tempSum = 0;
	int numberOn = 0;

	if(iteration1 > 3){  //ensures that the firefly is alive
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

	if(numberOn > 0) avgOffset = tempSum/numberOn;  //calculates the average
}

void shiftMod(){
	unsigned long currentMillis2 = millis();

	if((currentMillis2 - previousMillis2) >= (1000)){
		previousMillis2 = currentMillis2;

		totalMod += mod;
		mod = (avgOffset/2);

		if(mod==0  && iteration1>3 /*&& iteration2>3 && iteration3>3*/) Serial.println("SYNCHRONIZED!");
	}
}
