#include <SPI.h>
#include <SD.h>
#include <Time.h>
#define PROCESSING_VISUALIZER 1
#define SERIAL_PLOTTER  2

File fd;
char fileName[] = "SmtShoe.txt";

const uint8_t chipSelect = 8;
const uint8_t cardDetect = 9;

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
  Serial.begin(9600);
  while (!Serial);
  pinMode(cardDetect, INPUT);
  initializeCard();
  interruptSetup();
}

void loop()
{
  if (!digitalRead(cardDetect))
  {
    initializeCard();
  }
  fd = SD.open(fileName, FILE_WRITE);
  //serialOutput() ;

  //  if (QS == true) {    // A Heartbeat Was Found
  //    serialOutputWhenBeatHappens();   // A Beat Happened, Output that to serial.
  //    QS = false;                      // reset the Quantified Self flag for next time
  //  }

  if (fd)
  {
    fd.println(BPM);
    Serial.print("The BPM is ");
    Serial.print(BPM);
    Serial.println();
  }
  fd.close();
  delay(200);

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

