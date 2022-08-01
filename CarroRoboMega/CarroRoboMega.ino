/*
*/

#include <Servo.h>
#include <NewPing.h>

void forward(void);
void back(void);
void left(void);
void right(void);
void stopAll(void);
int readPing(void);
void forwardRight(void);
void forwardLeft(void);

/*
 * Ultrasonic sensor
 */
#define TRIGPIN 8
#define ECHOPIN 9
const int maximumDistance = 100;
NewPing sonar(TRIGPIN, ECHOPIN, maximumDistance); //sensor function
int distance;
unsigned long previousMillis = 0;        // will store last time distance was updated
 
/*
 * Motor
 * Board              Pins                  Frequency
 * Uno, Nano, Mini    3, 5, 6, 9, 10, 11    490 Hz (pins 5 and 6: 980 Hz)
 * Mega               2 - 13, 44 - 46       490 Hz (pins 4 and 13: 980 Hz)
 */
unsigned char carSpeed = 0x70;  // 60-80
boolean movingForward = false;
// Left Motor
const int motorEnLeft = 6;
const int motorInLeft1 = 41;
const int motorInLeft2 = 42;
// Right Motor
const int motorEnRight = 5;
const int motorInRight3 = 39;
const int motorInRight4 = 40;

/*
 * Servo
 */
int servoPin = 7; // Declare the Servo pin 
Servo servo;      // Create a front servo object 

/*
 * Lights
 */
const int ledPinFrontRight = 10;  // the number of the LED pin
const int ledPinFrontLeft  = 10;
const int ledPinRearRight  = 12;  // the number of the LED pin
const int ledPinRearLeft   = 12;  // the number of the LED pin

/*
 * Horn
 */
const int hornPin = 11;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store

void setup() {
  // Serial setup
  Serial.begin(9600);
  Serial1.begin(9600);
  
  // Motor setup
  pinMode(motorInLeft1, OUTPUT);
  pinMode(motorInLeft2, OUTPUT);
  pinMode(motorInRight3, OUTPUT);
  pinMode(motorInRight4, OUTPUT);

//  pinMode(motorEnLeft, OUTPUT); 
//  pinMode(motorEnRight, OUTPUT); 

  // Servo setup
  servo.attach(servoPin); 

  // Light setup
  // set the digital pin as output:
  pinMode(ledPinFrontRight, OUTPUT);
  pinMode(ledPinFrontLeft, OUTPUT);
  pinMode(ledPinRearRight, OUTPUT);
  pinMode(ledPinRearLeft, OUTPUT);
  digitalWrite(ledPinFrontRight, LOW);
  digitalWrite(ledPinFrontLeft, LOW);
  digitalWrite(ledPinRearRight, LOW);
  digitalWrite(ledPinRearLeft, LOW);

  /*
   * Ultrasonic
   */
  pinMode(TRIGPIN, OUTPUT);
  pinMode(ECHOPIN, INPUT);
  delay(2000);
  distance = readPing();
  delay(100);
  distance = readPing();
  delay(100);
  distance = readPing();
  delay(100);
  distance = readPing();
  delay(100);
  
  Serial.println("Começando...\n");
}

void loop() {
  static char command;

  if(distance < 20) {
      digitalWrite(ledPinFrontRight, HIGH);
      digitalWrite(ledPinFrontLeft, HIGH);
      digitalWrite(ledPinRearRight, HIGH);
      digitalWrite(ledPinRearLeft, HIGH);
      Serial.println("Aqui 2");
      if(movingForward) stopAll();
  }

  command = Serial1.read();
  switch(command) {
    case 'b':
    case 'B': // move back
      servo.write(90);
      back();
      break;
    case 'D': // Stop all
      Serial.println("Aqui 3");
      stopAll();
      break;
    case 'f':
    case 'F': // move forward
      servo.write(90);
      forward();
      break;
    case 'G': // Move forward left
      servo.write(90+45);
      forwardLeft();
      break;
    case 'H': // Move back left
      servo.write(90);
      backLeft();
      break;
    case 'I': // Move forward right
      servo.write(90-45);
      forwardRight();
      break;
    case 'J': // Move back right
      servo.write(90);
      backRight();
      break;
    case 'L': // move left
      servo.write(180);
      left();
      break;
    case 'R': // Right
      servo.write(0);
      right();
      break;
    case 'S': // Stop
      Serial.println("Aqui 4");
      stopAll();
      break;
    case 'V': // horn on
      analogWrite(hornPin, 127);  // 50% duty cicle
      break;
    case 'v': // horn off
      digitalWrite(hornPin, 0);   // 0% duty cicle
      break;
    case 'W': // Front lights on
      digitalWrite(ledPinFrontRight, HIGH);
      digitalWrite(ledPinFrontLeft, HIGH);
      break;
    case 'w': // Front lights off
      digitalWrite(ledPinFrontRight, LOW);
      digitalWrite(ledPinFrontLeft, LOW);
      break;
    case 'U': // back lights on
      digitalWrite(ledPinRearRight, HIGH);
      digitalWrite(ledPinRearLeft, HIGH);
      break;
    case 'u': // back ligths off
      digitalWrite(ledPinRearRight, LOW);
      digitalWrite(ledPinRearLeft, LOW);
      break;
    case '0': // Speed 0;
      carSpeed = 255*0/100;
      break;
    case '1': // Speed 10;
      carSpeed = 255*10/100;
      break;
    case '2': // Speed 20;
      carSpeed = 255*20/100;
      break;
    case '3': // Speed 30;
      carSpeed = 255*30/100;
      break;
    case '4': // Speed 40;
      carSpeed = 255*40/100;
      break;
    case '5': // Speed 50;
      carSpeed = 255*50/100;
      break;
    case '6': // Speed 60;
      carSpeed = 255*60/100;
      break;
    case '7': // Speed 70;
      carSpeed = 255*70/100;
      break;
    case '8': // Speed 80;
      carSpeed = 255*80/100;
      break;
    case '9': // Speed 90;
      carSpeed = 255*90/100;
      break;
    case 'q': // Speed 100;
      carSpeed = 255*100/100;
      break;
  }
  distance = readPing();
}

