#include <Wire.h>
#include "Waveshare_LCD1602_RGB.h"
#include <SPI.h>
#include <mcp2515.h>


Waveshare_LCD1602_RGB lcd(16,2);
struct can_frame canMsg;
MCP2515 mcp2515(9);




void setup() {
  Serial.begin(115200);
  lcd.init();
  
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS,MCP_16MHZ);
  mcp2515.setNormalMode();
  
}

uint8_t howlong(uint8_t a){
  uint8_t i=0;
  while(a>0){
    a = a/10;
    i++;

  }
  return i;
}


void showplz(int RPM,int distance){
  char numberArray[20];

  for(int i=0; i<20;i++)
    numberArray[i] = 0;

  ltoa(RPM, numberArray, 10);
    lcd.setCursor(0, 0);
    lcd.send_string("RPM: ");
    lcd.send_string(numberArray);

  for(int i=0; i<20;i++)
    numberArray[i] = 0;

  ltoa(distance, numberArray, 10);
    lcd.setCursor(0,1);
    lcd.send_string("Dis: ");
    lcd.send_string(numberArray);
    //intToch(distance);
    Serial.print("distance : ");
    Serial.println(distance);
    delay(1000);

}

void loop() {
  int RPM=0;
  int distance=0;

  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    Serial.print(canMsg.can_id, HEX); // print ID
    Serial.print(" "); 
    Serial.print(canMsg.can_dlc, HEX); // print DLC
    Serial.println(" ");

    RPM = canMsg.data[0] + canMsg.data[1]*256;
    distance = canMsg.data[2] + canMsg.data[3]*256;
    lcd.clear();
    showplz(RPM,distance);

    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
      Serial.print(canMsg.data[i],HEX);
      Serial.print(" ");
    }

    Serial.println();      
  }
}

