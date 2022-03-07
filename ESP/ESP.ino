// Include the libraries
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include <DallasTemperature.h>
#include <OneWire.h>


byte counter =0;

char* serial_tochar(int choose_data); 
char* ssid;
char* password;
char* data[]={"user","password"};

// define bins for sensors
DHT dht2(2,DHT11); // D4,dht sensor
//DS18B20 sensor
OneWire oneWire(0);  //D3
DallasTemperature DS18B20(&oneWire);

void connect_to_WiFi();
void postreadings (String reading1,String reading2);

void setup()
{
  Serial.begin(9600);
  Serial.println();
  connect_to_WiFi();
}

 
void loop() {
 
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

    // DS18B20 sensor for reading temperature
    DS18B20.requestTemperatures();       //Prepare DS18B20 for reading
    float temp = DS18B20.getTempCByIndex(0);
    String temperature = String(temp) + String("°C");    //Temperature integer to string conversion
    Serial.println("Temperature:");
    Serial.println( temperature);
    
   // dht sensor for reading humidity
    float Temp = dht2.readTemperature( );
    float Hum = dht2.readHumidity();  
    if (isnan(Temp) || isnan(Hum))     // Checking dht sensor working
  {                                   
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
    String humidity = String(Hum) + String("%");      //Humidity integer to string conversion
    Serial.println("Humidity:");
    Serial.println(humidity);
    delay(3000);  
    postreadings ( temperature, humidity);
  }  
  else {
    Serial.println("Error in WiFi connection");
  }
 
  delay(10000);  //Send a request every 30 seconds
}

//function for scanning networks and conncting to one of them
void connect_to_WiFi() {
  
  WiFi.disconnect();
  Serial.println();
  Serial.println("start scanning:-");
  int numberOfNetworks = WiFi.scanNetworks();
 
   for(int i =0; i<numberOfNetworks; i++){
 
      Serial.print("Network name: ");
      Serial.println(WiFi.SSID(i));
      Serial.print("Signal strength: ");
      Serial.println(WiFi.RSSI(i));
      Serial.println("-----------------------");
   }

   ssid = strtok(getname(0), " ");
   password = strtok(getname(1), " ");
   
  Serial.println("connecting");
  WiFi.begin(ssid,password);
  
  while ((!(WiFi.status() == WL_CONNECTED))){
    delay(500);
    Serial.print("..");
    counter++;
    if (counter==20) {
    // break after 10 sec
    break;
    }
  }
  switch (WiFi.status()) {
    
      case WL_CONNECTED:
            Serial.println();
            Serial.print("connected to:");
            Serial.println(ssid);
            Serial.println("IP");
            Serial.println(WiFi.localIP());
            break;

       // don't forget to check................
      case WL_CONNECT_FAILED:
            Serial.println("Wrong Password:(");
            break;

      default:
           Serial.println("Unknown error!");
           delay(3000);
           ESP.restart();
  } 
}

char *getname(int choose_data)
{
  while (Serial.available() == 0)
  {
  }
  String str = Serial.readString();
  str.toCharArray(data[choose_data], str.length());
  return data[choose_data];
}

// function to post values to server
void postreadings (String reading1,String reading2)
{
  // Set up the client objet
     WiFiClient client;
     HTTPClient http;
    http.begin(client,"http://toto3.pythonanywhere.com/test/");   //Specify request destination
    http.addHeader("Content-Type",  "application/json");  //Specify content-type header
  
   StaticJsonBuffer<200> jsonBuffer;
   JsonObject& values = jsonBuffer.createObject();
   values["Temperature"] = reading1;
   values["Humidity"] = reading2;
   
    char json_str[100]; 
    values.prettyPrintTo(json_str, sizeof(json_str));
    int httpCode = http.POST(json_str);       //Send the request
    
    if(httpCode > 0){
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.print("Response: ");Serial.println(payload);
        }
    }
    else{
         Serial.printf("[HTTP] GET... failed, error: %s", http.errorToString(httpCode).c_str());
    }
        http.end();  //Close connection
    
    }
