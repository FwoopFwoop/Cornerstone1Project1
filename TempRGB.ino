//--------------------INITIALIZATIONS--------------------

//Analog pin receiving thermometer input
const int tempPin = 0; 

//Second thermometer for checking if heating pad is ready
const int tempPin2 = 1;

//Digital pin receiving button input
const int buttonPin = 0;

//Output pins for displaying to the RGB output
const int redPin = 13;
const int greenPin = 12;
const int bluePin = 11;
  
//Interval constants for time control, all in milliseconds
const int readInterval = 1000; 
const int flashInterval = 500;

//Variable for storing the current time from the start of a measurement
int timer = 0;

//Flow control booleans
bool serialOn;
bool serialInit;
bool heatingUp;
bool checkingMetal;

//Calculation reference data
const int padReadyTemp = 50; //TODO: FIND. THIS IS TEMPORARY FOR TESTING PURPOSES 

//Enum for the different color options on the RGB LED
enum color{
  off,
  red,
  green,
  blue,
  yellow,
  magenta,
  cyan,
  white
};

//Value that stores the current color of the LED
color currentColor;

//---------------------FUNCTIONS---------------------------

//Called once at start of program
void setup() {
  //Set digital pin modes
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(buttonPin, INPUT);

  //Set default states for program flow control
  serialOn = false;
  serialInit = false;
  heatingUp = true;
  checkingMetal = false;

  //Set default state of the LED
  currentColor = off;
}

//Main loop called repeatedly
void loop() {

  //Read button state
  int buttonState = digitalRead(buttonPin);

  //Button behaviour
  if(buttonState==LOW){
    serialOn = true;
  }

  //Control flow by running one of several modes
  if(heatingUp) heatUp();
  else if (serialOn) serialTemp();
  else if (checkingMetal) checkMetal();

  //Update the LED color
  setLED();
}

//Runs the temperature analysis on the coin currently in the device
void checkMetal(){
  
}

//Runs until the heating pad is warm enough to function
void heatUp(){
  //Flash red and yellow to indicate the pad is heating up
  currentColor = (currentColor==red)? yellow : red;

  //If the pad is heated up, turn on blue "Ready to Read" LED, then exits heating mode
  if(getDegreesC(tempPin2)>=padReadyTemp){
    currentColor = blue;
    heatingUp = false;
    return;
  }

  delay(flashInterval);
}

//Changes the LED to the required color
void setLED(){
  switch(currentColor){
    case off:
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, LOW);
      digitalWrite(bluePin, LOW);
      break;
    case red:
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, LOW);
      digitalWrite(bluePin, LOW);
      break;
    case green:
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, HIGH);
      digitalWrite(bluePin, LOW);
      break;
    case blue:
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, LOW);
      digitalWrite(bluePin, HIGH);
      break;
    case magenta:
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, LOW);
      digitalWrite(bluePin, HIGH);
      break;
    case yellow:
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, HIGH);
      digitalWrite(bluePin, LOW);
      break;
    case cyan:
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, HIGH);
      digitalWrite(bluePin, HIGH);
      break;
    case white:
      digitalWrite(redPin, HIGH);
      digitalWrite(greenPin, HIGH);
      digitalWrite(bluePin, HIGH);
  }
}

//Initializes serial data gathering
void initSerial(){
  Serial.begin(9600);
  serialInit = true;
}

//Returns the time and temperature to the serial display
void serialTemp(){

  if(!serialInit) initSerial();

  Serial.print(getDegreesC(tempPin));
  Serial.print(" C, ");
  Serial.print(timer);
  Serial.println(" ms");

  //Flash the led cyan and magenta
  currentColor = (currentColor==magenta)? white : magenta;

  //Increment the timer
  timer+=readInterval;

  //Wait an interval before reading again
  delay(readInterval);
}

//Returns the temperature of the thermometer in degrees celcius
double getDegreesC(int pin){
  //Read the data off of the thermometer
  double voltage = getVoltage(pin); 
  
  //Convert the voltage reading into Celcius
  double degreesC = (voltage - 0.5) * 100.0;

  return degreesC;
}

//Returns the analog read of the pin as a voltage
double getVoltage(int pin){
  return (analogRead(pin) * 0.004882814);
}

