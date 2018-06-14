/*
Control LED intensity by a timer
Log data via a lux sensor
By Binh Nguyen, May 13, 2016
*/
#include <Wire.h>                                                         // Wire libary for I2C comunication
                                                          //the id number to enter the log order
#include "RTClib.h"                                                         // Date and time functions using a DS1307 RTC connected via I2C and Wire lib
RTC_DS1307 rtc;

#include <SPI.h>                                                          // Needed for SD libarary
#include <SD.h>                                                           // SD card to store data

#include <Adafruit_GFX.h>                                                     //For OLED Display
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4
Adafruit_SSD1306 oled(OLED_RESET);


#include <Adafruit_Sensor.h>                                                    // For Luminosity sensor
#include "Adafruit_TSL2591.h"
Adafruit_TSL2591 tsl = Adafruit_TSL2591(2591);                              // pass in a number for the sensor identifier (for your use later)
long lux2;                                                                   // store luminosity 

const int chipSelect = 53;                                                 // need to figure out for Adafruit SD breakout//https://learn.adafruit.com/adafruit-micro-sd-breakout-board-card-tutorial/wiring
                                                                            // CLK (52|13); DO (50|12); DI (51|11); CS (53|10)
char logFileName[] = "Light.txt";                                       // modify logFileName to identify your experiment, for exampe PBR_01_02, datalog1
String dataString;
long id = 1;                                                            //the id number to enter the log order

/*
#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 9              //define the pin # for temperature probe
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress ProbeP = { 0x28, 0xC2, 0xE8, 0x37, 0x07, 0x00, 0x00, 0xBF };                                            //may be helpful for check temperature, safety function
*/ 

float hour24 [] =    {0.0, 1.0, 2.0, 3.0, 4.0, 4.95,           
                      5.20, 5.45, 5.70, 5.95, 
                      6.20, 6.45, 6.70, 6.95,
                      7.20, 7.45, 7.70, 7.95, 
                      8.20, 8.45, 8.70, 8.95, 
                      9.20, 9.45, 9.70, 9.95,
                      10.20, 10.45, 10.70, 10.95,
                      11.20, 11.45, 11.70, 11.95,
                      12.20, 12.45, 12.70, 12.95,
                      13.20, 13.45, 13.70, 13.95,
                      14.20, 14.45, 14.70, 14.95,
                      15.20, 15.45, 15.70, 15.95,
                      16.20, 16.45, 16.70, 16.95,
                      17.20, 17.45, 17.70, 17.95,
                      18.20, 18.45, 18.70, 18.95,
                      19.20, 19.45, 19.70, 19.95,
                      20.0, 21.0, 22.0, 23.0, 23.5, 24.0};
                                                             //hour summer 2014
float intensity [] = {0.00, 0.00, 0.00, 0.00, 0.00, 0.00,  //hour 0.0, 1.0, 2.0, 3.0, 4.0, 4.95,
                      0.00, 0.00, 0.00, 0.00,      //     5.20, 5.45, 5.70, 5.95,
                      0.00, 0.01, 0.02, 0.06,     //     6.20, 6.45, 6.70, 6.95,
                      0.09, 0.13, 0.17, 0.22,     //     7.20, 7.45, 7.70, 7.95, 
                      0.26, 0.34, 0.37, 0.46,     //     8.20, 8.45, 8.70, 8.95,
                      0.54, 0.67, 0.65, 0.69,      //    9.20, 9.45, 9.70, 9.95,
                      0.77, 0.81, 0.90, 0.78,      //    10.20, 10.45, 10.70, 10.95,
                      0.89, 0.76, 0.84, 0.90,     //     11.20, 11.45, 11.70, 11.95,
                      0.87, 0.91, 1.00, 0.95,     //     12.20, 12.45, 12.70, 12.95,
                      0.83, 0.79, 0.79, 0.84,     //     13.20, 13.45, 13.70, 13.95,
                      0.82, 0.60, 0.48, 0.61,     //     14.20, 14.45, 14.70, 14.95,
                      0.42, 0.42, 0.47, 0.42,     //     15.20, 15.45, 15.70, 15.95,
                      0.39, 0.41, 0.37, 0.27,     //     16.20, 16.45, 16.70, 16.95,
                      0.31, 0.33, 0.30, 0.32,     //     17.20, 17.45, 17.70, 17.95,
                      0.25, 0.21, 0.18, 0.12,     //     18.20, 18.45, 18.70, 18.95,
                      0.07, 0.04, 0.02, 0.01,     //     19.20, 19.45, 19.70, 19.95,
                      0.00, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};       //     20.0, 21.0, 22.0, 23.0
                      
                        




                      
float timechecker;
int pinLED = 9;
int pinLEDX = 10;

