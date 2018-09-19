

//**THIS CODE WILL WORK ON ANY ARDUINO** 
//This code will output data to the Arduino serial monitor. Type commands into the Arduino serial monitor to control the EZO pH Circuit in I2C mode.
// Modifed from the I2C code by Atlas-Scientific by Binh Nguyen, April 04, 2016



#include <Wire.h>                   //enable I2C.
#define pH_address 99               //default I2C ID number for EZO pH Circuit.
#define DO_address 97               //default I2C ID number for EZO DO Circuit.

#include "RTClib.h"                                                         // Date and time functions using a DS1307 RTC connected via I2C and Wire lib
RTC_DS1307 rtc;

#include <SPI.h>                                                          // Needed for SD libarary
#include <SD.h>                                                           // SD card to store data

const int chipSelect = 53;                                                 // need to figure out for Adafruit SD breakout//https://learn.adafruit.com/adafruit-micro-sd-breakout-board-card-tutorial/wiring
char logFileName[] = "dataLT.txt";                                       // modify logFileName to identify your experiment, for exampe PBR_01_02, datalog1
long id = 1;                                                            //the id number to enter the log order


#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 9              //define the pin # for temperature probe
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress ProbeP = { 0x28, 0xC2, 0xE8, 0x37, 0x07, 0x00, 0x00, 0xBF };

String dataString;              // the main variant to store all data
String dataString2;             // a temporary variant to store Temperature/pH/DO for print out

char computerdata[20];           //we make a 20 byte character array to hold incoming data from a pc/mac/other.   
byte received_from_computer=0;   //we need to know how many characters have been received.    
byte serial_event=0;             //a flag to signal when data has been received from the pc/mac/other. 
byte code=0;                     //used to hold the I2C response code. 
char pH_data[20];                //we make a 20 byte character array to hold incoming data from the pH circuit. 
byte in_char=0;                  //used as a 1 byte buffer to store in bound bytes from the pH Circuit.   
byte i=0;                        //counter used for ph_data array. 
int time_=1800;                   //used to change the delay needed depending on the command sent to the EZO Class pH Circuit. 
float pH_float;                  //float var used to hold the float value of the pH. 
char DO_data[20];

void setup(){
  Serial.begin(9600);           //enable serial port.  
  Wire.begin(99);                 //enable I2C port.
  Wire.begin(97);

  lcd.init();
  lcd.begin(20,4);

  lcd.backlight();
  lcd.home();
  lcd.print("Hello PBR!");
  lcd.setCursor(0,1);
  lcd.print("Initializing...");

  Serial.print("RTC is...");   
  if (! rtc.begin()){ 
       Serial.println("RTC:  Real-time clock...NOT FOUND");
       while (1);// (Serial.println("RTC:  Real-time clock...FOUND"));
      }
   Serial.println("RUNNING");

   Serial.print("Real-time Clock...");
   if (! rtc.isrunning()){
       rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
       }
   Serial.println("WORKING");
   lcd.setCursor(0,0);
   lcd.println("RTC: OK");
    
  Serial.print("SD card...");                                 // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) 
      {  Serial.println("Failed");                          // don't do anything more:
         return;
      }
  Serial.println("OK");
   lcd.setCursor(0,1);
   lcd.println("SD card: OK");  

   Serial.print("Log File: ");
   Serial.print(logFileName);
   Serial.print("...");
   File logFile = SD.open(logFileName, FILE_WRITE);   // open the file. "datalog" and print the header
    if (logFile){
        logFile.println(", , , , , ,");                       //indicate there were data in the previous run
        String header = "ID, Date, Time, Turbidity, Temp(C), pH, DO";
        logFile.println(header);
        logFile.close();
        Serial.println("READY");
        //Serial.println(dataString);   // print to the serial port too:
      }
  else { Serial.println("error opening datalog"); }  // if the file isn't open, pop up an error:
   lcd.setCursor(0,2);
   lcd.print("Log file:");
   lcd.println(logFileName);      
     
  delay(1000);
  sensors.begin();
  sensors.setResolution(ProbeP, 10);
  lcd.clear();
  id = 0;
  }
        
