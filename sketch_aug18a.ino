#include <AltSoftSerial.h>
#include <Firebase_ESP_Client.h>
#include<DHT.h>
//#include<WiFi.h>
#include <ESP8266WiFi.h>
#define DHTPIN 13 
#define DHTTYPE DHT11
#define RE 6
#define DE 7
#define RE1 8
#define DE1 9

FirebaseAuth auth;
FirebaseConfig config; 
bool signupOK=false;
DHT dht(DHTPIN,DHTTYPE);
FirebaseData fbdo;
const int potPin=A0;
float ph;
float Value=0;

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"


char ssid[] = "tomtom";  // type your wifi name
char pass[] = "tomtom17041";  // type your wifi password

 
const byte ec[] = {0x01, 0x03, 0x00, 0x15, 0x00, 0x01, 0x95, 0xCE};
const byte salinity[] = {0x01, 0x03, 0x00, 0x14, 0x00, 0x01, 0xC4, 0x0E};
byte values[8]; 
const byte nitro[] = {0x01, 0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[] = {0x01, 0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte pota[] = {0x01, 0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};
byte values[11];
AltSoftSerial mod;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  dht.begin();
  pinMode(potPin,INPUT);
  WiFi.begin(ssid,pass);
  Serial.print("Connecting");
  while(WiFi.status()!=WL_CONNECTED){
    Serial.println(".");
    delay(500);

  }
  Serial.println();
  Serial.println("Connected");
  Serial.println(WiFi.localIP());
  config.database_url= "https://esp32test-ccee2-default-rtdb.firebaseio.com/";
  config.api_key= "AIzaSyBXWJ2vCbjFN2C6S45kPHRhIeKCFEqhEqg";
  if(Firebase.signUp(&config,&auth,"","")){
    Serial.println("ok");
    signupOK=true;
  }
  else
  {
    Serial.printf("%s\n",config.signer.signupError.message.c_str());
  }
  config.token_status_callback=tokenStatusCallback;
  Firebase.begin(&config,&auth);
  Firebase.reconnectWiFi(true);
}

void loop() {

  // put your main code here, to run repeatedly:
  float h=dht.readHumidity();
  float t=dht.readTemperature();
  if(Firebase.ready()&&signupOK){
    if(Firebase.RTDB.setFloat(&fbdo,"DHT_hum/humidity",h)){
      Serial.print("humidity");
      Serial.println(h);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: "+ fbdo.errorReason());
    }
    if(Firebase.RTDB.setFloat(&fbdo,"DHT_temp/temperature",t)){
      Serial.print("temperature");
      Serial.println(t);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: "+ fbdo.errorReason());
    }

    
    Value= analogRead(potPin);
    //Serial.print(Value);
    //Serial.print(" | ");
    //float voltage=Value*(3.3/4095.0);
    //ph=(3.3*voltage);
    ph=5;
    Serial.println(ph);
    if(Firebase.RTDB.setFloat(&fbdo,"pH_sense/pH",ph)){
      Serial.print("pH:");
      Serial.println(h);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: "+ fbdo.errorReason());
    }
    
    

    byte val1, val2, val3;
    Serial.print("Nitrogen: ");
    val1 = nitrogen();
    Serial.print(" = ");
    Serial.print(val1);
    Serial.println(" mg/kg");
    delay(250);
 
    Serial.print("Phosphorous: ");
    val2 = phosphorous();
    Serial.print(" = ");
    Serial.print(val2);
    Serial.println(" mg/kg");
    delay(250);
   
    Serial.print("Potassium: ");
    val3 = potassium();
    Serial.print(" = ");
    Serial.print(val3);
    Serial.println(" mg/kg");
    Serial.println();
    Serial.println();

    if(Firebase.RTDB.setFloat(&fbdo,"NPK1/Nitrogen",val1)){
      Serial.print("nitrogen");
      Serial.println(val1);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: "+ fbdo.errorReason());
    }
    if(Firebase.RTDB.setFloat(&fbdo,"NPK2/Phosphorus",val2)){
      Serial.print("phosphorus");
      Serial.println(val2);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: "+ fbdo.errorReason());
    }
    if(Firebase.RTDB.setFloat(&fbdo,"NPK3/Potassium",val3)){
      Serial.print("potassium");
      Serial.println(val3);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: "+ fbdo.errorReason());
    }
    
    digitalWrite(DE1, HIGH);
    digitalWrite(RE1, HIGH);
    delay(10);
   
    if (mod.write(ec, sizeof(ec)) == 8)
    {
      digitalWrite(DE1, LOW);
      digitalWrite(RE1, LOW);
      for (byte i = 0; i < 7; i++)
      {
        values[i] = mod.read();
        Serial.print(values[i], HEX);
      }
      Serial.println();
    }
    int soil_ec = int(values[3]<<8|values[4]);
    delay(1000);
    
    if(Firebase.RTDB.setFloat(&fbdo,"soil_ec/EC",soil_ec)){
      Serial.print("Soil EC");
      Serial.println(soil_ec);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: "+ fbdo.errorReason());
    }
    
    int moist = analogRead(A1);
    
    if(Firebase.RTDB.setFloat(&fbdo,"soil_moist/moisture",moisture)){
      Serial.print("Soil moisture:");
      Serial.println(moist);
    }
    else{
      Serial.println("FAILED");
      Serial.println("REASON: "+ fbdo.errorReason());
    }

     
    delay(500);
     
      
  }
}
byte nitrogen() 
    {
    // clear the receive buffer
      mod.flushInput();
     
      // switch RS-485 to transmit mode
      digitalWrite(DE, HIGH);
      digitalWrite(RE, HIGH);
      delay(1);
     
      // write out the message
      for (uint8_t i = 0; i < sizeof(nitro); i++ ) mod.write( nitro[i] );
     
      // wait for the transmission to complete
      mod.flush();
      
      // switching RS485 to receive mode
      digitalWrite(DE, LOW);
      digitalWrite(RE, LOW);
     
      // delay to allow response bytes to be received!
      delay(200);
     
      // read in the received bytes
      for (byte i = 0; i < 7; i++) {
        values[i] = mod.read();
        Serial.print(values[i], HEX);
        Serial.print(' ');
      }
      return values[4];
    }
   
  byte phosphorous() {
    mod.flushInput();
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(1);
    for (uint8_t i = 0; i < sizeof(phos); i++ ) mod.write( phos[i] );
    mod.flush();
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
  // delay to allow response bytes to be received!
    delay(200);
    for (byte i = 0; i < 7; i++) {
      values[i] = mod.read();
      Serial.print(values[i], HEX);
      Serial.print(' ');
    }
    return values[4];
  }
   
  byte potassium() {
    mod.flushInput();
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(1);
    for (uint8_t i = 0; i < sizeof(pota); i++ ) mod.write( pota[i] );
    mod.flush();
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
  // delay to allow response bytes to be received!
    delay(200);
    for (byte i = 0; i < 7; i++) {
      values[i] = mod.read();
      Serial.print(values[i], HEX);
      Serial.print(' ');
    }
    return values[4];
  }
