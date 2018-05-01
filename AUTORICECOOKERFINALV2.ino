#include <LiquidCrystal.h>

//////// CONSTANTS AND GLOBALS /////////
const String NAME = "Auto Rice Cooker";
const String WELCOME_MESSAGE = "  Press Select  ";
const String BLANK = "                ";

//LCD Display
LiquidCrystal lcd (12, 11, 5, 4, 3, 2);

//Selected outlet (0, 1, 2)
int selectedOutlet = 0; 

// Time Adjustment (minutes)
const int LARGETIME = 30; 
const int SMALLTIME = 5; 

// Controls for outlet. 
const int OUT1PIN = 10;
const int OUT2PIN = A5; 


// Input buttons
const int LARGETIMEPIN = 6;
const int SMALLTIMEPIN = 7;
const int SWITCHSTARTPIN = 8;
const int SWITCHMODEPIN = 9;
const int SWITCHSELECTPIN = 13;  

//Tracking Time
const int DELAY = 50; //Updating DELAY;

//Time remaining on each runtime, each outlet. (minutes)
int delay1 = 0;
int delay2 = 0;
int run1 = 0;
int run2 = 0;
boolean is1On = false;
boolean is2On = false;
boolean is1Power = false;
boolean is2Power = false;
boolean editing1 = false;
boolean editing2 = false;
boolean delay1Selected = false; //delay selected or rt selected? (must be in editing...)
boolean delay2Selected = false;
 
//Counts up to 1 minute before subtracting that time from tiem remaining
//(milliseconds)
double counter1 = 0;
double counter2 = 0;

// Mode //Modes for each outlet
const int TIMEMODE = 0;
const int DELAYMODE = 1;
int mode1 = 0;
int mode2 = 0;
const String TIMEMODENAME = "TIMER";
const String DELAYMODENAME = "DELAY";

const int RT = 0;
const int DT = 1;

const String printRT = "RT";
const String printDT = "DT";

char showingDtRt = DT; // Default is DT

// Button Variables (Keep track of pressed down or not)
int buttonLargeTimeState = 0;
int lastButtonLargeTimeState = 0;

int buttonSmallTimeState = 0;
int lastButtonSmallTimeState = 0;

int buttonStartState = 0;
int lastButtonStartState = 0;

int buttonModeState = 0;
int lastButtonModeState = 0;

int buttonSelectState = 0;
int lastButtonSelectState = 0;

boolean firstSelect = true;

//////// FUNCTIONS ////////

void setup() {
  //Setup LCD
  lcd.begin(16, 2);
  lcd.setCursor(0,0);
  lcd.print(NAME);
  lcd.setCursor(0,1);
  lcd.print(WELCOME_MESSAGE);
  lcd.autoscroll();

  //Setup pins
  pinMode(OUT1PIN, OUTPUT);
  pinMode(OUT2PIN, OUTPUT);
  digitalWrite(OUT1PIN, HIGH);
  digitalWrite(OUT2PIN, HIGH);

  pinMode(LARGETIMEPIN, INPUT);
  pinMode(SMALLTIMEPIN, INPUT);
  pinMode(SWITCHSTARTPIN, INPUT);
  pinMode(SWITCHMODEPIN, INPUT);
  pinMode(SWITCHSELECTPIN, INPUT);

  //START THIS SHIT!
  Serial.begin(9600);
}

int startT, stopT; //Keep track of start an stop time... minimizes error.

//RUNNING LOOP.
void loop() {
  if(!firstSelect){
    lcd.noAutoscroll();
    startT = millis();
  
    checkButtons();
    updateTimeRemaining();
    updatePrint();
  
    stopT = millis();

    int i = (DELAY - (stopT-startT));
    //Serial.println ("Delay Time is " + String(i));
    if(i > 0){
      delay(i);
    }else{
      Serial.println("TIME TO COMPUTE EXCEEDED " + String(DELAY) + " milliseconds");
    }
  }else{
    readSelect();
    delay(DELAY); //Just stall until the mode button is pressed "initiating" the program
  }
}

void checkButtons(){
  readLarge();
  readSmall();
  readStart();
  readMode();
  readSelect();
}

