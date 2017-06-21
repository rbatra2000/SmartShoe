void serialOutput(){
  switch(outputType){
    case PROCESSING_VISUALIZER:
      sendDataToSD('S', Signal);
      Serial.println("HI");
      break;
    case SERIAL_PLOTTER:
      if (BPM > 0)
      {
        str = str + BPM + " ";
        Serial.println();
        Serial.print(BPM);
      }
      //Serial.print(",");
      //Serial.print(IBI);
      //Serial.print(",");
      //Serial.print(Signal);
      break;
    default:
      break;
  }
}

void SDOutputWhenBeatHappens(){
  switch(outputType){
    case PROCESSING_VISUALIZER:
      sendDataToSD('B',BPM);
      sendDataToSD('Q',IBI); 
    default:
      break;
  }
}

void sendDataToSD(char symbol, int data ){
    fd.print(symbol);
    fd.println(data);
  }