/**
 * Motor forward
 */
void forward(void){
  movingForward = true;
  if(distance < 20) {
    Serial.println("Aqui 1");
    stopAll();
  } else {
    digitalWrite(motorInLeft1,HIGH);
    digitalWrite(motorInLeft2,LOW);
    digitalWrite(motorInRight3,LOW);
    digitalWrite(motorInRight4,HIGH);
    analogWrite(motorEnLeft,carSpeed);
    analogWrite(motorEnRight,carSpeed);
  }
//  Serial.print("Forward: "); Serial.println(carSpeed, HEX);
}

void forwardRight(){
  movingForward = true;
  if(distance < 20) {
    stopAll();
  } else {
    digitalWrite(motorInLeft1, HIGH);
    digitalWrite(motorInLeft2, LOW);
    digitalWrite(motorInRight3, LOW);
    digitalWrite(motorInRight4, HIGH);
    analogWrite(motorEnLeft, carSpeed);
    analogWrite(motorEnRight, LOW);
  }
//  Serial.print("Forward right: ");   Serial.print(speedLeft, HEX);  Serial.println(speedRight, HEX);
}

void forwardLeft(void){
    movingForward = true;
  if(distance < 20) {
    stopAll();
  } else {
    digitalWrite(motorInLeft1, HIGH);
    digitalWrite(motorInLeft2, LOW);
    digitalWrite(motorInRight3, LOW);
    digitalWrite(motorInRight4, HIGH);
    analogWrite(motorEnLeft,LOW);
    analogWrite(motorEnRight, carSpeed);
  }
//  Serial.println("Forward left");
}
 
void back(){
  movingForward = false;
  digitalWrite(motorInLeft1,LOW);
  digitalWrite(motorInLeft2,HIGH);
  digitalWrite(motorInRight3,HIGH);
  digitalWrite(motorInRight4,LOW);
  analogWrite(motorEnLeft,carSpeed);
  analogWrite(motorEnRight,carSpeed);
//  Serial.print("Back: "); Serial.println(carSpeed, HEX);
//  Serial.println(distance, DEC);
}
 
void backRight(void){
  movingForward = false;
  digitalWrite(motorInLeft1, LOW);
  digitalWrite(motorInLeft2, HIGH);
  digitalWrite(motorInRight3, HIGH);
  digitalWrite(motorInRight4, LOW);
  analogWrite(motorEnLeft, carSpeed);
  analogWrite(motorEnRight, LOW);
//  Serial.println("Back right");
}
 
void backLeft(void){
  movingForward = false;
  digitalWrite(motorInLeft1,LOW);
  digitalWrite(motorInLeft2,HIGH);
  digitalWrite(motorInRight3,HIGH);
  digitalWrite(motorInRight4,LOW);
  analogWrite(motorEnLeft, LOW);
  analogWrite(motorEnRight,carSpeed);
//  Serial.println("Back left");
}
 
void right(){
  digitalWrite(motorInLeft1, HIGH);
  digitalWrite(motorInLeft2, LOW);
  digitalWrite(motorInRight3, HIGH);
  digitalWrite(motorInRight4, LOW);
  analogWrite(motorEnLeft, carSpeed);
  analogWrite(motorEnRight, carSpeed);
//  Serial.println("Right");
}
 
void left(){
  digitalWrite(motorInLeft1, LOW);
  digitalWrite(motorInLeft2, HIGH);
  digitalWrite(motorInRight3, LOW);
  digitalWrite(motorInRight4, HIGH); 
  analogWrite(motorEnLeft, carSpeed);
  analogWrite(motorEnRight, carSpeed);
//  Serial.println("Left");
}
 
void stopAll(){
  movingForward = false;
  analogWrite(motorEnRight,LOW);
  analogWrite(motorEnLeft,LOW);
  digitalWrite(ledPinRearRight, HIGH);
  digitalWrite(ledPinRearLeft, HIGH);
//  Serial.println("Stop!");
}

/**
 * Ultrasonic sensor reader
 */
int readPing(){
  static unsigned long currentMillis;
  static unsigned long deltaT;

  currentMillis = millis();
  deltaT = currentMillis - previousMillis;
  previousMillis = currentMillis;
  if(deltaT < 70) delay(70 - deltaT);
//  delay(70);
  int cm = sonar.ping_cm();
  if (cm==0){
    cm=250;
  }
  return cm;
}