void updateTimeRemaining(){
  if(is1On){
    counter1 += DELAY;
    if(counter1 == 60000){ // 1 MINUTE
      if(delay1 > 0){
        delay1 --;
      }else if(run1 > 0){
        run1 --; 
      }
      counter1 = 0;
    }
    if(delay1 == 0 && !is1Power){
      turnOn(1);
    }
    if(run1 == 0){
      stopTimer(1);
    }
  }
  
  if(is2On){
    counter2 += DELAY;
    if(counter2 == 60000){ // 1 MINUTE
      if(delay2 > 0){
        delay2 --;
      }else if(run2 > 0){
        run2 --; 
      } 
      counter2=0;
    }
    if(delay2 == 0 && !is2Power){
      turnOn(2);
    }
    if(run2 == 0){
      stopTimer(2);
    }
  }
}

void clearScreen(){
  lcd.setCursor(0,0);
  lcd.print(BLANK);
  lcd.setCursor(0,1);
  lcd.print(BLANK);
}

// THIS SHIT IS UGLY BUT THERE'S NOTHING I CAN DO ABOUT IT XDXD
void updatePrint(){
  clearScreen();
  //Initiate Print Variables... 
  String r1, r2; //Row
  String m1, m2; //Mode
  String n1, n2; //Time mode (DT/RT)
  String t1, t2; //Time left (HH:MM)
  
  if(mode1 == TIMEMODE){
    m1 = TIMEMODENAME;
  }else{
    m1 = DELAYMODENAME;
  }
  if(mode2 == TIMEMODE){
    m2 = TIMEMODENAME;
  }else{
    m2 = DELAYMODENAME;
  }

  if((showingDtRt == DT) && (delay1 > 0)){ 
    n1 = printDT;
    t1 = convertTime(delay1);
  }else{
    n1 = printRT;
    t1 = convertTime(run1);
  }
  if((showingDtRt == DT) && (delay2 > 0)){
    n2 = printDT;
    t2 = convertTime(delay2);
  }else{
    n2 = printRT;
    t2 = convertTime(run2);
  }
  
  if(editing1){
    if(mode1 == DELAYMODE){
      r1 = "1-" + m1 + " DT " + convertTime(delay1);
      r2 = "        RT " + convertTime(run1);
    }else{
      r1 = "1-" + m1 + " RT " + convertTime(run1);
      r2 = BLANK;
    }
  }else if(editing2){
    if(mode2 == DELAYMODE){
      r1 = "2-" + m2 + " DT " + convertTime(delay2);
      r2 = "        RT " + convertTime(run2);
    }else{
      r1 = "2-" + m2 + " RT " + convertTime(run2);
      r2 = BLANK;
    }
  }else{
    //Default Display
    if(is1On){
      r1 = "1-" + m1 + " " + n1 + " " + t1;
    }else{
      r1 = "1 is Stopped";
    }
    if(is2On){
      r2 = "2-" + m2 + " " + n2 + " " + t2;  
    }else{
      r2 = "2 is Stopped";
    }
  }
  
  lcd.setCursor(0,0);
  lcd.print(r1);
  lcd.setCursor(0,1);
  lcd.print(r2);
}

void addTime(int t){
  if(selectedOutlet == 1){
    if(delay1Selected){
      delay1 += t;
      if(is1Power){
        turnOff(1);
      }
    }else{
      run1 += t;
    }
  }else if(selectedOutlet == 2){
    if(delay2Selected){
      delay2 += t;
      if(is2Power){
        turnOff(2);
      }
    }else{
      run2 += t;
    }
  }else{
    Serial.println("No timer selected, not adding " + String(t) + " minutes to any time");
  }
}

//Start/stop button was pressed... Adjusts the one that is currently selected
void startStop(){
  if(editing1){
    if(delay1Selected){
      delay1Selected = false; // switches to editing run1. 
    }else{
      if(is1On){
        stopTimer(1);
      }else{
        startTimer(1);
      }      
    }
  }else if(editing2){
    if(delay2Selected){
     delay2Selected = false; // switches to editing run2.  
    }else{
      if(is2On){
        stopTimer(2);
      }else{
        startTimer(2);
      }
    }
  }
}

void modePressed(){
  if(editing1){
    if(mode1 == TIMEMODE){
      mode1 = DELAYMODE;
      delay1Selected = true;
    }else{
      mode1 = TIMEMODE;
      delay1 = 0;
      delay1Selected = false;
    }
  }else if(editing2){
    if(mode2 == TIMEMODE){
      mode2 = DELAYMODE;
      delay2Selected = true;
    }else{
      mode2 = TIMEMODE;
      delay2 = 0;
      delay2Selected = false;
    }
  }else{
    if(showingDtRt == DT){
      showingDtRt = RT;
    }else{
      showingDtRt = DT;
    }
  }
}