void displaySensorDetails(void)
          {
            sensor_t sensor;
            tsl.getSensor(&sensor);
            Serial.println("------------------------------------");
            Serial.print  ("Sensor:       "); Serial.println(sensor.name);
            Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
            Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
            Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" lux");
            Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" lux");
            Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" lux");  
            Serial.println("------------------------------------");
            Serial.println("");
            delay(500);
          }

void configureSensor(void)
      {
                                                              // You can change the gain on the fly, to adapt to brighter/dimmer light situations
            //tsl.setGain(TSL2591_GAIN_LOW);                  // 1x gain (bright light)
            tsl.setGain(TSL2591_GAIN_MED);                    // 25x gain
            // tsl.setGain(TSL2591_GAIN_HIGH);                // 428x gain
            
                                                              // Changing the integration time gives you a longer time over which to sense light
                                                              // longer timelines are slower, but are good in very low light situtations!
            tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);     // shortest integration time (bright light)
            // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
            // tsl.setTiming(TSL2591_INTEGRATIONTIME_300MS);
            // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
            // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
            // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)
          
            /* Display the gain and integration time for reference sake */  
            Serial.println("------------------------------------");
            Serial.print  ("Gain:         ");
            tsl2591Gain_t gain = tsl.getGain();
            switch(gain)
            {
              case TSL2591_GAIN_LOW:
                Serial.println("1x (Low)");
                break;
              case TSL2591_GAIN_MED:
                Serial.println("25x (Medium)");
                break;
              case TSL2591_GAIN_HIGH:
                Serial.println("428x (High)");
                break;
              case TSL2591_GAIN_MAX:
                Serial.println("9876x (Max)");
                break;
            }
            Serial.print  ("Timing:       ");
            Serial.print((tsl.getTiming() + 1) * 100, DEC); 
            Serial.println(" ms");
            Serial.println("------------------------------------");
            Serial.println("");
      }


void advancedRead(void)
      {
            // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
            // That way you can do whatever math and comparisons you want!
            uint32_t lum = tsl.getFullLuminosity();
            uint16_t ir, full;
            ir = lum >> 16;
            full = lum & 0xFFFF;

            long vis = full - ir;
            long lux = tsl.calculateLux(full, ir);
            lux2 = lux;
            dataString += String(ir)+",";
            dataString += String(vis)+",";
            dataString += String(lux) +",";
            
            Serial.print("[ "); Serial.print(millis()); Serial.print(" ms ] ");
            Serial.print("IR: "); Serial.print(ir);  Serial.print("  ");
            Serial.print("Full: "); Serial.print(full); Serial.print("  ");
            Serial.print("Visible: "); Serial.print(full - ir); Serial.print("  ");
            Serial.print("Lux: "); Serial.println(tsl.calculateLux(full, ir));
      }
void unifiedSensorAPIRead(void)
      {        
            sensors_event_t event;                                                                 // Get a new sensor event
            tsl.getEvent(&event);
            //Serial.print("[ "); Serial.print(event.timestamp); Serial.print(" ms ] ");           // Display the results (light is measured in lux)
            if ((event.light == 0) |
                (event.light > 4294966000.0) | 
                (event.light <-4294966000.0))
              {
                Serial.println("Invalid data (adjust gain or timing)");                            // If event.light = 0 lux the sensor is probably saturated
                                                                                                   // and no reliable data could be generated! 
                                                                                                   // if event.light is +/- 4294967040 there was a float over/underflow 
              }
            else
              { Serial.print(event.light); Serial.println(" lux");}
      }

                                                      
void setup() 
  {
  
     Serial.begin(9600);                                                                           // Start Serial commnication
     
     pinMode(pinLED, OUTPUT);
     pinMode(pinLEDX, OUTPUT);
                                
     Serial.print("RTC is...");   
      if (! rtc.begin())
          { 
           Serial.println("RTC:  Real-time clock...NOT FOUND");
           while (1);// (Serial.println("RTC:  Real-time clock...FOUND"));
          }
     Serial.println("RUNNING");

       
     Serial.print("Real-time Clock...");
       if (! rtc.isrunning())
            {rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
            }
     Serial.println("WORKING");
           
           
       Serial.print("SD card...");                                                             // see if the card is present and can be initialized:
      if (!SD.begin(chipSelect)) 
          {  Serial.println("Failed");                                                         // don't do anything more:
             return;
          }
      Serial.println("OK");

     Serial.print("Log File: ");
       Serial.print(logFileName);
       Serial.print("...");
       File logFile = SD.open(logFileName, FILE_WRITE);                                       // open the file. "datalog" and print the header
        if (logFile)
          {
            logFile.println(", , , , , , ");                                                  //indicate there were data in the previous run
            String header = "ID, Date Time, TimeCheck, % Ouput, IR, VIS, Lux";
            logFile.println(header);
            logFile.close();
            Serial.println("READY");
          }
      else { Serial.println("error opening datalog"); }                                        // if the file isn't open, pop up an error:

     if (tsl.begin())                                                                          // Display some basic information on this sensor 
        { Serial.println("Found a TSL2591 sensor");} 
      else 
        { Serial.println("No sensor found ... check your wiring?");
          while (1);
        }
    displaySensorDetails();                                                                     // Display some basic information on this sensor 
    configureSensor();           
      
       
         
         // Setup OLED display
            oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
            oled.clearDisplay();
            delay(1000);
            oled.setTextSize(1);
            oled.setTextColor(WHITE);
            oled.setCursor(0,0);
            oled.println("Welcome...");
            oled.println("Light Control");
            oled.println("SD Card READY");
            oled.println("CLOCK READY");
            oled.println("LogFile READY");
            oled.display();
            oled.clearDisplay();
            id = 1;
            delay(4000);
   }

