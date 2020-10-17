#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

/*********************************VARIABLEN**********************************/

//PIN-BELEGUNG
//Zelt-Lichtsensoren (analog)
int light_1 = A8;
int light_2 = A9;
int light_3 = A10;

//Zelt-LEDs (digital)
int led_1 = 30;
int led_2 = 32;
int led_3 = 34;

//Sound
SoftwareSerial mySoftwareSerial(10, 11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

//Lautsprecher
int audio_1 = 7;
int audio_2 = 6;
int audio_3 = 5;

//Barometer-LEDs (digital)
int score_leds[] = {45, 47, 49, 51, 53};

//SENSOREN
//Grenzwerte der Lichtsensoren
int thresh_1, thresh_2, thresh_3;

//Messwerte der Lichtsensoren 
int val_1, val_2, val_3;

//LOGIK
//Punktestand
int score = 0;

//Counter für Sounds
int countHit = int(random(1, 5)); //hit sound 1-4
int countFail = int(random(5, 9)); //fail sounds 5-8
int countWin = int(random(9, 12)); //win sounds 9-11

//Launch
boolean launched;
unsigned long lastLaunchTime = 10000000; ////////////////////////////change to 3000
unsigned long timeUntilFail = 3000;

/**************************SETUP UND LOOP**********************************/
void setup() {
 //SERIAL COM
 Serial.begin(9600);

 //SOUND
 setupSound();
 
 //LIGHT-BARRIERS
 setupLightBarriers();
 calibrateLightBarriers();

 //SCORE BAROMETER
 setupScoreBarometer();

 Serial.println("setup() complete");

}

void loop() {
    checkForHit();
    delay(100);

    if (myDFPlayer.available()) {
      printDetail(myDFPlayer.readType(), myDFPlayer.read()); //Print the detail message from DFPlayer to handle different errors and states.
  }
}
/**************************** SOUND **********************************/

void setupSound(){
  //set pins for audio
   pinMode(audio_1, OUTPUT);
   pinMode(audio_2, OUTPUT);
   pinMode(audio_3, OUTPUT);
   
   mySoftwareSerial.begin(9600);
  
   Serial.println();
   Serial.println(F("DFRobot DFPlayer Mini Demo"));
   Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

   if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));
  
  myDFPlayer.volume(25);  //Set volume value. From 0 to 30

  Serial.println("setupSound() complete");
}

//0 = all, 1,2,3 = tent
void playHitSound(int tent){
  digitalWrite(audio_1, LOW);
  digitalWrite(audio_2, LOW);
  digitalWrite(audio_3, LOW);

   switch(tent){
    case 0:
      digitalWrite(audio_1, HIGH);
      digitalWrite(audio_2, HIGH);
      digitalWrite(audio_3, HIGH);
      break;
    case 1:
      digitalWrite(audio_1, HIGH);
      break;
    case 2:
      digitalWrite(audio_2, HIGH);
      break;
    case 3:
      digitalWrite(audio_3, HIGH);
      break;  
    default:
      digitalWrite(audio_1, HIGH);
      digitalWrite(audio_2, HIGH);
      digitalWrite(audio_3, HIGH);  
  }

  //Play random sound between 0001 and 0004
  //int id = int(random(1, 5));
  playSound(countHit);
  countHit++;
  if (countHit == 5) {
    countHit = 1;
  }
  Serial.println("Tent ");
  Serial.print(tent);
  Serial.println("CountHit ");
  Serial.print(countHit);
  Serial.println("playHitSound() complete");
}

void playFailSound(){
  digitalWrite(audio_1, HIGH);
  digitalWrite(audio_2, HIGH);
  digitalWrite(audio_3, HIGH);

  //Play random sound between 0005 and 0008
  //int id = int(random(5, 9));
  playSound(countFail);
  countFail++;
  if (countFail == 9) {
    countFail = 5;
  }
  Serial.println("playFailSound() complete");
}

void playWinnerSound(){
  digitalWrite(audio_1, HIGH);
  digitalWrite(audio_2, HIGH);
  digitalWrite(audio_3, HIGH);

  //Play random sound between 0009 and 0011
  playSound(countWin);
  countWin++;
  if (countWin == 12) {
    countWin = 9;
  }
  Serial.println("playWinnerSound() complete");
}

void playSound(int id){
  myDFPlayer.playMp3Folder(id);

  Serial.println("playSound() complete");
}

//CODE AUS SOUND BEISPIEL
void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
  
}

/********************************WIN LOGIC***********************************/

