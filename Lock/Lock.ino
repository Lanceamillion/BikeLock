// Copyright 2022 Lance Nichols
// This one goes on the bike

#include <SPI.h>
#include <RH_RF69.h>
#include <Wire.h>
#include <Adafruit_MSA301.h>
#include <Adafruit_Sensor.h>

Adafruit_MSA301 msa;

#define Speaker A1
#define LED A4
#define Button_LOW A5
#define Button A3

#define RFM69_CS      8
#define RFM69_INT     3
#define RFM69_RST     4

#define beepOnTime 150
#define beepOffTime 150
#define blinkOnTime 500
#define transmitOnTime 1000
#define threshold 105         //CHANGE THIS ONE TO SET SENSITIVITY

enum STATUS {STANDBY,ARMED,ALARM};
STATUS lockStatus = STANDBY;

unsigned long beepTime;
unsigned long transmitTime;
unsigned long blinkTime;

bool blinkOn = true;
bool beepOn = false;

char threshold = 40;

#define RF69_FREQ 915.0

//Create instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

void setup() {
  Serial.begin(115200);
  msa.begin(0x62);
  Serial.println("Begin setup...");
  
  //Setup Pins
  pinMode(RFM69_RST, OUTPUT);
  
  pinMode(Speaker, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(Button_LOW, OUTPUT);
  pinMode(Button, INPUT_PULLUP);
  
  //Init pins
  digitalWrite(RFM69_RST, LOW);
  
  digitalWrite(Speaker, HIGH);
  digitalWrite(LED, HIGH);
  digitalWrite(Button_LOW, LOW);

  //Reset Radio
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  
  //Init Radio
  if (!rf69.init()) {
    Serial.println("RFM69 radio init failed");
    while (1);
  }
  Serial.println("RFM69 radio init OK!");

  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }

  rf69.setTxPower(20, true);

  uint8_t key[] = { 0x1B, 0xF7, 0xC2, 0x9D, 0x53, 0xA8, 0x17, 0x0E,
                    0x2B, 0x43, 0xA7, 0x2E, 0xF3, 0xE9, 0xEE, 0xBD};
  rf69.setEncryptionKey(key);
  
  Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");
  //Init test done
  delay(100);
  digitalWrite(Speaker, LOW);
  digitalWrite(LED, LOW);
  Serial.println("Setup Complete");
}

void loop() {
  handleRadio();
  if(lockStatus == ARMED){
    checkBump();
    handleBlink();
  }
  if(lockStatus == ALARM){
    handleAlarm();
    handleTransmitAlarm();
  }
}

void handleBlink(){
  if(blinkOn){//If the led is blinking
    if(blinkTime + blinkOnTime < millis()){//If time has passed
      blinkTime = millis();//Set the time
      blinkOn = false;//Set flag to off
      digitalWrite(LED, LOW);//LED off
    }
  }else{//If LED is not blinking
    if(blinkTime + blinkOnTime < millis()){//If time has passed
      blinkTime = millis();//Set the time
      blinkOn = true;//Set flag to off
      digitalWrite(LED, HIGH);//LED on
    }
  }
}

void handleAlarm(){
  digitalWrite(LED, HIGH);//LED on
  if(beepOn){//If the speaker is beeping
    if(beepTime + beepOffTime < millis()){//If time has passed
      beepTime = millis();//Set the time
      beepOn = false;//Set flag to off
      digitalWrite(Speaker, LOW);//Beep off
    }
  }else{//If speaker is not beeping
    if(beepTime + beepOnTime < millis()){//If time has passed
      beepTime = millis();//Set the time
      beepOn = true;//Set flag to off
      digitalWrite(Speaker, HIGH);//Beep on
    }
  }
}

void AlarmOff(){
  beepOn = false;//Set flag to off
  digitalWrite(Speaker, LOW);//Beep off
  digitalWrite(LED, LOW);//LED off
}

void handleTransmitAlarm(){
  if(transmitTime + transmitOnTime < millis()){//If time has passed
    transmitTime = millis();//Set the time
    uint8_t data[] = "Alr";
    rf69.send(data, sizeof(data));
    rf69.waitPacketSent();
    Serial.println("Alr Sent");
  }
}

void checkBump(){
  sensors_event_t event; 
  msa.getEvent(&event);
  if(event.acceleration.x*event.acceleration.x+event.acceleration.y*event.acceleration.y+event.acceleration.z*event.acceleration.z > threshold){
    lockStatus = ALARM;
    blinkOn = true;
    beepOn = false;
    digitalWrite(Speaker, LOW);//Beep off
    Serial.println("moved!");
  }
}

void handleRadio(){
  if (rf69.available()) {
    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf69.recv(buf, &len)) {
      if (!len) return;
      buf[len] = 0;
      Serial.print("Received [");
      Serial.print(len);
      Serial.print("]: ");
      Serial.println((char*)buf);
      Serial.print("RSSI: ");
      Serial.println(rf69.lastRssi(), DEC);

      if(strstr((char *)buf, "Arm")){//If arm signal is recieved
        lockStatus = ARMED;
        blinkOn = false;
        blinkTime = millis();
        Serial.println("Lock Armed");
        AlarmOff();
        sendAck();
      }
      if(strstr((char *)buf, "Dis")){//If disarm signal is recieved
        lockStatus = STANDBY;
        Serial.println("Lock Disarmed");
        AlarmOff();
        sendAck();
      }
    }else{
      Serial.println("Receive failed");
    }
  }
}

void sendAck(){
  uint8_t data[] = "Ack";
  rf69.send(data, sizeof(data));
  rf69.waitPacketSent();
  Serial.println("Ack Sent");
}
