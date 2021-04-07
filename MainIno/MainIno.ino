
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include <WiFiManager.h>
#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>


#define RST_PIN         D3          // Pin for RFID reset pin
#define SS_PIN          D4         // Pin for SS RFID pin
#define TRIG_PIN        D8        // Pin for PIR pin
#define ECHO_PIN        D7         // Pin for PIR pin
#define SERVO_PIN       D4          // Pin for Servo PWM pin

// Object Define
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo servo;
WiFiManager wifiManager;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

//RFID CHECK MODULE

/**
 * Setup the RFID to work
 */
bool RFID_SETUP(){
  Serial.println("Open");
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  delay(4);       // Optional delay. Some board do need more time after init to be ready, see Readme
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  bool isReady = mfrc522.PCD_PerformSelfTest();
  if(isReady){
    Serial.println("RFID IS READY TO USE");
    return true;
  } else {
    Serial.println("RFID IS NOT READY TO USE, CHECK CONNECTION");
    return false;
  }
  
}


/**
 * Check the RFID is RFID is present, and is new card is present or not
 */
bool check_RFID(){
  showLed("RFID","RED");
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }
  Serial.println("RFID Present");
  showLed("RFID","GREEN");
  return 1;
}

/**
 * read the RFID card and return the UID value
 * @return String of UID value of the card
 */
