#include <SD.h> //SD Card Library
#include <TinyGPS++.h>  //GPS library
#include <SoftwareSerial.h> //Serial Monitor

File fd;  //File in SD Card
char fileName[] = "SmtShoe.txt";  //Name of SD file

const uint8_t chipSelect = 10;  //SD card reader pin
const uint8_t cardDetect = 9; //Checks if Card exists

bool alreadyBegan = false;  //For SD Card; checking if the SD card is being changed
int count = 0;  //For counting number of Heart Rate data

int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0

volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded!
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat".
volatile boolean QS = false;   

static const int RXPin = 4, TXPin = 5;  //For GPS module to transfer and receive data
static const uint32_t GPSBaud = 9600; //Baud Rate for the GPS (Learned that it should be 9600 through trial and error)

boolean pulsing = true; //Is the pulse sensor scanning for BPM?
boolean outputting = true;  //Is the pulse sensor outputting the BPM values?
String dateVal = "";  //The date and value generated from the methods

TinyGPSPlus gps;  // The TinyGPS++ object
SoftwareSerial ss(RXPin, TXPin);  // The serial connection to the GPS device

int sum = 0;  //The sum of all BPM values

//This is the initialization of the program; it introduces the user to the program with prompts and sets up the motors.
void setup()
{
  Serial.begin(9600);
  Serial.println(F("***************************************************"));
  Serial.println(F("Welcome to the SmartShoe!\nIf you have any issues, comments, or questions, please contact WiseTech at customerservice@wisetech.com!\nType 'HELP' for additional information!"));
  pinMode(cardDetect, INPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  initializeCard();
  ss.begin(GPSBaud);
  interruptSetup();
}

//This is the main method which processes the prompts that the user sends through the serial monitor.
void loop()
{
  if (Serial.available())
  {
    //Serial.println("Testing");
    String str = Serial.readString();
    //Average BPM
    if (str.equalsIgnoreCase("avg"))
    {
      Serial.println();
      Serial.print(F("The average is "));
      Serial.print((sum+0.0)/count);
      Serial.println();
    }
    //Reset Count/Sum of BPM
    else if (str.equalsIgnoreCase("reset"))
    {
      pulsing = true;
      sum=0;
      count=0;
      Serial.println();
      Serial.println(F("Reset!"));
    }
    //Produces the keywords for the user
    else if (str.equalsIgnoreCase("help"))
    {
      Serial.println(F("'BPM'\t=\tBeats per Minute"));
      Serial.println(F("'AVG'\t=\tAverage Beats per Minute"));
      Serial.println(F("'NAV'\t=\tNavigation to a Location"));
      Serial.println(F("'RESET'\t=\tReset Beats Per Minute values"));
      Serial.println(F("'TIME'\t=\tCurrent Time"));
      Serial.println(F("'SPD'\t=\tCurrent Speed"));
      Serial.println(F("'ALT'\t=\tCurrent Altitude"));
      Serial.println(F("'HELP'\t=\tAll keywords"));

    }
    //Altitude of the user
    else if (str.equalsIgnoreCase("alt"))
    {
      outputting = false;
      pulsing = false;
      Serial.println(F("The altitude is: "));
      Serial.println(gps.altitude.miles());
    }
    //Navigate the user to a location using the vibration motors
    else if (str.equalsIgnoreCase("nav"))
    {
      pulsing = false;
      outputting = false;
      Serial.println(F("Enter latitude: "));
      while(Serial.available() == 0);
      double latit = Serial.parseFloat();
      Serial.println(latit);
      Serial.println(F("Enter longitude: "));
      while(Serial.available() == 0);
      double longit = Serial.parseFloat();
      Serial.println(longit);

    unsigned long distanceKm =
    (unsigned long)TinyGPSPlus::distanceBetween(
      gps.location.lat(),
      gps.location.lng(),
      latit, 
      longit) / 1000;
      Serial.println();
  Serial.print(F("Distance to Location:"));
  printInt(distanceKm, gps.location.isValid(), 9);

  double course =
    TinyGPSPlus::courseTo(
      gps.location.lat(),
      gps.location.lng(),
      latit, 
      longit);

  const char *cardinal = TinyGPSPlus::cardinal(course);
  
  while (distanceKm > 0)
  { 
  Serial.println();
  Serial.print(F("Direction to Destination:"));
  printStr(gps.location.isValid() ? cardinal : "*** ", 6);
  Serial.println(course);
  Serial.print(F("Your Direction:"));
  printStr(gps.course.isValid() ? TinyGPSPlus::cardinal(gps.course.value()) : "*** ", 6);
  Serial.println(gps.course.deg());
    if (Serial.readString().equalsIgnoreCase("Stop"))
    {
      digitalWrite(6,LOW); 
      digitalWrite(7,LOW); 
      setup();
      break;
    }  
//  Serial.println();
//  Serial.println(gps.course.deg());
//  Serial.println(course);
//  Serial.println();
  if (gps.course.deg() + 10 < course || gps.course.deg() - 10 > course) {
  if (gps.course.deg() > course || (gps.course.deg() > 270 && course < 90))
  {
    digitalWrite(6,HIGH);
    digitalWrite(7,LOW); 
  }
  else if (gps.course.deg() < course || (gps.course.deg() < 90 && course > 270))
  {
    digitalWrite(7,HIGH);  
    digitalWrite(6,LOW);   
  }
  
  smartDelay(0);
  delay(2000);
  }
  distanceKm =
    (unsigned long)TinyGPSPlus::distanceBetween(
      gps.location.lat(),
      gps.location.lng(),
      latit, 
      longit) / 1000;

  course =
    TinyGPSPlus::courseTo(
      gps.location.lat(),
      gps.location.lng(),
      latit, 
      longit);
       Serial.println();
       Serial.print(F("Current Latitude: "));
       printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
       Serial.println();
       Serial.print(F("Current Longitude"));       
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  Serial.println();
  Serial.print(F("Distance to Location:"));
  printInt(distanceKm, gps.location.isValid(), 9);
  
    smartDelay(1000);
  }
 }
 //BPM values
    else if (str.equalsIgnoreCase("bpm"))
    {
      outputting = true;
      printHeart();
    }
    //Speed of the user
    else if (str.equalsIgnoreCase("spd"))
    {
      pulsing=false;
      outputting=false;
      Serial.println("Current Speed:");
      Serial.println(gps.speed.mph());
    }
    //Current date and time
    else if (str.equalsIgnoreCase("time"))
    {
     pulsing = false;
     printDateTime(gps.date, gps.time);
     Serial.println();
     smartDelay(1000);
      if (millis() > 5000 && gps.charsProcessed() < 10)
        Serial.println(F("No GPS data received: please try again in a few minutes."));
    }
  }
  outputting = false;
  pulsing = true;
  printHeart();
}

//Print the heart rate BPM values (20 values) and stores in SD card.
void printHeart()
{
  pulsing = true;
  //Serial.println("BackTesting");
  if (!digitalRead(cardDetect))
        {
         initializeCard();
        }
  fd = SD.open(fileName, FILE_WRITE);
  if (fd)
  {
    if (count == 0)
    {
      pulsing = false;
      Serial.println();
      delay(100);
      printDateTime(gps.date, gps.time);
      //printDateTime(gps.date, gps.time);
      fd.println();
      fd.println(dateVal);
      smartDelay(1000);
      pulsing = true;
    }
    fd.println(BPM);
    if (outputting)
    {
      Serial.println();
      for (int i=0; i<20; i++)
      {
        Serial.println(BPM);
        delay(500);
      }
    }
    count ++;
    sum += BPM;
  }
  fd.close();
  delay(200);
}

//Initialize card for storing data
void initializeCard(void)
{
  if (outputting) {
  Serial.print(F("Please wait for the SD card to initialize..."));

  // Is there even a card?
  if (!digitalRead(cardDetect))
  {
    Serial.println(F("Please press the left side of the shoe."));
    while (!digitalRead(cardDetect));
    }
    delay(250);
    
  }

  if (!SD.begin(chipSelect) && !alreadyBegan)
  {
    if (outputting) {
    Serial.println(F("Initialization failed!"));
    }
    initializeCard();
  }
  else
  {
    alreadyBegan = true;
  }

  if (outputting) {
  Serial.println(F("Initialization done."));

  Serial.print(fileName);
  if (SD.exists(fileName))
  {
    Serial.println(F(" already exists."));
  }
  else
  {
    Serial.println(F(" doesn't exist. Creating."));
  }

  Serial.print("Opening file: ");
  Serial.println(fileName);
  if (count == 0)
    Serial.println(F("Please put the shoe on and use the commands for navigation, location, time/date, or heart rate!"));
}
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do 
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}

//Print an integer
//Parameters are the value to print, whether it is valid, and the length (which is useless)
static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0) 
    sz[len-1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}

//Print the Date and time
//Paramters are the date and time from the GPS
static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  if (!d.isValid())
  {
    Serial.print(F(""));
  }
  else
  {
    //Serial.println("Test");
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
    dateVal = sz;
  }
  
  if (!t.isValid())
  {
    Serial.print(F(""));
  }
  else
  {
    //Serial.println("Test2");
    int time = t.hour() - 7;
    if (time <= 0)
    {
      time = time + 24;
    }
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", time, t.minute(), t.second());
    Serial.print(sz);
    dateVal += sz;
  }
  smartDelay(0);
  Serial.println();
}

//Print a string
//Parameters are the string to print and the length of the space (which is useless)
static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartDelay(0);
}

//Print Float
//Parameters are the value, whether it is valid, and length/precision
static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);    
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
}


