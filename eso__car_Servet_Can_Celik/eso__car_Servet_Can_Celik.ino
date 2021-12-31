#include "Display.h"
#include "DHT11.h"  //This provide us temperature values as centigrade

const int LEDRED = 4;         //Those are components which we use durign the process
const int LEDGREEN = 5;
const int LEDBLUE = 6;
const int LEDYELLOW = 7;
const int KEYRIGHT = 8;
const int KEYLEFT = 9;
const int LDR = A2;
const int NTC = A1;
const int KNOB = A0;

int oldkeystateR = HIGH;    //I declared 2 state(R for right button, L for left button) I use them in Bouncing method.
int oldkeystateL = HIGH;

int thresholdLow;       //After I start my arduino, I used this 3 values to turn on or off my headlights.
int thresholdHigh;
int rangeHalf = 50;

bool lambBlue = true;     //those are my bool's which would be used at an element of my if stataments.
bool lambGreen = true;
bool blueState = false;
bool greenState = false;
bool currentHeadLight = false;
bool alarmMode = true;
bool headLight=false;

unsigned long lasttimeB;
unsigned long lasttimeG;
unsigned long tempTime;

enum Modes { CALM, PANIC};   //I have 2 modes for this system and i switch between them with this enum.
Modes mode;

String command;       //The command which I send to C#
int steer;            //My steering wheel


void setup() {
  pinMode(LEDRED, OUTPUT);      //I activated all the components.
  pinMode(LEDGREEN, OUTPUT);
  pinMode(LEDBLUE, OUTPUT);
  pinMode(LEDYELLOW, OUTPUT);
  pinMode(KEYRIGHT, INPUT_PULLUP);
  pinMode(KEYLEFT, INPUT_PULLUP);
  pinMode(LDR, INPUT);
  pinMode(NTC, INPUT);
  pinMode(KNOB, INPUT);

  Serial.begin(115200);     //I used 115200 instead of 9600 for both here and visualStudio
  int val = analogRead(LDR);  //first light value
  thresholdLow = val - rangeHalf;
  thresholdHigh = val + rangeHalf;
  lasttimeB = millis();
  lasttimeG = millis();
  tempTime = millis();
  mode = CALM;
}

void loop() {
  Display.clear();

  int valCurrent = analogRead(LDR);   //This part operate headlights.
  if (valCurrent > thresholdHigh)
  {
    digitalWrite(LEDYELLOW, LOW);
    headLight = false;
  }
  else if (valCurrent < thresholdLow)
  {
    digitalWrite(LEDYELLOW, HIGH);
    headLight = true;
  }
  if (headLight != currentHeadLight)
  {
    currentHeadLight = headLight;
    Serial.println("headlight");
  }
  steer = analogRead(KNOB);     //This part for steering wheel.
  if (steer > 560 & blueState==HIGH)
  {
    blueState = LOW;
    Serial.println("left");
  }
  if (steer < 460 & greenState==HIGH)
  {
    greenState = LOW;
    Serial.println("right");
  }

  if (millis() > tempTime + 5000)   //I send one a every 5 seconds the current temperature.
  {
    float temp = DHT11.getTemperature();
    String tempstring = "Current temp is: ";
    tempstring.concat(temp);
    Serial.println(tempstring);
    tempTime = millis();
  }

  if (Serial.available() > 0)  // getting command from serial monitor and assigning it to a string variable
  {
    command = Serial.readStringUntil('\n');
    if (command == "Panic")
    {
      mode = PANIC;
    }
    else if (command == "Calm")
    {
      mode = CALM;
    }
  }


  if (mode == PANIC)      //while this mode is active our light will be blinking.
  {
    if (millis() > lasttimeB + 1000)
    {
      alarmMode = !alarmMode;
      lasttimeB = millis();
    }
    if (alarmMode == true)
    {
      digitalWrite(LEDRED, HIGH);
      digitalWrite(LEDBLUE, HIGH);
      digitalWrite(LEDGREEN, HIGH);
    }
    else if (alarmMode == false)
    {
      digitalWrite(LEDRED, LOW);
      digitalWrite(LEDBLUE, LOW);
      digitalWrite(LEDGREEN, LOW);
    }
    if (guardianL() == true)
    {
      blueState = !blueState;
      if(greenState)
      {
        greenState = false;
        Serial.println("right");
      }      
      Serial.println("left");
    }
    if (guardianR() == true)
    {
      greenState = !greenState;
      if(blueState)
      {
        blueState = false;
        Serial.println("left");
      }
      Serial.println("right");
    }

  }


  if (mode == CALM)     //this is our normal mode, all the system should work properly at this stage.
  {
    digitalWrite(LEDRED, LOW);
    if (guardianL() == true)
    {
      blueState = !blueState;
      Serial.println("left");
      if(greenState)
      {
        greenState = false;
        Serial.println("right");
      }
    }

    if (blueState)
    {
      if (millis() > lasttimeB + 500)
      {
        digitalWrite(LEDBLUE, lambBlue);
        lambBlue = !lambBlue;
        lasttimeB = millis();
      }
    }
    else
    {
      digitalWrite(LEDBLUE, LOW);
    }


    if (guardianR() == true)
    {
      greenState = !greenState;
      Serial.println("right");
      if(blueState)
      {
        blueState = false;
        Serial.println("left");
      }
    }

    if (greenState)
    {
      if (millis() > lasttimeG + 500)
      {
        digitalWrite(LEDGREEN, lambGreen);
        lambGreen = !lambGreen;
        lasttimeG = millis();
      }
    }
    else
    {
      digitalWrite(LEDGREEN, LOW);
    }
  }
}

bool guardianR()      //I created this "guardian" methods to prevent bouncing,they keep my buttons bounce free.
{
  bool keydownR = false;
  int keyR = digitalRead(KEYRIGHT);
  if (keyR != oldkeystateR) {
    delay(100);
    keyR = digitalRead(KEYRIGHT);
    if (keyR == LOW)
    {
      keydownR = true;
    }
  }

  oldkeystateR = keyR;
  return keydownR;
}

bool guardianL()
{
  bool keydownL = false;
  int keyL = digitalRead(KEYLEFT);
  if (keyL != oldkeystateL) {
    delay(100);
    keyL = digitalRead(KEYLEFT);
    if (keyL == LOW)
    {
      keydownL = true;
    }
  }

  oldkeystateL = keyL;
  return keydownL;
}