void selectPressed(){
  if(firstSelect){
    firstSelect = false;
  }else{
    if(selectedOutlet == 0){
      selectedOutlet = 1;
      editing1 = true;
      if(mode1 == DELAYMODE){
        delay1Selected = true;
      }
    }else if(selectedOutlet == 1){
      selectedOutlet = 2;
      editing1 = false;
      editing2 = true;
      if(mode2 == DELAYMODE){
        delay1Selected = false;
        delay2Selected = true;
      }
    }else if(selectedOutlet == 2){
      selectedOutlet = 0;
      editing2 = false;
      delay2Selected = false;
    }
  }
}

//Starts running the timer... 
void startTimer(int outlet){
  if(outlet == 1){
    editing1 = false;
    selectedOutlet = 0;
    is1On = true;
  }else{
    editing2 = false;
    selectedOutlet = 0;
    is2On = true;
  }
}

//Starts the running timer... 
void turnOn(int outlet){
  if(outlet == 1){
    digitalWrite(OUT1PIN, LOW);
    is1Power = true; 
  }else{
    digitalWrite(OUT2PIN, LOW);
    is2Power = true; 
  }
  Serial.println("Timer " + String(outlet) + " is on");
}

void stopTimer (int outlet){
  if(outlet == 1){
    turnOff(1);
    editing1 = false;
    selectedOutlet = 0;
    is1On = false;
    delay1 = 0;
    run1 = 0;
    Serial.println("Timer 1 Ended");
  }else{
    turnOff(2);
    editing2 = false;
    selectedOutlet = 0;
    is2On = false;
    delay2 = 0;
    run2 = 0;
    Serial.println("Timer 2 Ended");  
  }
}

//Cuts power to outlet.
void turnOff (int outlet){
  if(outlet == 1){
    is1Power = false;
    digitalWrite(OUT1PIN, HIGH);
  }else{
    is2Power = false;
    digitalWrite(OUT2PIN, HIGH);
  }
}

//Given time in minutes, return string in from of HH:MM
String convertTime(int t){
  int i = t/60;
  int j = t%60;
  String h, m;
  
  if(i < 10){
    h = "0" + String(i);
  }else{
    h = String(i);
  }

  if(j<10){
    m = "0" + String(j);
  }else{
    m = String(j);
  }
  return h + ":" + m;
}

// Reading Buttons...
void readLarge(){
  buttonLargeTimeState = digitalRead(LARGETIMEPIN);
  if(buttonLargeTimeState != lastButtonLargeTimeState){
    if(buttonLargeTimeState == HIGH){
      Serial.println("LARGE TIME CLICKED");
      addTime(LARGETIME);
    }
  }
  
  lastButtonLargeTimeState = buttonLargeTimeState;
}

void readSmall(){
  buttonSmallTimeState = digitalRead(SMALLTIMEPIN);
  if(buttonSmallTimeState != lastButtonSmallTimeState){
    if(buttonSmallTimeState == HIGH){
      Serial.println("SMALL TIME CLICKED");
      addTime(SMALLTIME);
    }
  }
  lastButtonSmallTimeState = buttonSmallTimeState;
}

void readStart(){
 buttonStartState = digitalRead(SWITCHSTARTPIN);
  if(buttonStartState != lastButtonStartState){
    if(buttonStartState == HIGH){ // If Start button is pressed, 
      Serial.println("START STOP CLICKED");
      startStop();
    }
  }
  lastButtonStartState = buttonStartState;
}  

void readMode(){
  buttonModeState = digitalRead(SWITCHMODEPIN);
  if(buttonModeState != lastButtonModeState){
    if(buttonModeState == HIGH){
      Serial.println("MODE CLICKED");
      modePressed();
    }
  }
  lastButtonModeState = buttonModeState;
}

void readSelect(){
  buttonSelectState = digitalRead(SWITCHSELECTPIN);
  if(buttonSelectState != lastButtonSelectState){
    if(buttonSelectState == HIGH){ //If Select Button is pressed
      Serial.println("SELECT CLICKED");
      selectPressed();
    }
  }
  lastButtonSelectState = buttonSelectState;
}
