
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
void RFID_SETUP(){
  Serial.println("Open");
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  delay(4);       // Optional delay. Some board do need more time after init to be ready, see Readme
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println("RFID IS READY TO USE");
}


/**
 * Check the RFID is RFID is present, and is new card is present or not
 */
bool check_RFID(){
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return 0;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return 0;
  }
  Serial.println("RFID Present");
  return 1;
}

/**
 * read the RFID card and return the UID value
 * @return String of UID value of the card
 */
String read_RFID(){
  Serial.println("Reading RFID UID");
  String userid;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
  Serial.print(mfrc522.uid.uidByte[i], HEX); 
  userid += String(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print("User UID: ");
  Serial.println(userid);
  return userid;
}

// CHECK TEMPERATURE MODULE
/**
 * read the temperature from temperature sensor
 * @return float of temperature
 */ 
float read_temperature(){
  Serial.println("Checking Temperature");
  mlx.begin();
  float temp_1 = mlx.readObjectTempC();
  Serial.print("First Temp: ");
  Serial.println(temp_1);
  delay(500);
  float temp_2 = mlx.readObjectTempC();
  Serial.print("Second Temp: ");
  Serial.println(temp_2);
  
  if(temp_1 >= 30 && temp_2 > 30){
    if(temp_1 - temp_2 <= 0.5 || temp_2 - temp_1 <= 0.5) {
      Serial.println("Temp OK");
      return temp_2;
    } else {
      Serial.println("Temp Is not OK");
      return 0;
    }
  } else {
    return 0;
  }
}

//CHECK MOTION
int check_motion(){
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
  if(temp <= 33.7) user_status = "Ok";
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
  Serial.println("Begin API POST");
  HTTPClient http;
  http.begin("http://192.168.0.14:7070/api/userLog/albertque"); 
  http.addHeader("Content-Type", "application/json");
  Serial.print(uid);
  Serial.print(temp);
  Serial.print(user_status);
  int httpResponseCode = http.POST("{\"uid\": \""+uid+"\",\"temperature\": \""+temp+"\",\"status\": \""+user_status+"\"}");
  Serial.print(httpResponseCode);
  if(httpResponseCode == 200) Serial.println("Api POST OK, Response is 200");
  if(httpResponseCode == 400) Serial.println("Api POST OK, Status not OK, Response is 400");
  if(httpResponseCode != 200 || httpResponseCode != 400) Serial.println("API POST IS NOT OK, CHECK IP OR SERVER");
  return httpResponseCode;
}

void warningDetect(){
  //TODO
  Serial.println("WARNING IS CALLED");
}

void dispenseLiquid(){
  Serial.println("MOVE SERVO TO 90 DEGREE");
  servo.write(90);
  delay(1500);
  Serial.println("MOVE SERVO TO POSITION");
  servo.write(0);
}
void setup() {
  // Setup Wifi Manager For using wifi
  Serial.begin(9600);
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // put your setup code here, to run once:
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
     RFID_SETUP();
     Serial.println("SETUP THE SERVO");
     servo.attach(SERVO_PIN);
}

void loop() {
  Serial.println("CHECK FOR RFID DATA");
  bool RFID_check = check_RFID();
  while(RFID_check == 0){
    RFID_check = check_RFID();
    delay(0);
  }
  String RFID_UID = read_RFID();
  Serial.println("RFID DATA IN, CONTINUE TO READ TEMP");
  float temp = 0;
  Serial.println("READ TEMPERATURE START");
  while(temp == 0){
    temp = read_temperature();
    delay(0);
  }
  Serial.println("TEMPERATURE READ IS OK, TEMPERATURE IS OK");
  Serial.println("CHECK FOR USER STATUS");
  String user_status = get_status(temp);
  Serial.println("STATUS IS OK, BEGIN API POST");
  int respondCode = POST_API(RFID_UID,temp,user_status);
  if(respondCode == 400){
    warningDetect();
  }
  Serial.println("RESPONSE IS OK, CHECKING MOTION");
  int isMotion = check_motion();
  while(isMotion == 0){
    isMotion = check_motion();
    delay(0);
  }
  Serial.println("MOTION IS OK, READY TO DISPENSE");
  dispenseLiquid();
  Serial.println("SEQUENCE FINISHED");
}
