#include <SPI.h>
#include <SD.h>
#include <Time.h>
#define PROCESSING_VISUALIZER 1
#define SERIAL_PLOTTER  2

File fd;
const uint8_t BUFFER_SIZE = 20;
char fileName[] = "SmtShoe.txt";
char buff[BUFFER_SIZE + 2] = "";
uint8_t index = 0;

const uint8_t chipSelect = 8;
const uint8_t cardDetect = 9;

enum states : uint8_t { NORMAL, E, EO };
uint8_t state = NORMAL;

bool alreadyBegan = false;
int count = 0;

volatile int BPM;
volatile int Signal;
volatile int IBI = 600;
volatile boolean Pulse = false;
volatile boolean QS = false;

int pulsePin = 0;
int blinkPin = 13;
int fadePin = 5;
int fadeRate = 0;


static int outputType = SERIAL_PLOTTER;


void setup()
{
  Serial.begin(115200);
  while (!Serial);
  pinMode(cardDetect, INPUT);
  initializeCard();
  interruptSetup();
}

void loop()
{
  fd = SD.open(fileName, FILE_WRITE);
  if (!digitalRead(cardDetect))
  {
    initializeCard();
  }
  
  serialOutput() ;

  if (QS == true) {    // A Heartbeat Was Found
    serialOutputWhenBeatHappens();   // A Beat Happened, Output that to serial.
    QS = false;                      // reset the Quantified Self flag for next time
  }

  Serial.print("The BPM is ");
  Serial.print(BPM);
  Serial.println();

  delay(200);
  fd.println(BPM);
  fd.close();

}

void initializeCard(void)
{
  Serial.print(F("Initializing SD card..."));

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
    Serial.println(F(" exists."));
  }
  else
  {
    Serial.println(F(" doesn't exist. Creating."));
  }

  Serial.print("Opening file: ");
  Serial.println(fileName);
  Serial.println(F("Connect the pulse sensor to the user and disconnect the SD card to terminate the data collection."));
  
}

