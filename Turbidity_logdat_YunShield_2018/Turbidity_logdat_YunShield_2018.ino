/*
- clean up version dated in July 26, 2016
- setup is similar to the version above
- Binh Nguyen, updated this on June 14, 2018
*/

#include <FileIO.h>     //FileIO class allow user to operate Linux file system
#include <Console.h>   //Console class provide the interactive between IDE and Yun Shield

#include <Wire.h>
#include <SPI.h>
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
bool _verbose = true;           //option to print alot or some message to minitor screen

//SET UP LOOP, RUN ONE
void setup() {
  // Initialize the Console
  Bridge.begin();       //to bridge between Arduino and YunShiled (running on Linux)
  Console.begin();      //similar to Serial.begin()
  FileSystem.begin();
  Console.println("Datalogger for TurbidoStat\n");

  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);   //Adafruit 0x3D, others: 0x3C
  oled.setTextSize(2);
  oled.setTextColor(WHITE);
  oled.setCursor(0,0);
  oled.println("SCEB");
  oled.println("Turbidity");
  oled.display();
  oled.clearDisplay();
  delay(1000);
  oled.setCursor(0,0);
  
  
  File dataFile = FileSystem.open("/mnt/sda1/data/logturbid.csv", FILE_APPEND);
 if (dataFile) {
  dataFile.println("--, Reset,--, Reset");
  dataFile.println("Time, Turbidity (Read), Turbidity (Set), # Pump ON");
  Console.println("SD Card READY ");
  oled.println("SD OK");
  dataFile.close();
  } else {
    Console.println("Check SD Card!");
    oled.println("Check Card");
  }
  
  pinMode(turbidAPin, INPUT);
  pinMode(potentiometer, INPUT);

  pinMode(samplingPump, OUTPUT);
  pinMode(mainPump, OUTPUT);
  digitalWrite(samplingPump, HIGH);      //Signal High, Relay open, disconnect from the power
  digitalWrite(mainPump, HIGH);

  oled.display();
  delay(1000);
  oled.clearDisplay();          
 }
 
 
void loop (){

  int read25, read35, read45, read55;
  // make a string that start with a timestamp for assembling the data to log:
  String dataString;
  dataString += getTimeStamp();
  dataString += " , ";
  setValue = map(analogRead(potentiometer), 0, 1023, 200, 700);
      
  int unsigned intercept = count % interval;
    
  if (intercept < 60) {
    digitalWrite(samplingPump, LOW);  //set relay for the sampling pump to turn on the sampling pump on
    aRead = analogRead(turbidAPin);
    Console.print("Turbidity reading :");
    Console.print(aRead);
    Console.print("|><| Set:");
    Console.println(setValue);

    switch (intercept){
      case 25:
        read25 = aRead;
        Console.println (read25);
      break;
      case 35:
        read35 = aRead;
        Console.println (read35);
      break;
      case 45:
        read45 = aRead;
        Console.println (read45);
      break;
      case 55:
        read55 = aRead;
        Console.println (read55);
      break;                     
     }
                               
    if ((aRead < setValue) && (intercept > 20)){
      delay(deLay);                     //2 seconds when the sampling pump ON, otherwise, 10 seconds
      int aRead1 = analogRead(turbidAPin);
      delay(deLay);
      int aRead2 = analogRead(turbidAPin);
      delay(deLay);
      int aRead3 = analogRead(turbidAPin);
      Console.print("Re-reads: ");
      Console.print(aRead1);
      Console.print(",");
      Console.print(aRead2);
      Console.print(",");
      Console.println(aRead3);
      
      //VERY IMPORTANT OPERATION           
      if ((aRead1 < setValue) && (aRead2 < setValue) && (aRead3 < setValue)) {
        digitalWrite(mainPump, LOW);   //turn on the main pump to dilute the culture
        count --;
        delay(90000);  //delay 90 seconds; adjust to fit the flowrate, and biomass density swing
        _pumpON ++;
      }  else digitalWrite(mainPump, HIGH);   // turn off the main pump
     } 
    } else {
      digitalWrite (samplingPump, HIGH);  //turn off sampling pump
      digitalWrite (mainPump, HIGH);  // turn off the main pump anyway,
    }
     
  if (intercept == 60){
    averageRead = round((read25+read35+read45+read55)/4);
  
    dataString += String(averageRead);
    dataString += ",";
    dataString += String(setValue);
    dataString += ",";
    dataString += String(_pumpON);
    _pumpON = 0;       //reset counter for the main pump

    File dataFile = FileSystem.open("/mnt/sda1/data/logturbid.csv", FILE_APPEND);
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      Console.print("Recording: ");
      Console.println(dataString);
     } else {
      Console.println("error opening logturbid.csv");
     } 
  }

   if (digitalRead (samplingPump) == LOW){
    deLay = 2000;  /// two second
   } else deLay = 10000;  /// 10 seconds
       

    
  if (_verbose){
    if (intercept <=60){      //to know if to wati for sampling, get coffee, or go home
      Console.print("Sampling: ");
      Console.print(intercept);
      Console.print(" Sampling round: 60, ");       
    } else {
      Console.print("Next sampling: ");
      int countingDown = interval - intercept;
      Console.print(countingDown);
    }
    
   Console.print(" Total Count:");
   Console.print(count);
   Console.print(" Sampling Interval:");
   Console.print(interval);
   Console.print(" Delay: ");
   Console.print(deLay);
   Console.print(" Sampling Pump: ");
   if (digitalRead(samplingPump) == 1) {
    Console.print("OFF");
   } else Console.print("ON");
   Console.print(" Main Pump: ");
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
 
//GET TIMESTAMP
String getTimeStamp() {
  String result;
  Process time;
  time.begin("date");
  time.addParameter("+%D %T");  // parameters: D for the complete date mm/dd/yy
                                //             T for the time hh:mm:ss    
  time.run();  // run the command
  while(time.available()>0) {
    char c = time.read();
    if(c != '\n')
    result += c;
  }
  return result;
}