String read_RFID(){
  showLed("RFID","RED");
  Serial.println("Reading RFID UID");
  String userid;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
  Serial.print(mfrc522.uid.uidByte[i], HEX); 
  userid += String(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print("User UID: ");
  Serial.println(userid);
  showLed("RFID","GREEN");
  return userid;
}

// CHECK TEMPERATURE MODULE
/**
 * read the temperature from temperature sensor
 * @return float of temperature
 */ 
float read_temperature(){
  showLed("nonRFID","RED");
  Serial.println("Checking Temperature");
  mlx.begin();
  float temp_1 = mlx.readObjectTempC();
  Serial.print("First Temp: ");
  Serial.println(temp_1);
  delay(500);
  float temp_2 = mlx.readObjectTempC();
  Serial.print("Second Temp: ");
  Serial.println(temp_2);
  if(temp_1 > 34 && temp_2 > 34){
    if(temp_1 - temp_2 <= 0.5 || temp_2 - temp_1 <= 0.5) {
      showLed("nonRFID","GREEN");
      Serial.println("Temp OK");
      return temp_2;
    } else {
      showLed("nonRFID","RED");
      Serial.println("Temp Is not OK");
      return 0;
    }
  } else {
    return 0;
  }
}

//CHECK MOTION
int check_motion(){
  showLed("nonRFID","RED");
  pinMode(TRIG_PIN, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(ECHO_PIN, INPUT); // Sets the echoPin as an INPUT
  int distance;
  int isMotion = 0;
  long duration;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(ECHO_PIN, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
  // Displays the distance on the Serial Monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  if(distance <= 2){
    showLed("nonRFID","GREEN");
    isMotion = 1;
    Serial.println("Motion Detected");
  }
  return isMotion;
}

/**
 * Get user status from temperature
 * @params float temperature
 * @return String user_status
 */
  
String get_status(float temp){
  String user_status;
  if(temp <= 37.3) user_status = "Ok";
  else user_status = "Warning";
  return user_status;
}


// API MODULE
/**
 * Post API to server
 * @params String uid
 * @params float temp
 * @params String user_status
 * @return int httpResponseCode
 */
int POST_API (String uid, float temp, String user_status) {
  showLed("nonRFID","RED");
  Serial.println("Begin API POST");
  HTTPClient http;
  http.begin("http://192.168.0.14:7070/api/userLog/albertque"); 
  http.addHeader("Content-Type", "application/json");
  Serial.println("UID: "+uid);
  Serial.print("Temp: ");
  Serial.println(temp);
  Serial.println("Status: "+user_status);
  int httpResponseCode = http.POST("{\"uid\": \""+uid+"\",\"temperature\": \""+temp+"\",\"status\": \""+user_status+"\"}");
  Serial.println("StatusCode: "+httpResponseCode);
  if(httpResponseCode == 200) Serial.println("Api POST OK, Response is 200");
  else if(httpResponseCode == 400) Serial.println("Api POST OK, Status not OK, Response is 400");
  else Serial.println("API POST IS NOT OK, CHECK IP OR SERVER");
  return httpResponseCode;
}

// LED Module
/*
 * Show LED based on which mode and which modules is used
 * @params String Mode
 * @params String Value
 */
void showLed(String Mode, String Value){
  int mode;
  if(Mode == "nonRFID") mode = 1;
  else mode = 2;
  switch(mode){
    case '1':
      pinMode(D3, OUTPUT);
      pinMode(D2, OUTPUT); 
      if(Value == "green"){
        digitalWrite(D2, HIGH);
        digitalWrite(D3, LOW);
      } else {
        digitalWrite(D3, HIGH);
        digitalWrite(D2, LOW);
      }
      break;
    case '2':
      pinMode(D4, OUTPUT);
      pinMode(D6, OUTPUT); 
      if(Value == "green"){
        digitalWrite(D6, HIGH);
        digitalWrite(D4, LOW);
      } else {
        digitalWrite(D4, HIGH);
        digitalWrite(D6, LOW);
      }
  }
}

/*
 * Detect and show Warning When user_status is warning or user temperature is above 37.3
 */
void warningDetect(){
  Serial.println("WARNING IS CALLED");
  showLed("nonRFID","RED");
  pinMode(D8, OUTPUT);
  digitalWrite(D8, HIGH);
  delay(60000);
}

// Servo Module
/*
 * Dispense the liquid either soap or hand sanitizing liquid
 */
void dispenseLiquid(){
  Serial.println("MOVE SERVO TO 90 DEGREE");
  servo.write(90);
  delay(1500);
  Serial.println("MOVE SERVO TO POSITION");
  servo.write(0);
}

// Main Module
void setup() {
  // Setup Wifi Manager For using wifi
  Serial.begin(9600);
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  bool res = wifiManager.autoConnect("SmartSani");
    if(!res) {
        Serial.println("Failed to connect");
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("Connected:)");
    }
   // Setup the RFID
     Serial.println("SETUP THE RFID");
     // Commented while RFID module is being fixed
//   bool rfidStatus = RFID_SETUP();
//   if(rfidStatus == false) {
//    Serial.println("RFID SETUP FAILED, CHECK CONNECTION");
//    while(rfidStatus == false){
//      rfidStatus = RFID_SETUP();
//      delay(1000);
//      Serial.print("rfidStatus: ");
//      Serial.println(rfidStatus);
//    }
//   }
//   Serial.println("SETUP THE SERVO");
//   servo.attach(SERVO_PIN);
}

void loop() {
  Serial.println("CHECK FOR RFID DATA");
  bool RFID_check = check_RFID();
  unsigned long startMillis = millis(); // Store the current milis
  unsigned long elapsedMillis = 0; // store the elapsed millis
  // Commented while RFID module is being fixed
//  while(RFID_check == 0){
//    RFID_check = check_RFID();
//    delay(0);
//  }

  String RFID_UID = read_RFID();
  
  //String RFID_UID = "BABABCFD";
  Serial.println("RFID DATA IN, CONTINUE TO READ TEMP");
  float temp = 0;
  Serial.println("READ TEMPERATURE START");
  startMillis = millis();
  while(temp == 0){
    temp = read_temperature();
    delay(0);
    elapsedMillis = millis();
    if(elapsedMillis - startMillis >= 30000) return; // if 30s is elapsed and temp is still 0, then return the loop
  }
  Serial.println("TEMPERATURE READ IS OK, TEMPERATURE IS OK");
  Serial.println("CHECK FOR USER STATUS");
  String user_status = get_status(temp);
  Serial.println("STATUS IS OK, BEGIN API POST");
  if(user_status == "Warning") {
    warningDetect();
    int respondCode = POST_API(RFID_UID,temp,user_status);
    return;
  }
  int respondCode = POST_API(RFID_UID,temp,user_status);
  int tries = 1;
  while(respondCode != 200){
    if(tries => 6) return; // return to loop is tried is 6 or above 6 
    respondCode = POST_API(RFID_UID,temp,user_status);
    tries = tries+1;
    delay(0);
  }
  showLed("nonRFID","GREEN");
  Serial.println("RESPONSE IS OK, CHECKING MOTION");
  int isMotion = check_motion();
  startMillis = millis();
  while(isMotion == 0){
    isMotion = check_motion();
    delay(0);
    elapsedMillis = millis();
    if(elapsedMillis - startMillis >= 30000) return;
  }
  Serial.println("MOTION IS OK, READY TO DISPENSE");
  dispenseLiquid();
  Serial.println("SEQUENCE FINISHED");
}
