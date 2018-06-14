/*
 * This version use a OLED screen, a Arduino Nano,
 * data is logged by a serial printout to a host computer (Raspeberry, laptop)
 * By Binh Nguyen, last updated June 14, 2018
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 oled(OLED_RESET);

// PIN MAPPING
#define turbidAPin    A0    //for analog signal from turbidity sensor
#define potentiometer A2
#define samplingPump  8     //to control sampling pump
#define mainPump      9     // to control the extraction/feed pump

// OPERATION PARAMETERs

int const interval = 240;    // the numbers of count for each cycle of sampling, needed to be large than 60
int deLay;                  //delay time after each sampling points, in miliseconds
int unsigned count = 0;
int unsigned _pumpON = 0;

int unsigned aRead = 0;
int unsigned averageRead = 0;
int unsigned setValue = 0;         //for potetiometer

//SET UP LOOP, RUN ONE
void setup(){

  //Start Serial communicaiton
  Serial.begin(9600);

  pinMode(turbidAPin, INPUT);
  pinMode(potentiometer, INPUT);
  pinMode(samplingPump, OUTPUT);
  pinMode(mainPump, OUTPUT);
  digitalWrite(samplingPump, HIGH);      //Signal High, Relay open (not connect)
  digitalWrite(mainPump, HIGH);
  
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Adafruit 0X3D, others: 0x3C
      
  // Clear the buffer.
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  oled.setCursor(0,0);
  oled.println("SCEB");
  oled.println("Turbidity");
  oled.display();
  oled.clearDisplay();
  delay(1000);
  oled.setCursor(0,0);
  oled.println("Setup");
  oled.println("Done");
  oled.display();
  delay(1000);
  oled.clearDisplay();
  }

//THIS LOOP RUN FOREVER
void loop (){
  
  int read25, read35, read45, read55;

  //brackets the range of potentiometer to the turbudity value
  setValue = map(analogRead(potentiometer), 0, 1023, 200, 700);
  updateDisplay(setValue, averageRead);
          
  int unsigned intercept = count % interval;  //get the remaining of count
     
  if (intercept < 60){
    digitalWrite(samplingPump, LOW);  //turn on the sampling pump
    aRead = analogRead(turbidAPin);
    updateDisplay(setValue, aRead);
    
    //log several values for better accuracy
    
    switch (intercept){
      case 25:
        read25 = aRead;
        break;
      case 35:
        read35 = aRead;
        break;
       case 45:
        read45 = aRead;
        break;
      case 55:
        read55 = aRead;
        break;                     
    }
  // TURN ON THE MAIN PUMP
  // Turbidity is higher than the set Value, and the sample was flushed                   
    if ((aRead < setValue) && (intercept > 20)){
      delay(1000);
      int aRead1 = analogRead(turbidAPin);
      delay(1000);
      int aRead2 = analogRead(turbidAPin);
      delay(1000);
      int aRead3 = analogRead(turbidAPin);
      
      if ((aRead1 < setValue) && (aRead2 < setValue) && (aRead3 < setValue)){
        digitalWrite(mainPump, LOW);   //turn on the main pump to dilute the culture
        delay(90000);  //delay 90 seconds, 
                       // canbe adjusted depend on how smooth the biomass conentration and the flowrate of the main pump
        _pumpON ++;
       } else {
        digitalWrite(mainPump, HIGH);   // turn off the main pump
       } 
    }        
    else {
      digitalWrite (samplingPump, HIGH);  //turn off the sampling pump
      digitalWrite (mainPump, HIGH);  // turn off the main pump anyway
    }           
  }     
// CONCATENATE DATA THEN PRINTOUT VIA SERIAL (USB)
 
  if (intercept == 60){
   // make a string that will print out to Serial line, and log to a computer
    String dataString = "";
    averageRead = round((read25+read35+read45+read55)/4);

    dataString += String(averageRead);
    dataString += ",";
    dataString += String(setValue);
    dataString += ",";
    dataString += String(_pumpON);
    _pumpON = 0;       //reset counter

    Serial.println(dataString); //Python script will catch this line and save to the computer
   }

  //ADJUST TIME DELAY BETWEEN EACH LOOP BETWEEN SAMPLING AND IDLING
   if (digitalRead (samplingPump) == LOW){
    deLay = 2000;  // two second
   } else{
    deLay = 10000;  // 10 seconds
   }
   
  delay(deLay);
  count ++;
}

void updateDisplay(int setValue, int aRead){
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0,0);
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  oled.println("Set |Read");
  oled.print(setValue);
  oled.print(" |");
  oled.println(aRead);
  oled.display();
}

