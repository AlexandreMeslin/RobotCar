/**
 * From: https://create.arduino.cc/projecthub/sraykov08/joystick-shield-6713f9
 */

#define BUTTON_UP 2
#define BUTTON_RIGHT 3
#define BUTTON_DOWN 4
#define BUTTON_LEFT 5
#define BUTTON_E 6
#define BUTTON_F 7
#define BUTTON_K 8
#define PIN_ANALOG_X 0
#define PIN_ANALOG_Y 1

#define DELAY 500

void setup() {
  Serial.begin(9600);
  // to enable pull up resistors first write pin mode
  // and then make that pin HIGH
  pinMode(BUTTON_UP, INPUT);
  digitalWrite(BUTTON_UP, HIGH);
  
  pinMode(BUTTON_RIGHT, INPUT);
  digitalWrite(BUTTON_RIGHT, HIGH);
  
  pinMode(BUTTON_DOWN, INPUT);
  digitalWrite(BUTTON_DOWN, HIGH);
  
  pinMode(BUTTON_LEFT, INPUT);
  digitalWrite(BUTTON_LEFT, HIGH);
  
  pinMode(BUTTON_E, INPUT);
  digitalWrite(BUTTON_E, HIGH);
  
  pinMode(BUTTON_F, INPUT);
  digitalWrite(BUTTON_F, HIGH);
  
  pinMode(BUTTON_K, INPUT);
  digitalWrite(BUTTON_K, HIGH);

  Serial.println("Starting...");
}

void loop() {
  if(digitalRead(BUTTON_UP) == LOW) {
    Serial.println("Button pro is pressed");
    delay(DELAY);
  } else if(digitalRead(BUTTON_RIGHT) == LOW) {
    Serial.println("Button 0 is pressed");
    delay(DELAY);
  } else if(digitalRead(BUTTON_DOWN) == LOW) {
    Serial.println("Button X is pressed");
    delay(DELAY);
  } else if(digitalRead(BUTTON_LEFT) == LOW) {
    Serial.println("Button");
    Serial.println((char)254);
    Serial.println(" is pressed");
    delay(DELAY);
  } else if(digitalRead(BUTTON_E) == LOW) {
    Serial.println("Button E is pressed");
    delay(DELAY);
 } else if(digitalRead(BUTTON_F) == LOW) {
    Serial.println("Button F is pressed");
    Serial.print("x: ");
    Serial.println(analogRead(PIN_ANALOG_X));
    // Print y axis values
    Serial.print("y: ");
    Serial.println(analogRead(PIN_ANALOG_Y));

    // Some delay to clearly observe your values on serial monitor.
    delay(500);
    delay(DELAY);
  }
  if(digitalRead(BUTTON_K) == LOW) {
    Serial.println("Button k  is pressed");
    delay(DELAY);
  }
}
