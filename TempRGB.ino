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

//Output pin for buzzer noises
const int buzzerPin = 9;

//Interval constants for time control, all in milliseconds
const int readInterval = 1000; 
const int flashInterval = 500;

//How many intervals the analysis will sample the temperature
const int samples = 30;

//Variable for storing the current time from the start of a measurement
int timer = 0;


//Flow control booleans
bool serialOn;
bool serialInit;
bool heatingUp;
bool checkingMetal;

//Temperature value for storing data during heatup sequence
double padTemp;
//Minimum "heated" temperature
const double readyTemp = 19;

//Known values for silver coin, based off of observed data
const double silverSlope = 0.0492;

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
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT);  

  //Set default states for program flow control
  serialOn = false;
  serialInit = false;
  heatingUp = true;
  checkingMetal = false;

  //Defualt reading of the heating pad
  padTemp = getDegreesC(tempPin2);

  //Set default state of the LED
  currentColor = off;
}

//Main loop called repeatedly
void loop() {

  //Read button state
  int buttonState = digitalRead(buttonPin);

  //Button behaviour
  if(buttonState==LOW){
    //serialOn = true;
    checkingMetal = true;
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

  Serial.begin(9600);
  
  //Create array to store the temperature readings
  double reading[samples];

  //Wait a second or two to actually read because when the
  //coin and sensor first touch they exchange some heat

  delay(readInterval*2);

  //For each sample that needs to be taken
  for(int i = 0; i<samples; i++){
    //Flash the LED
    currentColor = (currentColor==magenta)? blue:magenta;

    //In this case, we need to actually set the color here because
    //we never return to the main loop
    setLED();
    
    //Wait the read interval
    delay(readInterval);
    
    //Store the temperature difference between coin and pad
    reading[i] = getDegreesC(tempPin2) - getDegreesC(tempPin);

    Serial.print(i);
    Serial.print(" ");
    Serial.println(reading[i]);
  }

  //Analysis
  //I am using a trendline algorithim I learned from http://classroom.synonym.com/calculate-trendline-2709.html
  //The code implementation is my own

  //a is the number of samples times the sum of x*y for each (x,y)
  //b is the sum of all x values times the sum of all y values
  //c is the number of samples times the sum of all squared x values
  //d is the sum of all x values squared
  
  double a = 0.0;
  double b;
  double c = 0.0;
  double d;
  
  double xsum = 0.0;
  double ysum = 0.0;
  
  for(int i =0; i<samples; i++){
    a += (i * 1.0)* reading[i];
    ysum += reading[i];
    xsum += i;
    c += i*i;
  }
  
  a*= samples;
  b = xsum*ysum;
  c*= samples; 
  d = xsum*xsum;

  //Trendline slope is (a-b)/(c-d)
  double slope = abs(((a-b)*1.0)/((c-d)*1.0));
  
  //If slope is less than or equal to .4, it is probably silver
  if(slope<=.04){
    goodCoin();
  }
  //Otherwise, if it is on (.4,.5) it might be
  if(slope<=.05){
    maybeCoin();
  }
  //Otherwise, probably fake
  else{
    badCoin();
  }
  
  Serial.println(slope);
  checkingMetal = false;
}

//Called when the coin is believied to be silver
void goodCoin(){
  currentColor = green;
  goodSong();
}

//Called when the coin is believed to be fake
void badCoin(){
  currentColor = red;
  badSong();
}

void maybeCoin(){
  currentColor = yellow;
}

//Runs until the heating pad is warm enough to function
void heatUp(){
  //Flash red and yellow to indicate the pad is heating up
  currentColor = (currentColor==red)? yellow : red;

  //If the pad temperature is no longer increasing very much,
  //And is relatively warm,it is done heating up.
  if(abs(padTemp-getDegreesC(tempPin2)<.6 && padTemp>=readyTemp){
    currentColor = blue;
    heatingUp = false;
    readySong();
  }else{
    padTemp = getDegreesC(tempPin2);
    delay(flashInterval); 
  }
}

//Plays a song to indicate the coin is probably good
void goodSong(){
  char notes[] = "eaCbaEDbaCbgwe"; 
  int beats[] = {1,1,1,1,2,1,3,3,1,1,1,2,1,4};
  int songLength = 14;
  int tempo = 240;

  int i, duration;

  for (i = 0; i < songLength; i++){

    //How long the note should play based on the note and tempo
    duration = beats[i] * tempo;

    //If the note is a rest, pause
    if (notes[i] == ' '){
      delay(duration);  
    }
    //Otherwise, play the note
    else{
      tone(buzzerPin, frequency(notes[i]), duration);
      delay(duration); 
    }
    
    delay(tempo/10); 
  }
}

//Plays a song to indicate the pad is heated
void readySong(){
  char notes[] = "gcrfgcrfd"; 
  int beats[] = {6, 6, 1, 1, 4, 4, 1, 1, 6};
  int songLength = 9;
  int tempo = 280;

  int i;
  int duration;

  for (i = 0; i < songLength; i++){

    //How long the note should play based on the note and tempo
    duration = beats[i] * tempo;

    //If the note is a rest, pause
    if (notes[i] == ' '){
      delay(duration);  
    }
    
    //Otherwise, play the note
    else{
      tone(buzzerPin, frequency(notes[i]), duration);
      delay(duration); 
    }
    
    delay(tempo/10); 
  }
}

//Plays a song to indicate the coin is probably bad
void badSong(){
  char notes[] = "aaafCafCaEEEFCqfCa"; 
  int beats[] = {2, 2, 2, 1, 1, 2, 1, 1, 4, 2, 2, 2, 1, 1, 2, 1, 1, 2};
  int songLength = 18;
  int tempo = 206;

  int i;
  int duration;

  for (i = 0; i < songLength; i++){

    //How long the note should play based on the note and tempo
    duration = beats[i] * tempo;

    //If the note is a rest, pause
    if (notes[i] == ' '){
      delay(duration);  
    }
    
    //Otherwise, play the note
    else{
      tone(buzzerPin, frequency(notes[i]), duration);
      delay(duration); 
    }
    
    delay(tempo/10); 
  }
}

//Returns a numeric frequency given an alphabetic note
int frequency(char note){
  // This function takes a note character and returns the
  // corresponding frequency in Hz for the tone() function.
  int i;
  const int numNotes = 13;  // number of notes we're storing

  //Each letter name corresponds to an array frequency
  //Capital letters distinguish higher versiins of notes
  //Seemingly random letters like r, q, and w represent accidentals

  char names[] = {     'c', 'd', 'r', 'e', 'f', 'g', 'q', 'a', 'w', 'b', 'C','D' ,'E'};
  int frequencies[] = {262, 294, 311 ,330, 349, 392, 415, 440, 466, 494, 523, 587, 659};

  // Now we'll search through the letters in the array, and if
  // we find it, we'll return the frequency for that note.

  //For each member of the list of notes
  for (i = 0; i < numNotes; i++){ 
    //If the desired note at this point in the list
    if (names[i] == note){    
      //Return the corresponding frequency     
      return(frequencies[i]);     
    }
  }
  
  //If no frequency is found, return 0
  return(0); 
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
  Serial.print(" ");
  Serial.print(getDegreesC(tempPin2));
  Serial.print(" ");
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
  //Converts the 0-1023 analog range into volts
  return (analogRead(pin)  * (3.3/1023.0));
}