void loop() 
  { 

   dataString = String(id);
   dataString += String(',');    
  
    DateTime now = rtc.now();
    int houR = (int)now.hour();
    float minutE = (float)now.minute();
    
    timechecker = houR + minutE/60;
    //Serial.print("Minutes/60, ");
    //Serial.println(minutE/60);
    float record;
    int locator;
    for (int i=0; i<100; i ++)
        {
              /*
              Serial.print("i: ");
              Serial.print(i);
              Serial.print(", hour24[i]: ");
              Serial.print(hour24[i]);
              Serial.print(", Intensity[i]: ");
              Serial.print(intensity[i]);
              Serial.print("Checker - Time: ");
              Serial.println(timechecker - hour24[i]);
              */
              
              if (( hour24[i]-timechecker >=0) )
                {
                     /*
                      Serial.print("Check Time:,");
                      Serial.print(timechecker);
                      Serial.print(",");
                      Serial.print("hour24[i], ");
                      Serial.print(hour24[i]);
                      Serial.print(",");

                      delay(1000);
                      locator = i;
                      */

                      record = intensity[i];
                      //Serial.print("TimeCheck : hour24: ");
                      //float delta = timechecker - hour24[i];
                     // Serial.print("i:");
                      //Serial.print(hour24[i]);
                     // Serial.print("< Timecheck - hour24[i+1]");
                      //Serial.println(timechecker - hour24[i+1]);
                      
                      break;
                      
                }
                                
        }
     Serial.print("Locator:");
     Serial.println(locator);
     int lightout = record*255;
     analogWrite(pinLED, lightout);
     int lightout2 = 255- lightout; 
     analogWrite(pinLEDX, lightout2);  
     Serial.print("Lightout, ");  
     Serial.println(lightout);  
     //delay(2000);
     //Serial.println("Hardcode");
     //analogWrite(pinLED, 150);
    
    String datE = String(now.year(), DEC);
    datE += "/";
    datE += String(now.month(), DEC);
    datE += "/";
    datE += String(now.day(), DEC);
    
    String timE = String(now.hour(), DEC);
    timE += ":";
    timE += String(now.minute(), DEC);
    timE += ":";
    timE += String(now.second(), DEC);
    datE += " " + timE;
       
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(0,0);
    dataString += datE + ",";
    dataString += String(timechecker) + ",";
    dataString += String(record) + ",";
    
    advancedRead();
    
    oled.println(datE);
    oled.print("Time,hrs: ");
    oled.println(timechecker);
    oled.print("Output,%: ");
    oled.println(record);
    oled.print("LI: ");
    oled.print(lux2);
    oled.println(" lux");
    oled.display();
   // Serial.print("Lux record: ");
    //Serial.println(lux2);

    
 // Serial.println(dataString);
    File dataFile = SD.open(logFileName, FILE_WRITE);                         // open the file. note that only one file can be open at a time, so you have to close this one before opening another.

        if (dataFile)                                                         // if the file is available, write to it:
          {
            dataFile.println(dataString);
            dataFile.close();
            Serial.print("Recording: ");
            Serial.println(dataString);                                        // print to the serial port too:
          }

         else { Serial.println("error opening datalog file"); }                // if the file isn't open, pop up an error: 
    
    oled.clearDisplay();
    //Serial.println(id);
    delay(300000);   //delay for 5 minutes

    id ++;
    dataString = "";

  }



/*
void displayTemperature(DeviceAddress deviceAddress)
    {
      float tempC = sensors.getTempC(deviceAddress);
      if (tempC == -127.00)
        { lcd.print("Temperature Error");}
      else { //lcd.print("C=");
             dataString = String(tempC);
            // lcd.print(tempC);
            // lcd2.print(tempC);
             //lcd.print("F=");
             //lcd.print(DallasTemperature::toFahrenheit(tempC));
            }
    }  
*/





