#include <SD.h>
#define PROCESSING_VISUALIZER 1
#define SERIAL_PLOTTER  2
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

File fd;
char fileName[] = "SmtShoe.txt";

const uint8_t chipSelect = 10;
const uint8_t cardDetect = 9;

bool alreadyBegan = false;
int count = 0;

int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0

volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded!
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat".
volatile boolean QS = false;   

static const int RXPin = 4, TXPin = 3;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

int sum = 0;

static int outputType = SERIAL_PLOTTER;

void setup()
{
  Serial.begin(9600);
  ss.begin(GPSBaud);
  Serial.println("***************************************************");
  Serial.println("Welcome to the SmartShoe!\nIf you have any issues, comments, or questions, please contact WiseTech at customerservice@wisetech.com!");
  pinMode(cardDetect, INPUT);
  initializeCard();
  //interruptSetup();
}

void loop()
{

  if (Serial.available())
  {
    String str = Serial.readString();
    if (str.equalsIgnoreCase("average"))
    {
      Serial.println();
      Serial.print("The average is ");
      Serial.print((sum+0.0)/count);
      Serial.println();
    }
    else if (str.equalsIgnoreCase("reset"))
    {
      Serial.println();
      sum=0;
      count=0;
      Serial.println("Reset!");
    }
    else if (str.equalsIgnoreCase("bpm"))
    {
      Serial.println();
      for (int i=0; i < 20; i++)
      {
        if (!digitalRead(cardDetect))
        {
         initializeCard();
        }
        fd = SD.open(fileName, FILE_WRITE);

        if (fd)
        {
          printDateTime(gps.date, gps.time);
          fd.println(BPM);
          Serial.print("The BPM is ");
          Serial.print(BPM);
          count++;
          sum += BPM;
          Serial.println();
        }
        fd.close();
        delay(200);
      }
    }
    else if (str.equalsIgnoreCase("time"))
    {
     printDateTime(gps.date, gps.time);
 
     Serial.println();
  
     smartDelay(1000);

      if (millis() > 5000 && gps.charsProcessed() < 10)
        Serial.println(F("No GPS data received: check wiring"));
    }
  }
}
  
void initializeCard(void)
{
  Serial.print(F("Please wait for the SD card to initialize..."));

  // Is there even a card?
  if (!digitalRead(cardDetect))
  {
    Serial.println(F("No card detected. Waiting for card."));
    while (!digitalRead(cardDetect));
    delay(250);
  }

  if (!SD.begin(chipSelect) && !alreadyBegan)
  {
    Serial.println(F("Initialization failed!"));
    initializeCard();
  }
  else
  {
    alreadyBegan = true;
  }

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
  Serial.println(F("Please put the shoe on and use the commands for destination, location, time/date, or heart rate!"));
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

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
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

static String printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }
  
  if (!t.isValid())
  {
    return (F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour()-7, t.minute(), t.second());
    Serial.print(sz);
  }
  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartDelay(0);
}

