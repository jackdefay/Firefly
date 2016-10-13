/*
 MegaFireflyV1.ino

 Each Arduino models a firefly in a firefly swarm. These "fireflies" blink an LED at a given frequency,
 And modulate their frequency based on the fireflies around it. Each firefly communicates when they have
 turned their LED on or off to the fireflies it is connected to through serial communication. Each firefly
 takes in data from the fireflies it is connected to, and determines each of their blinking frequencies.
 Over time, the network as a whole will collapse on one frequency and become synchronized like a real
 life swarm of fireflies. 

 The circuit:
 A minimum of 4 arduino megas interconnected on their 3 serial ports, each with an led on pin 13
 
 Created by Jack Defay, 2016

 */

//not including any additional libraries

#define FREQUENCY 2000 //has to be between 0 and 6000, will use this to generalize program to upload to more than one arduino

int ledPin = 13;  //declares the output for the led

int mod = 0;  //declares the modifier value as a global variable

	int iteration1 = 1;  //declares "iteration," which keeps track of the number of times the if statement has triggered
	int iteration2 = 1;  //same for the other two if statements
	int iteration3 = 1;

	int frequency1 = 0;  //declares "frequency," which keeps track of the frequencies that each of the connected fireflies
 	int frequency2 = 0;  //are blinking at, calculated based on the values stored in the "input" arrays
    int frequency3 = 0;

//uses "unsigned long" format since these arrays store time. unsigned longs can store values twice as large(up to 2^32 - 1) as a normal long, but can only store possitive values
unsigned long input1[80][2];  //declares the 3, 2D input arrays of size 80 to store 80 state changes and the time they occur
unsigned long input2[80][2];  //first layer stores the time the change occured
unsigned long input3[80][2];  //second layer stores if it is on(1 for on, 0 for off)

int ledState = LOW;  //ledState used to set the LED
unsigned long previousMillis = 0;  //stores the last time LED was updated

bool started = false;

void setup() {

  pinMode(ledPin, OUTPUT);

//the for loop zeroes the arrays when the program starts
  for(int i = 0; i < 80; i++)\
  {
      for(int j = 0; j < 2; j++)
      {
          input1[i][j] = 0;
          input2[i][j] = 0;
          input3[i][j] = 0;
      }
  }

  Serial.begin(9600);  //starts, and sets the baud rate of the serial monitor

  Serial1.begin(9600);  //sets up the other three serail communication lines
  Serial2.begin(9600);  //which will be used to communicate with the other 3 linked arduino's
  Serial3.begin(9600);
  
  Serial.println("******************************************************************"); //a break to signal the start of the program on the serial monitor
}

void loop()
{

	iteration1 = checkPort(input1, 1, iteration1);  //checks to see if any of the ports 123 have changed in state
	iteration2 = checkPort(input2, 2, iteration2);
	iteration3 = checkPort(input3, 3, iteration3);

  
  delay(1);  //delays one millisecond to give a sense of time

  // check to see if it's time to blink the LED: if the difference between the current time and last time the LED blinked is bigger than the interval value
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= FREQUENCY + mod)
  {  //if the time passed since the previous blink equals the blink frequency, then led changes states
	    previousMillis = currentMillis;     // save the last time the LED blinked

	    if (ledState == LOW)
	    {
	    	ledState = HIGH;  // if the LED is off turn it on and vice-versa:
	    	Serial1.write(1);  //then broadcasts that the led has turned on, in real life this would be done with a sense like sight
	    	Serial2.write(1);  //but here we've simplified and streamlined the process of "sight" for communication
	    	Serial3.write(1);
	    } 
	    else
	    {
	    	ledState = LOW;
	    	Serial1.write(0);  //then broadcastst that the led has turned off
	    	Serial2.write(0);
	    	Serial3.write(0);
	    }

	    digitalWrite(ledPin, ledState);  // set the LED with the ledState of the variable:
  }

  if(millis()%1000==0){  //print statements for troubleshooting
	    Serial.print("the frequencies of the other fireflies are: ");
	    Serial.println(frequency1);
	    Serial.println(frequency2);
	    Serial.println(frequency3);

	    Serial.print("the iterations of the other fireflies are: ");
	    Serial.println(iteration1);
	    Serial.println(iteration2);
	    Serial.println(iteration3);

	    Serial.print("the time since this program started is: ");
	    Serial.println(currentMillis);
  }

  if(iteration1 > 3)
  {
	  frequency1 = (input1[iteration1-2][0] - input1[iteration1-3][0]);  //checks and updates the frequency of the first input
 	  started = true;
  }

  if(iteration2 > 2)
  {
	  frequency2 = input2[iteration2-1][0] - input2[iteration2-2][0];  //...                                       second
	  started = true;
  }

  if(iteration3 > 2)
  {
  	  frequency3 = input3[iteration3-1][0] - input3[iteration3-2][0];  //...                                       thrid
  	  started = true;
  } 
  //this process is inherently biased towards shifting to a longer frequency - this should simplify the decision making aspect, but in the futer I plan to have it based off an average to improve efficiency
  if(((FREQUENCY+mod) < frequency1) && (started) || ((FREQUENCY+mod) < frequency2) || ((FREQUENCY+mod) < frequency3)) mod++;  //shifts the frequency of the firefly longer by a tenth of a second if any of the other frequencies are larger

  else if(((FREQUENCY+mod) > frequency1) && (started) && ((FREQUENCY+mod) > frequency2) && ((FREQUENCY+mod) > frequency3)) mod--;  //shifts the frequency of the firefly down if all of the others are shorter

  else if(((FREQUENCY+mod) == frequency1) && (started) && ((FREQUENCY+mod) == frequency2) && ((FREQUENCY+mod) == frequency3))
  { 
 	  Serial.println("SYNCHRONIZED!");
 	  synchronized();  //enters "synchronized" function
  }  //confirmation if the frequencies are all the same

  
  //failsafe stops the program from running if the max value of the unsigned int format is exceded, or if the capacity of the arrays are exceded, by going into an infinite while loop
  if(millis() >= 4294967295 || iteration1 >=80 || iteration2 >=80 || iteration3 >=80) excededTime();
}

