/*
 Controlling a servo position using a potentiometer (variable resistor)
 by Michal Rinott <http://people.interaction-ivrea.it/m.rinott>

 modified on 8 Nov 2013
 by Scott Fitzgerald
 http://www.arduino.cc/en/Tutorial/Knob
*/

#include <Servo.h>

/**************************VARIABLES********************************/

//Servos
Servo servo_richtung;
Servo servo_staerke;
Servo servo_launch;

//Launch-Servo Grenzwerte
int servo_launch_closed = 90;
int servo_launch_open = 0;

//Pin-Belegung
int pin_poti = A2;
int pin_own_switch = 5;

//Sensorwerte
int val_richtung;
int val_staerke;
int val_own_switch;

//Kommunikation zu Arduino Mega
int sendLaunch = 24;


/******************************SETUP UND LOOP*********************************/

void setup() {
  Serial.begin(4800);

  //PIN MODES
  pinMode(pin_poti, INPUT);
  pinMode(pin_own_switch, INPUT_PULLUP);
  pinMode(sendLaunch, OUTPUT);

  //SERVOS
  //servo_richtung.attach(11);  // attaches the servo on pin 9 to the servo object

  servo_staerke.attach(9);
  servo_staerke.write(0);
  //servo_staerke.write(30);

  servo_launch.attach(10);
  servo_launch.write(0);
  delay(1000);
  servo_launch.write(90);
  delay(100);
  
}

void loop() {
  readSensors();
  delay(100);

  
//  val_richtung = analogRead(potpin);            // reads the value of the potentiometer (value between 0 and 1023)
//  val_richtung = map(val, 0, 1023, 45, 135);     // scale it to use it with the servo (value between 0 and 180)
//  sweep(servo_richtung, servo_richtung.read(), val_richtung, 25);                  // sets the servo position according to the scaled value
//  delay(15);                           // waits for the servo to get there

//  val_staerke = analogRead(pin_poti);            // reads the value of the potentiometer (value between 0 and 1023)
//  val_staerke = map(val_staerke, 0, 1023, 0, 80);     // scale it to use it with the servo (value between 0 and 180)
//  sweep(servo_staerke, servo_staerke.read(), val_staerke, 25);                  // sets the servo position according to the scaled value
//  delay(15);                           // waits for the servo to get there
}

/***************************CATAPULT CONTROL**********************************/

void readSensors(){
  val_own_switch = digitalRead(pin_own_switch);
  if(val_own_switch == LOW){
    Serial.print("Own Switch active! Value: ");
    Serial.println(val_own_switch);

    //launchCatapult();

    //setup catapult
    servo_staerke.write(90);
    delay(2000);

    //open launcher
    servo_launch.write(0);
    //delay(100);

    //throw catapult
    servo_staerke.write(120);
    delay(2000);
    servo_staerke.write(0);
    delay(15);

    //close launcher
    servo_launch.write(90);
  }
}


void launchCatapult(){
  //Arduino Mega benachrichtigen
  //sendLaunchEvent();

  //Loslassen: 90° drehen
  servo_launch.write(0);
  delay(100);

  //Arm zurücksetzen (auf Ausgangspos)
  servo_staerke.write(0);
  delay(100);

  //Befestigen: 90° drehen
  servo_launch.write(90);
  delay(100);

  Serial.println("LAUNCHED");
  digitalWrite(sendLaunch, HIGH);
  delay(1000);
  digitalWrite(sendLaunch, LOW);
  Serial.println("sendLaunch finished");
}


void sweep(Servo servo, int from, int to, int speed) {
  int pos = 0 ;
  if (from < to) {
    for (pos = from; pos <= to; pos += 1) { 
      servo.write(pos);              
      delay(speed);                       
    }
  } else {
    for (pos = from; pos >= to; pos -= 1) { 
      servo.write(pos);              
      delay(speed);                       
    } 
  }
}
