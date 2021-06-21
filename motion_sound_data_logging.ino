/**
   Detects 'Active' and 'Inactive' motion as well as sound sensed by sound sensor and logs it with timestamps to a .csv file in SDCard.
   Uses  HCSR505 PIR Passive Infra Red Motion Detector and XC444 Sound Sensor Module.
   Modified code from https://learn.adafruit.com/adafruit-data-logger-shield/using-the-real-time-clock-3
   https://github.com/adafruit/Light-and-Temp-logger
   Modified by: Sarah Satish Masih
   Author: Niroshinie Fernando 
*/

//Libraries
#include "SD.h"
#include <Wire.h>
#include "RTClib.h"

#define LOG_INTERVAL  1000 // mills between entries. 

#define SYNC_INTERVAL 1000 // mills between calls to flush() - to write data to the card
uint32_t syncTime = 0; // time of last sync()


/*
 determines whether the code written on the serial moniter should be 
 also noted down on the logfile

*/
#define ECHO_TO_SERIAL   1 // echo data to serial port. 
const int ledpin=9;
const int redPin= 3;
const int bluePin= 3;
const int greenPin= 3;
const int soundpin=A2;
const int threshold=520;
//Variables
char active [] = "Active";
char inactive [] = "Inactive";
char* state ;

RTC_DS1307 RTC; // define time object 

// for the data logging shield, we use digital pin 10 for the SD cs line

const int chipSelect = 10;

// the logging file
File logfile;

void setup()
{
  Serial.begin(9600);

  // initialize the SD card
  initSDcard();

  // create a new file
  createFile();


  /**
   * connect to RTC
   */
  initRTC();


  /**
     Now we print the header. The header is the first line of the file and helps your spreadsheet or math program identify whats coming up next.
     The data is in CSV (comma separated value) format so the header is too: "millis,stamp,datetime,ativity,soundsens,sensor" the first item millis is milliseconds since the Arduino started,
     stamp is the timestamp, datetime is the time and date from the RTC in human readable format, activity displays motion is present or not, soundsens gives out sound sensed, 
     sensor shows if any sensor is active at that time.
  */
  logfile.println("millis,stamp,datetime,activity,soundsens,sensor");
#if ECHO_TO_SERIAL
  Serial.println("millis,stamp,datetime,activity,soundsens,sensor");
#endif //ECHO_TO_SERIAL

  pinMode(6, INPUT);
  pinMode(ledpin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT); 
  Serial.println("Start");
}

void loop()
{
  DateTime now;

  // delay for the amount of time we want between readings
  delay((LOG_INTERVAL - 1) - (millis() % LOG_INTERVAL));

  // log milliseconds since starting
  uint32_t m = millis();
  logfile.print(m);           // milliseconds since start
  logfile.print(", ");
#if ECHO_TO_SERIAL
  Serial.print(m);         // milliseconds since start
  Serial.print(", ");
#endif

  // fetch the time
  now = RTC.now();
  // log time
  logfile.print(now.unixtime()); // seconds since 2000
  logfile.print(", ");
  logfile.print(now.year(), DEC);
  logfile.print("/");
  logfile.print(now.month(), DEC);
  logfile.print("/");
  logfile.print(now.day(), DEC);
  logfile.print(" ");
  logfile.print(now.hour(), DEC);
  logfile.print(":");
  logfile.print(now.minute(), DEC);
  logfile.print(":");
  logfile.print(now.second(), DEC);
#if ECHO_TO_SERIAL
  Serial.print(now.unixtime()); // seconds since 2000
  Serial.print(", ");
  Serial.print(now.year(), DEC);
  Serial.print("/");
  Serial.print(now.month(), DEC);
  Serial.print("/");
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(":");
  Serial.print(now.minute(), DEC);
  Serial.print(":");
  Serial.print(now.second(), DEC);
#endif //ECHO_TO_SERIAL

  //Read data and store it to variable
int soundsens= analogRead(soundpin);

  if (digitalRead(6) == HIGH) //checking if motion sensor detects something
  {

       state = active;
   Serial.print(", MOTION: PRESENT ");
   Serial.print(",");
   Serial.print(soundsens);
   
  logfile.print(", MOTION: PRESENT");
  logfile.print(",");
  logfile.print(soundsens);
  
  //digitalWrite(bluePin,LOW);
  digitalWrite(ledpin,HIGH);
 // digitalWrite(redPin,LOW);
  //  digitalWrite(greenPin,HIGH);

  //delay(10000);
      //  digitalWrite(ledpin,LOW);

    }
  else if (soundsens>=threshold)//checking if sound sensed is above the threshold
  {
    state = active;
    Serial.print(", MOTION: INACTIVE");
    Serial.print(",");
    Serial.print(soundsens);
    
    logfile.print(",MOTION: INACTIVE");
    logfile.print(",");
    logfile.print(soundsens);
      digitalWrite(ledpin,HIGH);
/*
    digitalWrite(bluePin,HIGH);
    digitalWrite(greenPin,LOW);
    digitalWrite(redPin,LOW);*/
  //  delay(10000);
         // digitalWrite(ledpin,LOW);

  }
  else //sound is below threshold and no motion detected
  {
    Serial.print(", MOTION: INACTIVE");
    Serial.print(",");
    Serial.print(soundsens);
    
    logfile.print(",MOTION: INACTIVE");
    logfile.print(",");
    logfile.print(soundsens);
    
    state = inactive;
      digitalWrite(ledpin,LOW);
/*
    digitalWrite(bluePin,LOW);
    digitalWrite(greenPin,LOW);
    digitalWrite(redPin,LOW);*/
    //delay(1000);
  }

  logfile.print(", ");
  logfile.println(state);
    delay(200);


#if ECHO_TO_SERIAL
  Serial.print(", ");
  Serial.println(state);


#endif //ECHO_TO_SERIAL

  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();

  logfile.flush();
}


/**

error incase the SD Card starts malfunctioning, e.g does not open
*/
void error(char const *str)
{
  Serial.print("error: ");
  Serial.println(str);

  while (1);
}

void initSDcard()
{
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");

}

void createFile()
{
  //file name must be in 8.3 format (name length at most 8 characters, follwed by a '.' and then a three character extension.
  char filename[] = "MLOG00.CSV";
  for (uint8_t i = 0; i < 100; i++) {
    filename[4] = i / 10 + '0';
    filename[5] = i % 10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE);
      break;  // leave the loop!
    }
  }

  if (! logfile) {
    error("couldnt create file");
  }

  Serial.print("Logging to: ");
  Serial.println(filename);
}

void initRTC()
{
  Wire.begin();
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL

  }
}
