/* 
 * Project Midterm2 - Smart Plant Watering System (miN)
 * Author: Sterling Powers
 * Date: 03_19_25
 */


 #include "Particle.h"
 #include "Adafruit_GFX.h"
 #include "Adafruit_SSD1306.h"
 #include "Adafruit_BME280.h"
 #include <Adafruit_MQTT.h>
 #include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h"
 #include "Adafruit_MQTT/Adafruit_MQTT.h"
 #include "credentials.h"

//Global State
TCPClient TheClient; 

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details. 
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY); 

// Setup Feeds to publish or subscribe 
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname> 
Adafruit_MQTT_Subscribe subFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/buttonOnOff"); 
Adafruit_MQTT_Publish pubFeed1 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/moisturelevel");
Adafruit_MQTT_Publish pubFeed2 = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/smartPlant");
 
 //OLED & Bme
 Adafruit_SSD1306 display(-1);
 Adafruit_BME280 bme;
 
 //BME
 float temperature, humidty, pressure;

 //Moisture sensor & OLED
 String dateTime, timeOnly;
 const int DIRTSPIKE = A0;
 const int PUMP = D16;
 int moist, lastTime;

 unsigned long pumpRunTime = 500;  
 unsigned long waitTime = 60000;  
 unsigned long lastPumpTime = 0;  
 unsigned long lastMoistureCheckTime = 0; 


//MQTT Functions
 void MQTT_connect();
 bool MQTT_ping();
 float subValue,pubValue;

SYSTEM_MODE(AUTOMATIC);

SYSTEM_THREAD(ENABLED);


void setup() {
  Serial.begin (9600);

    // // Connect to Internet but not Particle Cloud
    // WiFi.on();
    // WiFi.connect();
    // while(WiFi.connecting()) {
    //   Serial.printf(".");
    // }

  // Setup MQTT subscription
  mqtt.subscribe(&subFeed);

//Start OLED
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  delay(2000);

  //Start BME
  bme.begin (0x76);

//Time Requires Particle Cloud
  Time.zone (-6);
  Particle.syncTime ();
  
pinMode (DIRTSPIKE,INPUT);
pinMode (PUMP, OUTPUT);
}

void loop() {
  //digitalWrite (PUMP, LOW);


//collect data and send to OLED
    dateTime = Time.timeStr();
    timeOnly = dateTime.substring (11,19);

    float tempC=bme.readTemperature();
    float tempF=(tempC *9.0/5.0)+32.0;

    float hum = bme.readHumidity();

    moist = analogRead(DIRTSPIKE);

    display.clearDisplay();
    display.setCursor(0,0);
    display.printf("Time: %s\n",timeOnly.c_str());
    display.printf("Temp: %.2fF\n", tempF);
    display.printf("Humidity: %.2f %%\n", hum);
    display.printf("Moisture = %i\n",moist);
    display.display();

  if (millis() - lastMoistureCheckTime > waitTime) {
    lastMoistureCheckTime = millis();
    if (moist>3300){
      if (millis() - lastPumpTime > pumpRunTime) {  
      digitalWrite(PUMP, HIGH); 
      lastPumpTime = millis();  
      }
    }
  }
  if (millis() - lastPumpTime >= pumpRunTime && digitalRead(PUMP) == HIGH) {
    digitalWrite(PUMP, LOW); 
    }

  if (millis()-lastTime>10000){
    lastTime=millis();

      if(mqtt.Update()) {
      pubValue=analogRead (DIRTSPIKE);
      pubFeed1.publish(pubValue);

  String tempStr = "Temp: " + String(tempF, 2) + "F";
  pubFeed2.publish(tempStr.c_str());

  String humStr = "Humidity: " + String(hum,2);
  pubFeed2.publish(humStr.c_str());
  } 
}
}
  