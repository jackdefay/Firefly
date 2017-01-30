//FireflyFlareV1.ino

#define BASEVOLTAGE 0.5
#define MAXVOLTAGE 2.2

int led = 13;  //declare the pin for the led, on the mega this allows for pwm control

void setup() {

    pinMode(led, OUTPUT);

    randomSeed(analogRead(0));

    Serial.begin(115200);  //initiates the serial monitor
    Serial1.begin(115200);  //initiates the other 3 serial ports to listen to the connected "fireflies"
    Serial2.begin(115200);
    Serial3.begin(115200);

    Serial.println("*****************************************************");  //to signal the start of the program

    double pwm = convert2PWM(BASEVOLTAGE);
    analogWrite(led, pwm);
}

void loop() {

	if(millis()%100 == 0) start();  //randomly checks if its time to instigate a flare

	checkPort1();  //listen to the three ports on loop
	checkPort2();
	checkPort3();
}

void checkPort1(){
	if(Serial1.available()){  //only reads from the port if there is available data in the buffer
		int readValue = Serial1.read();  //reads the first value stored in the serial buffer

		if(readValue == 1) flare();
	}
}

void checkPort2(){
	if(Serial2.available()){  //only reads from the port if there is available data in the buffer
		int readValue = Serial2.read();  //reads the first value stored in the serial buffer

		if(readValue == 1) flare();
	}
}

void checkPort3(){
	if(Serial3.available()){  //only reads from the port if there is available data in the buffer
		int readValue = Serial3.read();  //reads the first value stored in the serial buffer

		if(readValue == 1) flare();
	}
}

void start(){
	int randtemp = random(500);  //generates a random number between 0 and 500

	Serial.println(randtemp);

	if(randtemp == 57){  //if the random number = 57 (just a value I chose), then it sends the start signal. This gives it a 1 in 500 chance to start each time it loops
		flare();
	}
}

double convert2PWM(double voltage){
	double temp = (voltage/5)*255;

	return temp;
}

void flare(){

	delay(1000);  //waits two seconds

	Serial1.write(1);
	Serial2.write(1);
	Serial3.write(1);

	Serial1.flush();
	Serial2.flush();
	Serial3.flush();

	Serial1.end();
	Serial2.end();
	Serial3.end();

	//Serial.println("1");

	double pwm1 = convert2PWM(BASEVOLTAGE);  //converts voltage to pwm
	double pwm2 = convert2PWM(MAXVOLTAGE);

	int i = pwm1/1;  //baseline voltage as a pwm input, rounds off to the neares integer pwm
	int j = pwm2/1;  //max voltage as a pwm input

	int delaytime = (5000/(j-i));  //so the flare always takes 5 seconds, rounds off to the nearest millisecond

	for(i; i<=j; i++){  //flares
		analogWrite(led, i);
		delay(delaytime);
	}

	digitalWrite(led, LOW);  //then writes the led to turn off

	delay(10000);  //once the flare has finished, waits 10 seconds in "hibernation" before watching the other fireflies again

	analogWrite(led, pwm1);  //then writes the led to its base voltage

	Serial1.begin(115200);
	Serial2.begin(115200);
	Serial3.begin(115200);
}