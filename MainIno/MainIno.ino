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


#define RST_PIN         9          // Pin for RFID reset pin
#define SS_PIN          10         // Pin for SS RFID pin
#define PIR_PIN         14         // Pin for PIR pin
#define SERVO_PIN       2          // Pin for Servo PWM pin

// Object Define
MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo servo;
WiFiManager wifiManager;
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

/**
 * Setup the RFID to work
 */
void RFID_SETUP(){
  Serial.begin(115200);    // Initialize serial communications with the PC
  while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  delay(4);       // Optional delay. Some board do need more time after init to be ready, see Readme
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

  return 1;
}

/**
 * read the RFID card and return the UID value
 * @return String of UID value of the card
 */
String read_RFID(){
  String userid;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
  Serial.print(mfrc522.uid.uidByte[i], HEX); 
  userid += String(mfrc522.uid.uidByte[i], HEX);
  }
  return userid;
}
/**
 * read the temperature from temperature sensor
 * @return float of temperature
 */ 
float read_temperature(){
  mlx.begin();
  float temp_1 = mlx.readObjectTempC();
  delay(500);
  float temp_2 = mlx.readObjectTempC();
  if(temp_1 >= 30 && temp_2 > 30){
    if(temp_1 - temp_2 <= 0.5 || temp_2 - temp_1 <= 0.5) {
      return temp_2;
    } else {
      return 0;
    }
  }
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

/**
 * Post API to server
 * @params String uid
 * @params float temp
 * @params String user_status
 * @return int httpResponseCode
 */
int POST_API (String uid, float temp, String user_status) {
  HTTPClient http;
  http.begin("http://192.168.0.12:7070/api/userLog/albertque"); 
  http.addHeader("Content-Type", "application/json");
  Serial.print(uid);
  Serial.print(temp);
  Serial.print(user_status);
  int httpResponseCode = http.POST("{\"uid\": \""+uid+"\",\"temperature\": \""+temp+"\",\"status\": \""+user_status+"\"}");
  Serial.print(httpResponseCode);
  return httpResponseCode;
       
}

void warningDetect(){
  
}

void setup() {
  pinMode(PIR_PIN, INPUT);
  // Setup Wifi Manager For using wifi
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  // put your setup code here, to run once:
  bool res = wifiManager.autoConnect("SmartSani");
    if(!res) {
        Serial.println("Failed to connect");
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
    }
   // Setup the RFID
   RFID_SETUP();
}

void loop() {
  bool RFID_check = check_RFID();
  while(RFID_check == 0){
    RFID_check = check_RFID();
  }
  String RFID_UID = read_RFID();
  float temp = 0;
  while(temp == 0){
    temp = read_temperature();
  }
  String user_status = get_status(temp);
  int respondCode = POST_API(RFID_UID,temp,user_status);
  if(respondCode == 400){ 
    //warning_detect();
    delay(900000);
    return;
  }
  long state = digitalRead(PIR_PIN);
  while(state == LOW)
  {
    state = digitalRead(PIR_PIN);
  }
  servo.write(90);
  delay(1500);
  servo.write(0);
}