void checkForHit(){
  if(millis() < lastLaunchTime + timeUntilFail){
    //CHECK FOR WIN
    //Serial.println("checkForHit() - Checking...");
    int tent = checkLightBarriers();
    if(tent != 0){
      //WIN
      Serial.println("checkForHit() - WIN detected");
      printSuccess(tent);
      adjustScore();
      //sound
      if (score<5) {
        playHitSound(tent);
        //playFailSound();
      }
      //set inactive
      launched = false;
    }
  } else {
    //FAIL
     Serial.println("checkForHit() - FAIL detected");
    //sound
    playFailSound();
    //set inactive
    launched = false;
   } 
   //Serial.println("checkForHit() complete");
}

void adjustScore(){
  score++;
  setScoreBarometer(score);
  if(score == 5){
    playWinnerSound();
    resetScore();
  }

  Serial.print("New score: ");
  Serial.println(score);
}


/********************************LIGHT BARRIERS***********************************/

void setupLightBarriers(){
   //Leds
   pinMode(led_1, OUTPUT);
   pinMode(led_2, OUTPUT);
   pinMode(led_3, OUTPUT);

   digitalWrite(led_1, HIGH);
   digitalWrite(led_2, HIGH);
   digitalWrite(led_3, HIGH);

   //Lichtsensoren
   pinMode(light_1, INPUT);
   pinMode(light_2, INPUT);
   pinMode(light_3, INPUT);

   Serial.println("setupLightBarriers() complete");
}

void calibrateLightBarriers(){
  //Lichtsensoren lesen
  readLightBarriers();

  Serial.print("val_1: ");
  Serial.println(val_1);
  Serial.print("val_2: ");
  Serial.println(val_2);
  Serial.print("val_3: ");
  Serial.println(val_3);

  //Grenzwerte setzen
  //thresh_1 = val_1 + 20;
  //thresh_2 = val_2 - 20;
  //thresh_3 = val_3 +100;

  thresh_1 = val_1 + 10;
  thresh_2 = val_2 - 10;
  thresh_3 = val_3 +80;

  Serial.print("thresh_1: ");
  Serial.println(thresh_1);
  Serial.print("thresh_2: ");
  Serial.println(thresh_2);
  Serial.print("thresh_3: ");
  Serial.println(thresh_3);

  Serial.println("calibrateLightBarriers() complete");
}

void readLightBarriers(){
  //Lichtsensoren lesen
  val_1 = analogRead(light_1);
  val_2 = analogRead(light_2);
  val_3 = analogRead(light_3);

//  Serial.print("Zelt 1: ");
//  Serial.println(val_1);
//  Serial.print("Zelt 2: ");
//  Serial.println(val_2);
//  Serial.print("Zelt 3: ");
//  Serial.println(val_3);
 //Serial.println("readLightBarriers() complete");
}

//return 0 for nothing, 1/2/3 for tent
int checkLightBarriers(){
  //Serial.println("checkLightBarriers()");
  //Lichtsensoren lesen
  readLightBarriers();

  //Treffer prüfen
  if(val_1 > thresh_1){
    return 1;
  } else if(val_2 < thresh_2){
    return 2;
  } else if(val_3 > thresh_3){
    return 3;
  }

  return 0; 
}


/***********SCORE BAROMETER**************/

void setupScoreBarometer(){
  for(int i = 0; i < 5; i++){
    pinMode(score_leds[i], OUTPUT);
    digitalWrite(score_leds[i], LOW);
  }

  Serial.println("setupScoreBarometer() complete");
}

void setScoreBarometer(int score){

  //old score items
  if(score > 1){
    for(int i = 0; i < score - 1; i++){
    digitalWrite(score_leds[i], HIGH);
    delay(100);    
    }
  }

  //new score item
  digitalWrite(score_leds[score - 1], HIGH);
  delay(300);
  digitalWrite(score_leds[score - 1], LOW);
  delay(300);
  digitalWrite(score_leds[score - 1], HIGH);
  delay(300);
  digitalWrite(score_leds[score - 1], LOW);
  delay(300);
  digitalWrite(score_leds[score - 1], HIGH);

  Serial.println("setScoreBarometer() complete");
}

void resetScore() {
  delay(25000); //für eine längere Party
  score = 0;
  //shut down all score LED's
  for(int i = 0; i < 5; i++){
    digitalWrite(score_leds[i], LOW);
  }
  Serial.println("resetScore() complete");
}

/********PRINTS**********/

void printSuccess(int tent_id){
  String tent;
  switch(tent_id){
    case 1:
      tent = "Schottenhamel";
      break;
    case 2:
      tent = "Hacker";
      break;
    case 3:
      tent = "Paulaner";
      break;
    default:
      tent = "Unbekannt";
  }
  Serial.print(tent);
  Serial.println(" getroffen!");
}

void printScore(){
  Serial.print("Punktestand: ");
  Serial.println(score);
}
