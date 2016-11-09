	/*
	 MegaFireflyOffset.ino

	 Each Arduino models a firefly in a firefly swarm. These "fireflies" blink an LED at a constant frequency,
	 And modulate their offset based on the fireflies around it. Each firefly communicates when it has
	 turned its LED on or off to the fireflies it is connected to through serial communication. Each firefly
	 takes in data from the fireflies it is connected to, and determines each of their blinking offsets.
	 Over time, the network as a whole will collapse on one offset and become synchronized like a real
	 life swarm of fireflies. 

	 The circuit:
	 A minimum of 4 arduino megas interconnected on their 3 serial ports, each with an led on pin 13
	 Each arduino should also be wired to one "Start Button" on pin 2
	 
	 Created by Jack Defay, 2016

	*/

	//not including any additional libraries

	#define FREQUENCY 1000  //should remain constant across uploads, but could change this value to model different species of fireflies
	#define OFFSET 100 //Use this to generalize program to upload to more than one arduino

	int ledPin = 13;  //declares the output for the led
	int buttonPin = 2;  //declares the output for the startbutton

	int mod = 0;  //declares the modifier value as a global variable

		int iteration1 = 1;  //declares "iteration," which keeps track of the number of times the if statement has triggered
		int iteration2 = 1;  //same for the other two if statements
		int iteration3 = 1;
		//int myIteration = 1;

		unsigned long offset1 = 0;  //declares "offset," which keeps track of the offset that each of the connected fireflies
	 	unsigned long offset2 = 0;  //are blinking at, calculated based on the values stored in the "input" arrays
	    unsigned long offset3 = 0;

	    unsigned long avgOffset = 0;  //the average offset value, this is used to judge how close this firefly is to the correct offset value

	//uses "unsigned long" format since these arrays store time. unsigned longs can store values twice as large(up to 2^32 - 1) as a normal long, but can only store possitive values
	unsigned long input1[80][2];  //declares the 3, 2D input arrays of size 80 to store 80 state changes and the time they occur
	unsigned long input2[80][2];  //first layer stores the time the change occured
	unsigned long input3[80][2];  //second layer stores if it is on(1 for on, 0 for off)

	//unsigned Long myData[80][2];  //declares an identical array to keep track of the times for this firefly

	int ledState = LOW;  //ledState used to set the LED
	unsigned long previousMillis = 0;  //stores the last time LED was updated

	bool started = false;  //boolean for starting the frequency calculations only once there are enough data points to operate with
	bool button = false;  //boolean for starting the "loop" portion of the program once the button has been pressed

	void setup() {

	    pinMode(ledPin, OUTPUT);  //sets the "ledpin" to an output, so it can output to the LED
	    pinMode(buttonPin, INPUT);  //sets the "buttonpin" to an input, so it can take in values from the button

	//the for loop zeroes the arrays when the program starts
	    for(int i = 0; i < 80; i++){
	        for(int j = 0; j < 2; j++){
	            input1[i][j] = 0;
	            input2[i][j] = 0;
	            input3[i][j] = 0;
	            //myData[i][j] = 0;
	        }
	    }

	    Serial.begin(9600);  //starts, and sets the baud rate of the serial monitor

	    Serial1.begin(9600);  //sets up the other three serail communication lines
	    Serial2.begin(9600);  //which will be used to communicate with the other 3 linked arduino's
	    Serial3.begin(9600);
	  
	    Serial.println("******************************************************************"); //a break to signal the start of the program on the serial monitor
	}

	void loop(){

		if(digitalRead(buttonPin) == HIGH){  //a simple if statement to start the rest of "loop" when the button (on pin2) is pressed
			delay(1000);
			button = true;
			previousMillis = millis() + OFFSET;  //this resets the "previousMillis" to the current time, modified for the initial offset, so each firefly restarts its counter, preventing built up flashes all occuring at once
		}

		while(button){  //starts main portion of the program when the button is pressed

			iteration1 = checkPort(input1, 1, iteration1);  //checks to see if any of the ports 123 have changed in state
			iteration2 = checkPort(input2, 2, iteration2);
			iteration3 = checkPort(input3, 3, iteration3);

			timeToBlink();

			printSht();

			updateAvg();

			shiftMod();
		  
		    //failsafe stops the program from running if the max value of the unsigned int format is exceded, or if the capacity of the arrays are exceded, by going into an infinite while loop
		    if((millis() >= 4294967295) || (iteration1 >=80) || (iteration2 >=80) || (iteration3 >=80)) excededTime();
	    }//end bracket for the while loop
	}//end bracket for the loop

	void excededTime(){  //this function is called whenever there is no more space in the array, or if the time counter has been exceded
	    Serial.println("time either exceded 136 years (lol), or there isn't enough space in the arrays - need a larger variable format to store more data");  // outputs information to the serial monitor as a reminder

	    while(true){  //infinite loop that blinks the led at a fairly quick pace to give a visual
	        if(millis()%500 == 0){
	            if(ledState==LOW) ledState = HIGH;  //every 1 second, switches the led from off to on
	            else if(ledState==HIGH) ledState = LOW;  //or on to off, the "else" is key to prevent both cases from running
	            digitalWrite(ledPin, ledState);  //then writes the change to the led
	            //Serial.println("*");  //then a print statement to signal that it timed out
	        }
	    }
	}

	void synchronized(){  //called when the firefly is synched up with all adjacent fireflies
	                      //will apply a more adaptive method later
	    while(true){      //blinks the led at the synchronzed frequency, but does so in a way that lines up all of the blinking
		    if(millis()%(FREQUENCY) == 0){  //since the fireflies are all sync'd up, this time fram will be the same across each firefly
		        if(ledState==LOW) ledState = HIGH;
		        else if(ledState==HIGH) ledState = LOW;
		        digitalWrite(ledPin, ledState);  //same logic as the exededTime function
		    }
	    }
	}

	int checkPort(unsigned long input[][2], int port, int iteration){  //generalized function for reading and recording data from the ports

	    unsigned long temp = input[iteration-1][1]; //sets "temp" to the last recorded state of the given firefly (either on or off)
	    //unsigned long time = millis();  //an intermediate variable for recording

	    if((port == 1) && (Serial1.available())){  //if the firefly we're looking at is on port 1, AND there is data stored in that port's buffer:
	  		int readValue = Serial1.read();

	  		Serial.print("111the recieved value is: ");
	  		Serial.println(readValue);

	  	    if((readValue == 1) && (temp == 0)){  //if the first byte read = 1, and the last recorded state of said firefly is 0:
	  	     										//then a change of state must have occured in said firefly (in this case off to on)
			    input[iteration][0] = millis();  //then record the time this state change occured
			    input[iteration][1] = 1; 		   //and record that the firefly is now on
			    iteration++;					   //note that one more peice of data has been recorded, we're keeping track of this to prevent over recording data in the array
		  		//Serial.println("iteration1 should increase");
		    }

		    else if((readValue == 0) && (temp == 1)){ //then if the previous "if statement" was invalid,
		    									      //checks to see if the firefly we're looking at has gone from on to off
			    input[iteration][0] = millis();   //records the time this occured
			    input[iteration][1] = 0;			//records that said firefly is now off
			    iteration++;						//again, notes that one more piece of data has been recorded, this also allows us to keep track of which slot in the array we're on
		  		//Serial.println("iteration1 should increase");
		    }
		    Serial.print("111the time recorded is: ");  //print statements for troubleshooting
		    Serial.println(input[iteration-1][0]);  //since iteration++ happens, this is actually the current value
		    //Serial.print("temp1 = ");
	        //Serial.println(temp);
	    }

	    else if((port == 2) && (Serial2.available())){  //if the firefly we're looking at is on port 2, AND there is data stored in that port's buffer:
	  		
	  		int readValue = Serial2.read();

	  		Serial.print("222the recieved value is: ");
	  		Serial.println(readValue);

	  	    if((readValue == 1) && (temp == 0)){  //if the first byte read = 1, and the last recorded state of said firefly is 0:
	  	   										  //then a change of state must have occured in said firefly (in this case off to on)
			    input[iteration][0] = millis();   //then record the time this state change occured
			    input[iteration][1] = 1; 		  //and record that the firefly is now on
			    iteration++;					  //note that one more peice of data has been recorded, we're keeping track of this to prevent over recording data in the array
		 		//Serial.println("iteration2 should increase");
		    }

		    else if((readValue == 0) && (temp == 1)){ //then if the previous "if statement" was invalid,
		   											 //checks to see if the firefly we're looking at has gone from on to off
			    input[iteration][0] = millis();   //records the time this occured
			    input[iteration][1] = 0;			//records that said firefly is now off
			    iteration++;						//again, notes that one more piece of data has been recorded, this also allows us to keep track of which slot in the array we're on
		 		//Serial.println("iteration2 should increase");
		    }
		    Serial.print("222the time recorded is: ");  //print statement for troubleshooting
		    Serial.println(input[iteration-1][0]);
		    //Serial.print("temp2 = ");
		    //Serial.println(temp);
	    }

	    else if((port == 3) && (Serial3.available())){  //if the firefly we're looking at is on port 3, AND there is data stored in that port's buffer:
	  
	  		int readValue = Serial3.read();

	  		Serial.print("333the recieved value is: ");
	  		Serial.println(readValue);

	  	    if((readValue == 1) && (temp == 0)){  //if the first byte read = 1, and the last recorded state of said firefly is 0:
	  	     									 //then a change of state must have occured in said firefly (in this case off to on)
			    input[iteration][0] = millis();  //then record the time this state change occured
			    input[iteration][1] = 1; 		 //and record that the firefly is now on
			    iteration++;					 //note that one more peice of data has been recorded, we're keeping track of this to prevent over recording data in the array
			    //Serial.println("iteration3 should increase");
		    }

		    else if((readValue == 0) && (temp == 1)){ //then if the previous "if statement" was invalid,
	    										      //checks to see if the firefly we're looking at has gone from on to off
			    input[iteration][0] = millis();       //records the time this occured
			    input[iteration][1] = 0;			  //records that said firefly is now off
			    iteration++;						  //again, notes that one more piece of data has been recorded, this also allows us to keep track of which slot in the array we're on
		 		//Serial.println("iteration3 should increase");
		    }
		    Serial.print("333the time recorded is: ");  //print statements for troubleshooting
		    Serial.println(input[iteration-1][0]);
		    //Serial.print("temp3 = ");
	  	    //Serial.println(temp);
	    }
		return iteration;  //returns the "iteration" to make sure that iteration is being passed back to the loop
	}//close bracket for the funtion

	void timeToBlink(){
	// check to see if it's time to blink the LED: if the difference between the current time and last time the LED blinked is bigger than the interval value
		unsigned long currentMillis = millis();

		//THIS IS THE SPOT THAT I NEED TO FIX - THE OFFSET VALUE MUST BE CORRECTLY FACTORED IN TO THIS PART, NOT SURE HOW YET

		if ((currentMillis - previousMillis) >= (FREQUENCY+mod)){  //if the time passed since the previous blink equals the blink frequency, then led changes states
			previousMillis = currentMillis;     // save the last time the LED blinked
			int send = 0;  //an intermediate variable for sending data

			if (ledState == LOW){
				send = 1;
			    ledState = HIGH;  // if the LED is off turn it on and vice-versa:
			} 

			else{
				send = 0;
			   	ledState = LOW;
			}

			digitalWrite(ledPin, ledState);  // set the LED with the ledState of the variable:
			Serial1.write(send);  //then broadcasts that the led has changed state, in real life this would be done with a sense like sight
			Serial2.write(send);  //but here we've simplified and streamlined the process of "sight" for communication
		    Serial3.write(send);

		    //myData[myIteration][0] = millis();  //records the time change for this firefly, in the same foramt as the other fireflies
		    //myData[myIteration][1] = send;
		    //myIteration++;
		}
	}

	void printSht(){
		if(millis()%1000==0){  //print statements for troubleshooting
			Serial.print("the offset of the other fireflies are: ");
			Serial.println(offset1);
			Serial.println(offset2);
			Serial.println(offset3);

			Serial.print("the iterations of the other fireflies are: ");
			Serial.println(iteration1);
			Serial.println(iteration2);
			Serial.println(iteration3);

			Serial.print("the avgOffset is: ");
			Serial.println(avgOffset);

			Serial.print("FREQUENCY + mod = ");
			Serial.println(FREQUENCY + mod);
		}
	}

	void updateAvg(){
		if(iteration1 > 2){  //if the first array has stored at least 2 data points, then calculates the offsets by taking the taking the remainer of the division of the last recorded value and the time interval: FREQUENCY
		 	offset1 = (input1[iteration1-1][0])%FREQUENCY;
		}

		if(iteration2 > 2){
			offset2 = (input2[iteration2-1][0]) % FREQUENCY;
		}

		if(iteration3 > 2){
			offset3 = (input3[iteration3-1][0]) % FREQUENCY;
		}


		if((iteration1>2) && (iteration2>2) && (iteration3>2)){  //an extra if case to tell the program when there are enough data points to do calculations
		   	started = true;
		}
		  
		if((started)){  //calculates the average to evaluate how synchronized this firefly is
		    avgOffset = ((offset1+offset2+offset3+OFFSET+mod)/4);  //calculates the arithmatic mean rounded off to a whole number(in milliseconds) - also, averages all connected fireflies AND itself to avoid differences in data.
		}
	}

	void shiftMod(){
		if((started) && (avgOffset) < (OFFSET + mod)) mod++;  //shifts the frequency of the firefly longer by a tenth of a second if the average is larger

		else if((started) && (avgOffset) > (OFFSET + mod)) mod--;  //shifts the frequency of the firefly down if the average is shorter

		else if((started) && (avgOffset) == (OFFSET + mod)){ //CHANGE THIS LATER TO CHECK EQUIVALENCE TO EACH CONNECTION INDEPENDANTELY
		    Serial.println("SYNCHRONIZED!");
		    synchronized();
		}  //if all of the frequencies are equivalent, then enters the "synchronized" function - will switch to a more adaptable approach later

	}
