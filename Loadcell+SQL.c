#include <HX711_ADC.h>
#include <WiFi.h> // calling the wifi library 
#include <WiFiClient.h> // create client that able to connect the internet IP address and port as client 
#include <WebServer.h>  
#include <SPI.h> // Serial Peripheral Interface where communicating device in short distance 
#include <MFRC522.h> // allow to read or write data from RFID tags easily
#include <ESPmDNS.h> // Multicast DNS (mDNS) provides a naming service system that is easy to set up and maintain, for computers on a local link

#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif

//pins:
const int HX711_dout = 4; //mcu > HX711 dout pin
const int HX711_sck = 5; //mcu > HX711 sck pin

//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);
const int calVal_calVal_eepromAdress = 0;
unsigned long t = 0;
float Weight=0;
float calibrationValue = 696.0;
static boolean newDataReady = 0;
const int serialPrintInterval = 500; //increase value to slow down serial print activity
  
// declaration of the  Wifi 
const char* ssid = "SATASIA1_A7000R";// 
const char* password = "Strand01";
//WiFiClient client;
char server[] = "192.168.0.144";   //eg: 192.168.0.222


WiFiClient client;    


void setup()
{
  Serial.begin(9600); 
  delay(10);
  Serial.println();

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password); // ESP32 accessing the wifi 
 
  while (WiFi.status() != WL_CONNECTED) //once connected to the wifi it will break the loop 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  Serial.println("Server started");
  Serial.print(WiFi.localIP());
  delay(1000);
  Serial.println("connecting...");

  Serial.println("Starting...");
   // calibration value
  
#if defined(ESP8266) || defined(ESP32)
#endif
  LoadCell.begin();
  unsigned long stabilizingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilizing time
  boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  LoadCell.start(stabilizingtime, _tare);
  if (LoadCell.getTareTimeoutFlag()) 
  {
    Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
  }
  else 
  {
    LoadCell.setCalFactor(calibrationValue); // set calibration factor (float)
    Serial.println("Startup is complete");
  }
  while (!LoadCell.update());
  {
      Serial.print("Calibration value: ");
      Serial.println(LoadCell.getCalFactor());
      Serial.print("HX711 measured conversion time ms: ");
      Serial.println(LoadCell.getConversionTime());
      Serial.print("HX711 measured sampling rate HZ: ");
      Serial.println(LoadCell.getSPS());
      Serial.print("HX711 measured settlingtime ms: ");
      Serial.println(LoadCell.getSettlingTime());
      Serial.println("Note that the settling time may increase significantly if you use delay() in your sketch!");
      if (LoadCell.getSPS() < 7) 
      {
        Serial.println("!!Sampling rate is lower than specification, check MCU>HX711 wiring and pin designations");
      }
      else if (LoadCell.getSPS() > 100) 
      {
        Serial.println("!!Sampling rate is higher than specification, check MCU>HX711 wiring and pin designations");
      }
  }



  
 }
void loop()
{ 
  
 // check for new data/start next conversion:
  if (LoadCell.update()) 
  {
    newDataReady = true;
  }
  // get smoothed value from the dataset:
  Sending_To_phpmyadmindatabase();  // call the function 
  delay(3000); // interval
 }

 void Sending_To_phpmyadmindatabase()   //CONNECTING WITH MYSQL
 {
   if (client.connect(server, 80))
   {
      // get smoothed value from the dataset:
      if (newDataReady) {
        if (millis() > t + serialPrintInterval) {
          Weight = LoadCell.getData();
          Serial.print("Load_cell output val: ");
          Serial.println(Weight);
          newDataReady = 0;
          t = millis();
        }
      }

      // receive command from serial terminal, send 't' to initiate tare operation:
      if (Serial.available() > 0) 
      {
        char inByte = Serial.read();
        if (inByte == 't') LoadCell.tareNoDelay();
      }

      // check if last tare operation is complete:
      if (LoadCell.getTareStatus() == true) 
      {
        Serial.println("Tare complete");
      }
     
    Serial.println("connected");
    // Make a HTTP request:
    Serial.print("GET /testcode/loadcell.php?Weight=");
    client.print("GET /testcode/loadcell.php?Weight=");     //YOUR URL
    Serial.println(Weight);
    client.print(Weight);
    
    
    //client.print("&temperature=");
    //Serial.println("&temperature=");
    //client.print(temperatureData);a
    //Serial.println(temperatureData);
    
    client.print(" ");      //SPACE BEFORE HTTP/1.1
    client.print("HTTP/1.1");
    client.println();
    client.println("Host: 192.168.0.144");
    client.println("Connection: close");
    client.println();
  } 
    else 
  {
    // if you didn't get a connection to the server:
    Serial.println("connection failed");
  }
 }