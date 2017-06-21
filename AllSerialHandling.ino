void serialOutput(){
  switch(outputType){
    case PROCESSING_VISUALIZER:
      sendDataToSD('S', Signal);
      break;
    case SERIAL_PLOTTER:
      Serial.println();
      Serial.print(BPM);
      Serial.print(",");
      Serial.print(IBI);
      Serial.print(",");
      Serial.print(Signal);
      break;
    default:
      break;
  }
}

void serialOutputWhenBeatHappens(){
  switch(outputType){
    case PROCESSING_VISUALIZER:
      sendDataToSD('B',BPM);
      sendDataToSD('Q',IBI); 
    default:
      break;
  }
}

void sendDataToSD(char symbol, int data ){
    Serial.print(symbol);
    Serial.println(data);
  }
