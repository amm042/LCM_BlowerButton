
#include <limits.h>

const int RED = 4;
const int GREEN = 5; 
// RED, GREEN active LOW

const int RELAY =8;
const int BTN = 6;

const unsigned long DEBOUNCE_MS = 150;
const unsigned long RUN_MS = 15000;
const unsigned long OFF_MS = 5000; 

unsigned long msago (unsigned long ago){
  unsigned long now = millis();
  if (now < ago){
    // rollover
    return now + ULONG_MAX - ago + 1;
  }else {
    return now - ago;
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode (RELAY, OUTPUT);

  pinMode (BTN, INPUT);
  digitalWrite(BTN, HIGH); // pull up.
  // OFF
  digitalWrite(RED, HIGH);
  digitalWrite(GREEN, HIGH);
  digitalWrite(RELAY, LOW);

  Serial.begin(9600);
}

static int state = 0;
static unsigned long pressTime = 0;
static unsigned long onTime = 0;
static unsigned long cooldownTime = 0;
void loop() {
  int amt= 0;
  if (state == 0){          // OFF - YELLOW
    digitalWrite(RELAY, LOW); 
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, LOW);
    if (digitalRead(BTN) == LOW){
      state = 1;
      pressTime = millis();
      Serial.println("Press");
    }
  } else if (state == 1){ // debounce RED
    digitalWrite(RELAY, LOW);
    if (msago(pressTime) > DEBOUNCE_MS){
      state = 2;
    } else {
      if (digitalRead(BTN) == HIGH){
        state = 0; // failed to debounce
      } else{
        amt = map(msago(pressTime), 0, DEBOUNCE_MS, 0, 255);
        analogWrite(GREEN, 255-amt);
        analogWrite(RED, 255);           
        Serial.print("debounce - ");
        Serial.println(amt);
      }
    }
  } else if (state == 2){ // release button after debounce - GREEN
    digitalWrite(RELAY, LOW); 
    digitalWrite(RED, HIGH);
    digitalWrite(GREEN, LOW);
    if (digitalRead(BTN) == HIGH){
      state = 3;
      onTime = millis();
    }
  } else if (state == 3){ // activate - GREEN
    digitalWrite(RELAY, HIGH);
    if (msago(onTime) > RUN_MS){
      state = 4;
      cooldownTime = millis();
    }else{
      amt = map(msago(onTime), 0, RUN_MS, 0, 255);
      analogWrite(GREEN, amt);
      analogWrite(RED, 255);   
      Serial.print("active - ");
      Serial.println(amt);
    }
  } else if (state == 4){ // cool down. - RED
    digitalWrite(RELAY, LOW);
    if (msago(cooldownTime) > OFF_MS){
      state = 5;
    }else{
      amt = map(msago(cooldownTime), 0, OFF_MS, 0, 255);
      //digitalWrite(RED, LOW);
      //digitalWrite(GREEN, HIGH);
      analogWrite(RED, 0);
      //analogWrite(GREEN, 255);
      analogWrite(GREEN, 255);
      Serial.print("cooldown - ");
      Serial.println(amt);
    }
  } else if (state == 5){ // ensure button is released.
    digitalWrite(RELAY, LOW);
    digitalWrite(RED, LOW);
    digitalWrite(GREEN, HIGH);  
    if (digitalRead(BTN) == HIGH){
      state = 0;
      Serial.print("idle - ");
    }
  } else {
    state = 0;
  }
  delay(10);
}
