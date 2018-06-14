/*
http://wiki.dragino.com/index.php?title=Yun_Shield
Collect turbidity data and log them into SD card on Yun Shield
By Binh Nguyen, April 11, 2016
Updated on July 26, 2016: improving the display and counting

*/

#include <FileIO.h>     //FileIO class allow user to operate Linux file system
#include <Console.h>   //Console class provide the interactive between IDE and Yun Shield

#include <Wire.h>
#include <SPI.h>
#include <Math.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 oled(OLED_RESET);

#define SLAVE_ADDRESS 11    //assign an I2C address for slave unit (turbidity controller) 0x60 is for OLED

//Relay Pins
int samplingPump = 8;
int mainPump = 9;

//Turbidity sensor Pins

int turbidAPin = A0;   //read analog data from A0 Pin on Arduino
int turbidDPin = 5;    //read digial data, possible analog data from pin #12
int aRead;
int dRead;


//Counting to set up interval of reading
int interval = 240; // the numbers of count for each cycle of sampling, needed to be large than 60
int deLay; //delay time after each sampling points, in miliseconds
int unsigned count = 0;
//int samplingcount = 0;
int unsigned _pumpON = 0;

//Set value to turn on the dilution pump
int setValue;
int setPin = A2;
int read25, read35, read45, read55;
int averageRead;
 
void setup() 
    {
  

      // Initialize the Console
      Bridge.begin();
      Console.begin();
      FileSystem.begin();
     
      //while(!Console);  // wait for Serial port to connect.
      Console.println("Datalogger for TurbidoStat\n");

 

     File dataFile = FileSystem.open("/mnt/sda1/data/logturbid.csv", FILE_APPEND);
 
    // if the file is available, write to it:
        if (dataFile) 
            {
                dataFile.println("--, Reset,--, Reset");
                dataFile.println("Time, Turbidity (Read), Turbidity (Set), # Pump ON");
                 Console.println("SD Card READY ");
                dataFile.close();
    
              } 

        pinMode(turbidAPin, INPUT);
        pinMode(turbidDPin, INPUT);
        pinMode(setPin, INPUT);

        pinMode(samplingPump, OUTPUT);
        pinMode(mainPump, OUTPUT);
        digitalWrite(samplingPump, HIGH);      //Signal High, Relay open
        digitalWrite(mainPump, HIGH);
     
        // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
        oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64), modified by Binh, the orginal IC2 was 0x3D (61)
        // init done
        
            
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
        oled.println("SD OK");
        oled.println("Logging");
        oled.display();
        delay(1000);
        oled.clearDisplay();
        //oled.noDisplay();
        //delay(2000);
        

              
   }
 
 
void loop () {
      // make a string that start with a timestamp for assembling the data to log:
      String dataString;
      dataString += getTimeStamp();
      dataString += " , ";

      oled.setTextSize(1);
      oled.setTextColor(WHITE);
      oled.setCursor(0,0);
      //oled.println("Working...");
      //oled.display();
      int setValueRaw = analogRead(setPin);           //uncomment to activate the external reading
      //int setValueRaw = 0;
      setValue = map(setValueRaw, 0, 1023, 200, 700);
          
      int unsigned intercept = fmod (count, interval);

      //int aSet = setValue - tolerant;
      
        
      if (intercept < 60)
            {
                digitalWrite(samplingPump, LOW);  //set relay for the sampling pump to turn on the sampling pump on
                //digitalWrite(mainPump, LOW);
                aRead = analogRead(turbidAPin);
                dRead = analogRead(turbidDPin);
                Console.print("Reading Analog from D:");
                Console.print(dRead);
                Console.print("||| Analog (main):");
                Console.print(aRead);
                //Serial.print(" A/D:");
                //float noise = float(aRead)/float(dRead);
                //Serial.print(noise, 2);
                Console.print("|><| Set:");
                Console.println(setValue);

                switch (intercept)
                    {
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
                                           
                if ((aRead < setValue) && (intercept > 20))
                    {
                      delay(1000);
                      int aRead1 = analogRead(turbidAPin);
                      delay(1000);
                      int aRead2 = analogRead(turbidAPin);
                      delay(1000);
                      int aRead3 = analogRead(turbidAPin);
                      Console.print("Re-reads: ");
                      Console.print(aRead1);
                      Console.print(",");
                      Console.print(aRead2);
                      Console.print(",");
                      Console.println(aRead3);
                      
                      
                      if ((aRead1 < setValue) && (aRead2 < setValue) && (aRead3 < setValue))
                          {
                              digitalWrite(mainPump, LOW);   //turn on the main pump to dilute the culture
                              count --;
                              delay(90000);  //delay 90 seconds, the main pump will extract biomass in 4 minutes that creates a windows of biomass swing
                              _pumpON ++;
                          }
                      else {digitalWrite(mainPump, HIGH);}   
                    }
                 else
                     { digitalWrite(mainPump, HIGH);   // turn off the main pump
 
                     }                
            }
            
        else
            {
                digitalWrite (samplingPump, HIGH);  //turn of sampling pump
                digitalWrite (mainPump, HIGH);  // turn of the main pump anyway,
            
             }
     
        if (intercept == 60)
            {
                  averageRead = (read25+read35+read45+read55)/4;

                  dataString += String(averageRead);
                  dataString += ",";
                  dataString += String(setValue);
                  dataString += ",";
                  dataString += String(_pumpON);
                  _pumpON = 0;       //reset counter for the number of time when the main pump is ON


                   // The USB flash card is mounted at "/mnt/sda1" by default
                  File dataFile = FileSystem.open("/mnt/sda1/data/logturbid.csv", FILE_APPEND);
                 
                  // if the file is available, write to it:
                  if (dataFile) 
                      {
                            dataFile.println(dataString);
                            dataFile.close();
                            // print to the serial port too:
                            Console.print("Recording: ");
                            Console.println(dataString);
                      }  
                  // if the file isn't open, pop up an error:
                  else {
                        Console.println("error opening logturbid.csv");
                        } 
            }
       oled.setTextSize(2);
       oled.setTextColor(WHITE);
       oled.println("Set |Read");
       oled.print(setValue);
       oled.print(" |");
       oled.println(aRead);
       oled.display();
       if (digitalRead (samplingPump) == LOW)
       deLay = 2000;  /// two second
       else deLay = 10000;  /// 10 seconds
       
       if (intercept <=60)
          {
            Console.print("Sampling: ");
            Console.print(intercept);
            Console.print(" Sampling round: 60, ");       
          }
        else
        {
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
       if (digitalRead(samplingPump) == 1)
            {
              Console.print("OFF");
            }
       else Console.print("ON");   
       
       Console.print(" Main Pump: ");
       if (digitalRead(mainPump) == 1)
            {
              Console.println("OFF");
            }
       else Console.println("ON.");  
                  
      delay(deLay);
      oled.clearDisplay();
      
      count ++;
}
 
// getTimeStamp function return a string with the time stamp
// Yun Shield will call the Linux "date" command and get the time stamp
String getTimeStamp() {
  String result;
  Process time;
  // date is a command line utility to get the date and the time 
  // in different formats depending on the additional parameter 
  time.begin("date");
  time.addParameter("+%D %T");  // parameters: D for the complete date mm/dd/yy
                                //             T for the time hh:mm:ss    
  time.run();  // run the command
 
  // read the output of the command
  while(time.available()>0) {
    char c = time.read();
    if(c != '\n')
      result += c;
  }
 
  return result;
}