void excededTime(){  //this function is called whenever there is no more space in the array, or if the time counter has been exceded
  Serial.println("time exceded 136 years (lol), need a larger variable format to store more time data");  // outputs information to the serial monitor as a reminder

  while(true)
  {  //infinite loop that blinks the led at a fairly quick pace to give a visual
    if(millis()%1000 == 0)
    {
      if(ledState==LOW) ledState = HIGH;
      else if(ledState==HIGH) ledState = LOW;
      digitalWrite(ledPin, ledState);
    }
  }
}

void synchronized(){  //called when the firefly is synced up with its immediate neighbors, loops the light based on the currently sync'd timing
	while(true){	  //will have a more adaptable strategy later, for now this is just a standin
		digitalWrite(ledPin, HIGH);
		delay(FREQUENCY + mod);
		digitalWrite(ledPin, LOW);
		delay(FREQUENCY + mod);
	}
}

int checkPort(unsigned long input[][2], int port, int iteration){  //generalized function for reading and recording data from the ports

  unsigned long temp = input[iteration-1][1]; //sets "temp" to the last recorded state of the given firefly (either on or off)

  if((port == 1) && (Serial1.available()))  //if the firefly we're looking at is on port 1, AND there is data stored in that port's buffer:
  {
  	  unsigned long time = millis();
  	  if((Serial1.read() == 1) && (temp == 0))  //if the first byte read = 1, and the last recorded state of said firefly is 0:
  	  {										//then a change of state must have occured in said firefly (in this case off to on)
		    input[iteration-1][0] = time; //millis();  //then record the time this state change occured
		    input[iteration-1][1] = 1; 		   //and record that the firefly is now on
		    iteration++;					   //note that one more peice of data has been recorded, we're keeping track of this to prevent over recording data in the array
	  }

	  else if((Serial1.read() == 0) && (temp == 1)) //then if the previous "if statement" was invalid,
	  {											//checks to see if the firefly we're looking at has gone from on to off
		    input[iteration-1][0] = time;   //records the time this occured
		    input[iteration-1][1] = 0;			//records that said firefly is now off
		    iteration++;						//again, notes that one more piece of data has been recorded, this also allows us to keep track of which slot in the array we're on
	  }
	  Serial.print("millis = ");
	  Serial.println(millis());
	  Serial.print("time = ");
	  Serial.println(time);
	  Serial.print("111the time recorded is: ");
	  Serial.println(input[iteration-2][0]);
  }

  else if((port == 2) && (Serial1.available()))  //if the firefly we're looking at is on port 2, AND there is data stored in that port's buffer:
  {
  	  unsigned long time = millis();
  	  if((Serial2.read() == 1) && (temp == 0))  //if the first byte read = 1, and the last recorded state of said firefly is 0:
  	  {										//then a change of state must have occured in said firefly (in this case off to on)
		    input[iteration-1][0] = time; //millis();  //then record the time this state change occured
		    input[iteration-1][1] = 1; 		   //and record that the firefly is now on
		    iteration++;					   //note that one more peice of data has been recorded, we're keeping track of this to prevent over recording data in the array
	  }

	  else if((Serial2.read() == 0) && (temp == 1)) //then if the previous "if statement" was invalid,
	  {											//checks to see if the firefly we're looking at has gone from on to off
		    input[iteration-1][0] = time;   //records the time this occured
		    input[iteration-1][1] = 0;			//records that said firefly is now off
		    iteration++;						//again, notes that one more piece of data has been recorded, this also allows us to keep track of which slot in the array we're on
	  }
	  Serial.print("millis = ");  //print statements for troubleshooting
	  Serial.println(millis());
	  Serial.print("time = ");
	  Serial.println(time);
	  Serial.print("222the time recorded is: ");
	  Serial.println(input[iteration-2][0]);
  }

  else if((port == 3) && (Serial3.available()))  //if the firefly we're looking at is on port 3, AND there is data stored in that port's buffer:
  {
  	  unsigned long time = millis();
  	  if((Serial3.read() == 1) && (temp == 0))  //if the first byte read = 1, and the last recorded state of said firefly is 0:
  	  {										//then a change of state must have occured in said firefly (in this case off to on)
		    input[iteration-1][0] = time; //millis();  //then record the time this state change occured
		    input[iteration-1][1] = 1; 		   //and record that the firefly is now on
		    iteration++;					   //note that one more peice of data has been recorded, we're keeping track of this to prevent over recording data in the array
	  }

	  else if((Serial3.read() == 0) && (temp == 1)) //then if the previous "if statement" was invalid,
	  {											//checks to see if the firefly we're looking at has gone from on to off
		    input[iteration-1][0] = time;   //records the time this occured
		    input[iteration-1][1] = 0;			//records that said firefly is now off
		    iteration++;						//again, notes that one more piece of data has been recorded, this also allows us to keep track of which slot in the array we're on
	  }
	  Serial.print("millis = ");  //print statements for troubleshooting
	  Serial.println(millis());
	  Serial.print("time = ");
	  Serial.println(time);
	  Serial.print("333the time recorded is: ");
	  Serial.println(input[iteration-2][0]);
  }

	return iteration;  //returns the "iteration" to make sure that integer is being passed back to the loop
}