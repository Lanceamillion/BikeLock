// Copyright 2022 Lance Nichols
// This one goes in your pocket

#include <SPI.h>
#include <RH_RF69.h>

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

enum STATUS {STANDBY,ARMED,ALARM};
STATUS lockStatus = STANDBY;

unsigned long beepTime;
unsigned long blinkTime;

bool blinkOn = true;
bool beepOn = false;

#define RF69_FREQ 915.0

//Create instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

void setup() {
  Serial.begin(115200);
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
  
  //Init test done
  delay(100);
  digitalWrite(Speaker, LOW);
  digitalWrite(LED, LOW);

  Serial.println("Setup Complete");
}

void loop() {
  checkButton();
  if(lockStatus == ARMED){
    checkRecieve();
    handleBlink();
  }
  if(lockStatus == ALARM){
    handleAlarm();
  }
}

void checkButton(){
  if(!digitalRead(Button)){
    if(lockStatus == STANDBY){//If standby
      transmitArm();
    }else if(lockStatus == ARMED){//Armed
      transmitDisarm();
    }else{//Alarm
      transmitArm();
    }
    delay(500);
    while(!digitalRead(Button));
  }
}

void transmitArm(){
  uint8_t data[] = "Arm";
  rf69.send(data, sizeof(data));
  rf69.waitPacketSent();
  Serial.println("Arm Sent");
  //Listen for ACK
  uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  if(rf69.waitAvailableTimeout(500)){
    if(rf69.recv(buf, &len)){
      Serial.print("Got a reply: ");
      Serial.println((char*)buf);
      lockStatus = ARMED;
      blinkOn = false;
      blinkTime = millis();
      digitalWrite(LED, LOW);
      digitalWrite(Speaker, LOW);
    }else{
      Serial.println("Receive failed");
    }
  }else{
    Serial.println("No Ack");
  }
}

void transmitDisarm(){
  uint8_t data[] = "Dis";
  rf69.send(data, sizeof(data));
  rf69.waitPacketSent();
  Serial.println("Dis Sent");
  //Listen for ACK
  uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
  if (rf69.waitAvailableTimeout(500)){
    if (rf69.recv(buf, &len)) {
      Serial.print("Got a reply: ");
      Serial.println((char*)buf);
      lockStatus = STANDBY;
      digitalWrite(LED, LOW);
    } else {
      Serial.println("Receive failed");
    }
  } else {
    Serial.println("No Ack");
  }
}

void checkRecieve(){
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
      if(strstr((char *)buf, "Alr")){
        lockStatus = ALARM;
        Serial.println("Alarm Signal Recieved");
      }
    } else {
      Serial.println("Receive failed");
    }
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
