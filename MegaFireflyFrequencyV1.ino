/*
 MegaFireflyFrequencyV1.ino

 Each Arduino models a firefly in a firefly swarm. These "fireflies" blink an LED at a given frequency,
 And modulate their frequency based on the fireflies around it. Each firefly communicates when it has
 turned its LED on or off to the fireflies it is connected to through serial communication. Each firefly
 takes in data from the fireflies it is connected to, and determines each of their blinking frequencies.
 Over time, the network as a whole will collapse on one frequency and become synchronized like a real
 life swarm of fireflies. 

 The circuit:
 A minimum of 4 arduino megas interconnected on their 3 serial ports, each with an led on pin 13
 Each arduino should also be wired to one "Start Button" on pin 2
 
 Created by Jack Defay, 2016

 */

//not including any additional libraries

#define FREQUENCY 5000 //Use this to generalize program to upload to more than one arduino

int ledPin = 13;  //declares the output for the led
int buttonPin = 2;

int mod = 0;  //declares the modifier value as a global variable

	int iteration1 = 2;  //declares "iteration," which keeps track of the number of times the if statement has triggered
	int iteration2 = 2;  //same for the other two if statements
	int iteration3 = 2;

	unsigned long frequency1 = 0;  //declares "frequency," which keeps track of the frequencies that each of the connected fireflies
 	unsigned long frequency2 = 0;  //are blinking at, calculated based on the values stored in the "input" arrays
    unsigned long frequency3 = 0;

    unsigned long avgfrequency = 0;

int send = 0;  //an intermediate variable for sending data

//uses "unsigned long" format since these arrays store time. unsigned longs can store values twice as large(up to 2^32 - 1) as a normal long, but can only store possitive values
unsigned long input1[80][2];  //declares the 3, 2D input arrays of size 80 to store 80 state changes and the time they occur
unsigned long input2[80][2];  //first layer stores the time the change occured
unsigned long input3[80][2];  //second layer stores if it is on(1 for on, 0 for off)

int ledState = LOW;  //ledState used to set the LED
unsigned long previousMillis = 0;  //stores the last time LED was updated

bool started = false;  //boolean for starting the frequency calculations only once there are enough data points to operate with
bool button = false;  //boolean for starting the "loop" portion of the program once the button has been pressed

