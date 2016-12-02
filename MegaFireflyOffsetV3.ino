//MegaFireflyOffsetV3.ino

#define PERIOD 2000  //the duration of the blink in milliseconds
#define OFFSET 1000  //the initial wait time for the before the start of the algorithm
#define TIMEOUT (PERIOD + 1000) //the time before a connected firefly times out, and is no longer considered active

int led = 13;  //declare the pin for the led

int mod = 0;  //this variable will be used to shift the temporal position of the blinking
int totalmod = OFFSET; //for troubleshooting

unsigned int iteration1 = 1;
//unsigned int iteration2 = 1;
//unsigned int iteration3 = 1;
unsigned int myIteration = 1;

int offset1 = 0;
//int offset2 = 0;
//int offset3 = 0;

int avgOffset = 0;

unsigned long input1[80][2];
//unsigned long input2[80][2];
//unsigned long input3[80][2];
unsigned long myInput[80][2];

int ledState = LOW;
unsigned long previousMillis = 0;

bool startButton = false;

void setup() {

    pinMode(led, OUTPUT);

    for(int i=0; i<80; i++){
    	for(int j=0; j<2; j++){
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

	checkPort1();  //listen to the three ports on loop
	  //if it returns a 0 or 1, log it
	  //if it returns a 2, then initiate rest of the program with startbutton vriable

	//checks the 3 ports
	//blinks if it is time to, IF THE STARTBUTTON BOOL IS TRUE
	//update the average value with new data
	//shift the wavelength based on the mod variable

	//if times out, then use last stored values...or overwrite them...?
}

void checkPort1(){
	if(Serial1.available()){  //only reads from the port if there is available data in the buffer
		int readValue = Serial1.read();  //reads the first value stored in the serial buffer

		if(readValue==2){
			if(startButton==false){
				Serial1.write(2);  //then relays the 2 value across all of its ports
				//Serial2.write(2);
				//Serial3.write(3);
				delay(1000+OFFSET);
				previousMillis = millis();
			}

			startButton = true;  //if the firefly recieves a 2, it takes this as a signal to initiates the other processes
		}

		else if(readValue != input1[iteration1-1][1]){
			input1[iteration1][0] = millis();
			input1[iteration1][1] = readValue;
			iteration1++;
		}
	}
}

void timeToBlink(){
	unsigned long currentMillis = millis();

	if(((currentMillis - previousMillis) >= (PERIOD + mod)) && startButton){
		previousMillis = currentMillis;
		int valueToSend = 0;

		if(ledState == LOW){
			valueToSend = 1;
			ledState = HIGH;
		}
		else if(ledState == HIGH){
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
		if(myInput[iteration1-1][0] > input1[iteration1-1][0]){
			unsigned int posTemp = (myInput[iteration1-1][0] - input1[iteration1-1][0]);
			offset = (-1)*(posTemp);
		}
		else{
			offset1 = (input1[iteration1-1][0] - myInput[iteration1-1][0]);
		}
	}
}