void loop(){                    //the main loop.
  dataString = String(id);
  dataString = String(',');
  DateTime now = rtc.now();
  dataString = String(now.year(), DEC);
  dataString += String('/');
  dataString +=  String(now.month(), DEC);
  dataString += String('/');
  dataString += String(now.day(), DEC);
  dataString += String(' ');
  dataString += String(now.hour(), DEC);
  dataString += String(':');
  dataString += String(now.minute(), DEC);
  dataString += String(':');
  dataString += String(now.second(), DEC);
  lcd.home();
  lcd.print(dataString);
  sensors.requestTemperatures();
  displayTemperature(ProbeP);
            
  Wire.beginTransmission(pH_address); //call the circuit by its ID number.  
  Wire.write('r');                 //hard code r to read
  Wire.endTransmission();          //end the I2C data transmission.    
  delay(time_);                    //wait the correct amount of time for the circuit to complete its instruction. 
  Wire.requestFrom(pH_address,20,1); //call the circuit and request 20 bytes (this may be more than we ne

  while(Wire.available()){
      in_char = Wire.read();           //receive a byte.
     //Serial.println(in_char);
     if ((in_char > 31) && (in_char <127)){ //check if the char is usable (printable)
       pH_data[i]= in_char;             //load this byte into our array.
       i+=1; 
        }                           
     if(in_char==0){                 //if we see that we have been sent a null command.     
        i=0;                        //reset the counter i to 0.
        Wire.endTransmission();     //end the I2C data transmission.
        break;                      //exit the while loop.
          }
    }
    
   serial_event=0;                                 //reset the serial event flag.
   dataString2 += ",";
   dataString2 += String(pH_data);
   Wire.beginTransmission(DO_address);             //call the circuit by its ID number.  
   Wire.write('r');
   Wire.endTransmission();                         //end the I2C data transmission. 
   delay(time_);                                   //wait the correct amount of time for the circuit to complete its instruction. 
   Wire.requestFrom(DO_address,20,1);              //call the circuit and request 20 bytes (this may be more then we need).
   //code=Wire.read();                             //the first byte is the response code, we read this separately.  

   while(Wire.available()){                        //are there bytes to receive. 
       in_char = Wire.read();                      //receive a byte.
       if ((in_char > 31) && (in_char <127)){        //check if the char is usable (printable)
          DO_data[i]= in_char;                         //load this byte into our array.
          i+=1;                                              //incur the counter for the array element. 
          }
       if(in_char==0){                                        //if we see that we have been sent a null command. 
        i=0;                                 //reset the counter i to 0.
        Wire.endTransmission();              //end the I2C data transmission.
        break;                               //exit the while loop.
         } 
        }
    serial_event=0;                         //reset the serial event flag.
    pH_float = atof (pH_data); 
    dataString2 += ",";
    dataString2 += String(DO_data);
    lcd.setCursor(0,1);
    lcd.print("Temperature/ pH/ DO");
    lcd.setCursor(0,2);
    lcd.print(dataString2);

    dataString += ',';
    dataString += dataString2;
    File dataFile = SD.open(logFileName, FILE_WRITE);                 // open the file. note that only one file can be open at a time, so you have to close this one before opening another.
    if (dataFile)                                                    // if the file is available, write to it:
      {
        dataFile.println(dataString);
        dataFile.close();
        Serial.println(dataString);                                  // print to the serial port too:
      }

     else { Serial.println("error opening datalog file"); }          // if the file isn't open, pop up an error: 
   
    lcd.setCursor(0,3);
    lcd.print("Running(x5m):");
    lcd.setCursor(15,3);
    lcd.print(id);
    id ++;                                                              // increase one ID next iteration
    dataString = "";
    delay(300000);  //delay 5 mintues = 5*60*1000 ms
    lcd.clear();
  }

  void displayTemperature(DeviceAddress deviceAddress){
      float tempC = sensors.getTempC(deviceAddress);
      if (tempC == -127.00)
        { lcd.print("Temperature Error");}
      else {
             dataString2 = String(tempC);
            }
    }  