void setup() {

  pinMode(ledPin, OUTPUT);  //sets the "ledpin" to an output, so it can output to the LED
  pinMode(buttonPin, INPUT);  //sets the "buttonpin" to an input, so it can take in values from the button

//the for loop zeroes the arrays when the program starts
  for(int i = 0; i < 80; i++)
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
	if((digitalRead(buttonPin) == HIGH)/* && (millis()%1000) == 0*/){  //a simple if statement to start the rest of "loop" when the button (on pin2) is pressed
		button = true;
		previousMillis = millis();  //this resets the "previousMillis" to the current time, so each firefly restarts its counter, preventing built up flashes all occuring at once
	}

	while(button)  //starts main portion of the program when the button is pressed
	{

		//unsigned long tempInput1 = input1[iteration1-1][1];

		iteration1 = checkPort(input1, 1, iteration1);  //checks to see if any of the ports 123 have changed in state
		iteration2 = checkPort(input2, 2, iteration2);
		iteration3 = checkPort(input3, 3, iteration3);

	  // check to see if it's time to blink the LED: if the difference between the current time and last time the LED blinked is bigger than the interval value
	  unsigned long currentMillis = millis();

	  if (currentMillis - previousMillis >= FREQUENCY + mod)
	  {  //if the time passed since the previous blink equals the blink frequency, then led changes states
		    previousMillis = currentMillis;     // save the last time the LED blinked

		    if (ledState == LOW)
		    {
		    	send = 1;
		    	ledState = HIGH;  // if the LED is off turn it on and vice-versa:
		    } 
		    else
		    {
		    	send = 0;
		    	ledState = LOW;
		    }

		    digitalWrite(ledPin, ledState);  // set the LED with the ledState of the variable:
		    Serial1.write(send);  //then broadcasts that the led has changed state, in real life this would be done with a sense like sight
		    Serial2.write(send);  //but here we've simplified and streamlined the process of "sight" for communication
		    Serial3.write(send);
	  }

	  if(millis()%2000==0){  //print statements for troubleshooting
		    Serial.print("the frequencies of the other fireflies are: ");
		    Serial.println(frequency1);
		    Serial.println(frequency2);
		    Serial.println(frequency3);

		    Serial.print("the iterations of the other fireflies are: ");
		    Serial.println(iteration1);
		    Serial.println(iteration2);
		    Serial.println(iteration3);

			Serial.print("the avgfrequency is: ");
			Serial.println(avgfrequency);

		    Serial.print("FREQUENCY + mod = ");
		    Serial.println(FREQUENCY + mod);
	  }

	  if(iteration1 > 3)  //if the first array has stored at least 3 data points, then calculates the frequency by taking the difference of the last two data points
	  {
		  frequency1 = input1[iteration1-1][0] - input1[iteration1-2][0];  //checks and updates the frequency of the first input
	 	  started = true;
	  }

	  if(iteration2 > 3)
	  {
		  frequency2 = input2[iteration2-1][0] - input2[iteration2-2][0];  //...                                     second
		  started = true;
	  }

	  if(iteration3 > 3)
	  {
	  	  frequency3 = input3[iteration3-1][0] - input3[iteration3-2][0];  //...                                     thrid
	  	  started = true;
	  } 
	  
	  if((started))  //calculates the average to evaluate how synchronized this firefly is
	  {
	  	avgfrequency = ((frequency1+frequency2+frequency3+(FREQUENCY+mod))/4);  //calculates the arithmatic mean rounded off to a whole number(in milliseconds) - also, averages all connected fireflies AND itself to avoid differences in data.
	  }

	  if((started) && ((FREQUENCY+mod) < (avgfrequency))) mod++;  //shifts the frequency of the firefly longer by a tenth of a second if the average is larger

	  else if((started) && ((FREQUENCY+mod) > (avgfrequency))) mod--;  //shifts the frequency of the firefly down if the average is shorter

	  else if((started) && (((FREQUENCY+mod) == frequency1) && ((FREQUENCY+mod) == frequency2) && ((FREQUENCY+mod) == frequency3)))
	  { 
	 	  Serial.println("SYNCHRONIZED!");
	 	  synchronized();
	  }  //if all of the frequencies are equivalent, then enters the "synchronized" function - will switch to a more adaptable approach later

	  
	  //failsafe stops the program from running if the max value of the unsigned int format is exceded, or if the capacity of the arrays are exceded, by going into an infinite while loop
	  if((millis() >= 4294967295) || (iteration1 >=80) || (iteration2 >=80) || (iteration3 >=80) || ((FREQUENCY + mod) == 0)) excededTime();
    }//end bracket for the while loop
}//end bracket for the loop

void excededTime(){  //this function is called whenever there is no more space in the array, or if the time counter has been exceded
  Serial.println("time either exceded 136 years (lol), or there isn't enough space in the arrays - need a larger variable format to store more data");  // outputs information to the serial monitor as a reminder

  while(true)
  {  //infinite loop that blinks the led at a fairly quick pace to give a visual
    if(millis()%1000 == 0)
    {
      if(ledState==LOW) ledState = HIGH;  //every 1 second, switches the led from off to on
      else if(ledState==HIGH) ledState = LOW;  //or on to off, the "else" is key to prevent both cases from running
      digitalWrite(ledPin, ledState);  //then writes the change to the led
      Serial.println("*");  //then a print statement to signal that it timed out
    }
  }
}

void synchronized()  //called when the firefly is synched up with all adjacent fireflies
{  //will apply a more adaptive method later
   while(true)
   {  //blinks the led at the synchronzed frequency, but does so in a way that lines up all of the blinking
	    if(millis()%(FREQUENCY + mod) == 0)  //since the fireflies are all sync'd up, this time fram will be the same across each firefly
	    {
	      if(ledState==LOW) ledState = HIGH;
	      else if(ledState==HIGH) ledState = LOW;
	      digitalWrite(ledPin, ledState);  //same logic as the exededTime function
	    }
    }
}

