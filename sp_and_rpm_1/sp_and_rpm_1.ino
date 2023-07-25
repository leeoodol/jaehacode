#include <SPI.h>
#include <mcp2515.h>

//--------distance--------------------
const int TRIG_PIN = 7;
const int ECHO_PIN = 6;

//------------rpm---------------------
const byte PulsesPerRevolution = 2;
const unsigned long ZeroTimeout = 100000;
const byte numReadings = 2;
volatile unsigned long LastTimeWeMeasured;
volatile unsigned long PeriodBetweenPulses = ZeroTimeout + 1000;
volatile unsigned long PeriodAverage = ZeroTimeout + 1000;
unsigned long FrequencyRaw;
unsigned long FrequencyReal;
unsigned int PulseCounter = 1;
unsigned long PeriodSum;
unsigned long LastTimeCycleMeasure = LastTimeWeMeasured;
unsigned long CurrentMicros = micros();
unsigned int AmountOfReadings = 1;
unsigned int ZeroDebouncingExtra;
unsigned long readings[numReadings];
unsigned long readIndex;  
unsigned long total; 
unsigned long average;

//-----------can----------------------------
struct can_frame speed;
MCP2515 mcp2515(9);  

long whatisDistance(){

  long duration, distance;
 
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN,HIGH);
 
  // convert the time into a distance
  distance = duration / 29.1 / 2 ;
  return distance;

}

void Pulse_Event() {
  PeriodBetweenPulses = micros() - LastTimeWeMeasured;
  LastTimeWeMeasured = micros();
  if (PulseCounter >= AmountOfReadings)  {
    PeriodAverage = PeriodSum / AmountOfReadings;
    PulseCounter = 1;
    PeriodSum = PeriodBetweenPulses;
    int RemapedAmountOfReadings = map(PeriodBetweenPulses, 40000, 5000, 1, 10);
    RemapedAmountOfReadings = constrain(RemapedAmountOfReadings, 1, 10);
    AmountOfReadings = RemapedAmountOfReadings;
  } 
  else {
    PulseCounter++;
    PeriodSum = PeriodSum + PeriodBetweenPulses;
  }
}

long whatisRpm(){
  long RPM;
  LastTimeCycleMeasure = LastTimeWeMeasured;
  CurrentMicros = micros();
  if (CurrentMicros < LastTimeCycleMeasure) {
    LastTimeCycleMeasure = CurrentMicros;
  }
  FrequencyRaw = 10000000000 / PeriodAverage;
  if (PeriodBetweenPulses > ZeroTimeout - ZeroDebouncingExtra || CurrentMicros - LastTimeCycleMeasure > ZeroTimeout - ZeroDebouncingExtra) {
    FrequencyRaw = 0;  // Set frequency as 0.
    ZeroDebouncingExtra = 2000;
  } else {
    ZeroDebouncingExtra = 0;
  }
  FrequencyReal = FrequencyRaw / 10000;

  RPM = FrequencyRaw / PulsesPerRevolution * 60;
  RPM = RPM / 10000;
  total = total - readings[readIndex];
  readings[readIndex] = RPM;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;

  if (readIndex >= numReadings) {
    readIndex = 0;
  }
  average = total / numReadings;

  return RPM;
}

void setup() {
  Serial.begin(115200);
  //lcd.init();
  //lcd.backlight();

  attachInterrupt(digitalPinToInterrupt(2), Pulse_Event, RISING);
  delay(1000);

//-------------distance setting---------------
  pinMode(TRIG_PIN,OUTPUT);
  pinMode(ECHO_PIN,INPUT);

//-------------can setting-------------------
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS,MCP_8MHZ);
  mcp2515.setNormalMode();
  speed.can_id = 0x123;
  speed.can_dlc = 8;
  for(int i=0;i<8;i++){
    speed.data[i] = 0;
  }
}

void loop() {

  long distance = whatisDistance();
  long RPM = whatisRpm();

  speed.data[0] = RPM%256;
  speed.data[1] = RPM/256;
  speed.data[2] = distance%256;
  speed.data[3] = distance/256;

  for(int i=0;i<8;i++){
    Serial.print(speed.data[i]);
    Serial.print(" ");
  }

  Serial.println();
  mcp2515.sendMessage(&speed);
  delay(100);
}
