#include <SPI.h>
#include <SD.h>
#define PROCESSING_VISUALIZER 1
#define SERIAL_PLOTTER  2

File fd;
const uint8_t BUFFER_SIZE = 20;
char fileName[] = "SmartShoe.txt";
char buff[BUFFER_SIZE+2] = "";
uint8_t index = 0;

const uint8_t chipSelect = 8;
const uint8_t cardDetect = 9;

enum states: uint8_t { NORMAL, E, EO };
uint8_t state = NORMAL;

bool alreadyBegan = false;        

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
  if (!digitalRead(cardDetect))
  {
    initializeCard();
  }
  
  if (Serial.available() > 0)
  {
    readByte();

    if (index == BUFFER_SIZE)
    {
      flushBuffer();
    }
  }
                         
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
  Serial.println(F("Connect the pulse sensor to the user and typing 'EOF' in the serial monitor will terminate data."));
    serialOutput() ;

  if (QS == true){                             
        serialOutputWhenBeatHappens();  
        QS = false;                  
  }
  delay(200);  
}

void eof(void)
{
  index -= 3;
  flushBuffer();

  fd = SD.open(fileName);
  if (fd)
  {
    Serial.println("");
    Serial.print(fileName);
    Serial.println(":");

    while (fd.available())
    {
      Serial.write(fd.read());
    }

    Serial.println("");
  }
  else
  {
    Serial.print("Error opening ");
    Serial.println(fileName);
  }
  fd.close();
}

void flushBuffer(void)
{
  fd = SD.open(fileName, FILE_WRITE);
  if (fd) {
    switch (state)
    {
    case NORMAL:
      break;
    case E:
      readByte();
      readByte();
      break;
    case EO:
      readByte();
      break;
    }
    fd.write(buff, index);
    fd.flush();
    index = 0;
    fd.close();
  }
}

void readByte(void)
{
  byte byteRead = Serial.read();
  //Serial.write(byteRead);
  //Serial.print(byteRead);
  buff[index++] = byteRead;

  if (byteRead == 'E' && state == NORMAL)
  {
    state = E;
  }
  else if (byteRead == 'O' && state == E)
  {
    state = EO;
  }
  else if (byteRead == 'F' && state == EO)
  {
    eof();
    state = NORMAL;
  }
}