int checkPort(unsigned long input[][2], int port, int iteration){  //generalized function for reading and recording data from the ports

  unsigned long temp = input[iteration-1][1]; //sets "temp" to the last recorded state of the given firefly (either on or off)
  //unsigned long time = millis();  //an intermediate variable for recording


  if((port == 1) && (Serial1.available()))  //if the firefly we're looking at is on port 1, AND there is data stored in that port's buffer:
  {
  		int readValue = Serial1.read();

  		Serial.print("111the recieved value is: ");
  		Serial.println(readValue);

  	  if((readValue == 1) && (temp == 0))  //if the first byte read = 1, and the last recorded state of said firefly is 0:
  	  {										//then a change of state must have occured in said firefly (in this case off to on)
		    input[iteration][0] = millis();  //then record the time this state change occured
		    input[iteration][1] = 1; 		   //and record that the firefly is now on
		    iteration++;					   //note that one more peice of data has been recorded, we're keeping track of this to prevent over recording data in the array
	  		Serial.print("iteration1 should increase");
	  }

	  else if((readValue == 0) && (temp == 1)) //then if the previous "if statement" was invalid,
	  {											//checks to see if the firefly we're looking at has gone from on to off
		    input[iteration][0] = millis();   //records the time this occured
		    input[iteration][1] = 0;			//records that said firefly is now off
		    iteration++;						//again, notes that one more piece of data has been recorded, this also allows us to keep track of which slot in the array we're on
	  		Serial.print("iteration1 should increase");
	  }
	  Serial.print("111the time recorded is: ");  //print statements for troubleshooting
	  Serial.println(input[iteration-1][0]);  //since iteration++ happens, this is actually the current value
	  Serial.print("temp1 = ");
      Serial.println(temp);
  }

  else if((port == 2) && (Serial2.available()))  //if the firefly we're looking at is on port 2, AND there is data stored in that port's buffer:
  {	
  		int readValue = Serial2.read();

  		Serial.print("222the recieved value is: ");
  		Serial.println(readValue);

  	  if((readValue == 1) && (temp == 0))  //if the first byte read = 1, and the last recorded state of said firefly is 0:
  	  {										//then a change of state must have occured in said firefly (in this case off to on)
		    input[iteration][0] = millis();  //then record the time this state change occured
		    input[iteration][1] = 1; 		   //and record that the firefly is now on
		    iteration++;					   //note that one more peice of data has been recorded, we're keeping track of this to prevent over recording data in the array
	 		Serial.print("iteration2 should increase");
	  }

	  else if((readValue == 0) && (temp == 1)) //then if the previous "if statement" was invalid,
	  {											//checks to see if the firefly we're looking at has gone from on to off
		    input[iteration][0] = millis();   //records the time this occured
		    input[iteration][1] = 0;			//records that said firefly is now off
		    iteration++;						//again, notes that one more piece of data has been recorded, this also allows us to keep track of which slot in the array we're on
	 		Serial.print("iteration2 should increase");
	  }
	  Serial.print("222the time recorded is: ");  //print statement for troubleshooting
	  Serial.println(input[iteration-1][0]);
	  Serial.print("temp2 = ");
	  Serial.println(temp);
  }

  else if((port == 3) && (Serial3.available()))  //if the firefly we're looking at is on port 3, AND there is data stored in that port's buffer:
  {
  		int readValue = Serial3.read();

  		Serial.print("333the recieved value is: ");
  		Serial.println(readValue);

  	  if((readValue == 1) && (temp == 0))  //if the first byte read = 1, and the last recorded state of said firefly is 0:
  	  {										//then a change of state must have occured in said firefly (in this case off to on)
		    input[iteration][0] = millis();  //then record the time this state change occured
		    input[iteration][1] = 1; 		   //and record that the firefly is now on
		    iteration++;					   //note that one more peice of data has been recorded, we're keeping track of this to prevent over recording data in the array
		    Serial.print("iteration3 should increase");
	  }

	  else if((readValue == 0) && (temp == 1)) //then if the previous "if statement" was invalid,
	  {											//checks to see if the firefly we're looking at has gone from on to off
		    input[iteration][0] = millis();   //records the time this occured
		    input[iteration][1] = 0;			//records that said firefly is now off
		    iteration++;						//again, notes that one more piece of data has been recorded, this also allows us to keep track of which slot in the array we're on
	 		Serial.print("iteration3 should increase");
	  }
	  Serial.print("333the time recorded is: ");  //print statements for troubleshooting
	  Serial.println(input[iteration-1][0]);
	  Serial.print("temp3 = ");
  	  Serial.println(temp);
  }
	return iteration;  //returns the "iteration" to make sure that iteration is being passed back to the loop
}//close bracket for the funtion
