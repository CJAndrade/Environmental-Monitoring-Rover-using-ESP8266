//Carmelito 10/02/2017 Created for Environmental Monitoring Rover project using Sparkfun ESP8266 Thing Dev Board
//and uploading data to the Cayenne IoT dashboard.
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "DHT.h"
//Based on the testing sketch for various DHT humidity/temperature sensors
//Written by ladyada, public domain from - https://github.com/adafruit/DHT-sensor-library
//#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP8266.h>
//Based on  Cayenne using an ESP8266 https://github.com/myDevicesIoT/Cayenne-MQTT-ESP8266/blob/master/examples/ESP8266/ESP8266.ino

// WiFi Definitions
const char WiFiSSID[] = "Your_WiFi_Router_name";
const char WiFiPSK[] = "Your_WiFi_Router_password";

// Pin Definitions
const int LED_PIN = 5; // Thing's onboard, green LED
const int ANALOG_PIN = A0; // The only analog pin on the Thing
const int DIGITAL_PIN = 12; // Digital pin to be read
#define DHTPIN 2     // what digital pin we're connected to
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

//Motor Driver Pin Definitions
int STBY = 12; //standby

//Motor A -left
int PWMA = 13; //PWM pin for Speed control
int AIN1 = 0; //Direction
int AIN2 = 4; //Direction

//Motor B - right
int PWMB = 15; //PWM pin for Speed control
int BIN1 = 14; //Direction
int BIN2 = 5; //Direction


//Webserver Definitions
WiFiServer server(80);
String readString;

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char username[] = "Cayenne_MQTT_USERNAME";
char password[] = "Cayenne_MQTT_PASSWORD";
char clientID[] = "Cayenne_CLIENT_ID";
unsigned long lastMillis = 0;

DHT dht(DHTPIN, DHTTYPE);


void setup()
{
  initHardware();
  connectWiFi();
  server.begin();
  setupMDNS();
  dht.begin();
  //Cayenne.begin(username, password, clientID, WiFiSSID, WiFiPSK);
  Cayenne.begin(username, password, clientID); //since we are connected to the wifi,do not pass WiFi router definitions
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(STBY, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
}

void loop()
{
  // Check if a client has connected
  WiFiClient client = server.available();
  if (client)
  {
   while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //read  one by one char HTTP request
        if (readString.length() < 100) {
          //storing characters to string
          readString += c;
         }
         //if HTTP request has ended
         if (c == '\n') {
           //Serial.println(readString);//comment out for deployment of the project
           client.println("HTTP/1.1 200 OK"); //send new page
           client.println("Content-Type: text/html");
           client.println("Connection: close");
           client.println();
           client.println("<!DOCTYPE HTML>");
           client.println("<html>");
           client.println("<head>");
           client.println("<title>Thing Dev Rover Controller</title>");
           client.println("<link href='https://rawgit.com/CJAndrade/Environmental-Monitoring-Rover-using-ESP8266/master/robostyle.css' rel='stylesheet' type='text/css'>");
           client.println("</head>");
           client.println("<body>");
           client.println("<h1>Thing Dev Rover Controller</h1>");
           client.println("<br/><table><tr><td></td>");
           client.println("<td><a href=\"/?moveFoward\"\"><button>Foward</button></a></td>");
           client.println("<td></td></tr><tr>");
           client.println("<td><a href=\"/?left\"\"><button>Left</button></a></td>");
           client.println("<td><a href=\"/?back\"\"><button> &nbsp;Back&nbsp;</button></a></td>");
           client.println("<td><a href=\"/?right\"\"><button>Right</button></a></td>");
           client.println("</tr><tr><td></td><td>");
           client.println("<a href=\"/?stopAll\"\"><button>&nbsp;stop&nbsp;</button></a>");
           client.println("</td><td></td></tr><tr>");
           client.println("<td></td><td></td>"); //<a href=\"/?picture\"\"><button>Picture</button></a>
           client.println("<td><a href=\"/?cayenne\"\"><button>Cayenne</button></a></td>");
           client.println("</tr></table>");
           client.println("<br/>");
           client.println("<iframe height='300px' width='100%' src='https://cayenne.mydevices.com/shared/XXXXXXXXXXXXXXXXXXXXX'></iframe>");
           client.println("</body>");
           client.println("</html>");

           delay(1);
           //stopping client
           client.stop();

           //perform a action when the button on the webpage is clicked
           if (readString.indexOf("?left") >0){
            stop();
            delayMicroseconds(4);
            turn(200,'L');
            //Serial.println("left");
           }
           if (readString.indexOf("?back") >0){
            stop();
            delayMicroseconds(4);
            move(220,'B');
            //Serial.println("back");
           }
           if (readString.indexOf("?right") >0){
            stop();
            delayMicroseconds(4);
            turn(210,'R');
            //Serial.println("right");
           }
           if (readString.indexOf("?moveFoward") >0){
            stop();
            delayMicroseconds(4);
            move(220,'F');
            //Serial.println("moveFoward");
           }
           if (readString.indexOf("?stopAll") >0){
              stop();
           //Serial.println("stopAll");
           }
           if (readString.indexOf("?picture") >0){
            //Serial.println("Take a Picture");
           }
           if (readString.indexOf("?cayenne") >0){
            //Serial.println("Posting to Cayenne");
            stop();
            delayMicroseconds(4);
            uploadCayenne();
            //Serial.println("Data Posted...");
           }
           //clearing string for next read
           readString="";
         }
      }
   }
  }
}


void connectWiFi()
{
  byte ledStatus = LOW;
  //Serial.println();
  //Serial.println("Connecting to: " + String(WiFiSSID));
  // Set WiFi mode to station (as opposed to AP or AP_STA)
  WiFi.mode(WIFI_STA);

  // WiFI.begin([ssid], [passkey]) initiates a WiFI connection
  // to the stated [ssid], using the [passkey] as a WPA, WPA2,
  // or WEP passphrase.
  WiFi.begin(WiFiSSID, WiFiPSK);

  // Use the WiFi.status() function to check if the ESP8266
  // is connected to a WiFi network.
  while (WiFi.status() != WL_CONNECTED)
  {
    // Blink the LED
    digitalWrite(LED_PIN, ledStatus); // Write LED high/low
    ledStatus = (ledStatus == HIGH) ? LOW : HIGH;

    // Delays allow the ESP8266 to perform critical tasks
    // defined outside of the sketch. These tasks include
    // setting up, and maintaining, a WiFi connection.
    delay(100);
    // Potentially infinite loops are generally dangerous.
    // Add delays -- allowing the processor to perform other
    // tasks -- wherever possible.
  }
  //Serial.println("WiFi connected");
  //Serial.println("IP address: ");
  //Serial.println(WiFi.localIP());
}

void setupMDNS()
{
  // Call MDNS.begin(<domain>) to set up mDNS to point to
  // "<domain>.local"
  if (!MDNS.begin("thing"))
  {
    //Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);
    }
  }
  //Serial.println("mDNS responder started");

}

void initHardware()
{
  //Serial.begin(9600);
  pinMode(DIGITAL_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  // Don't need to set ANALOG_PIN as input,
  // that's all it can be.
}
void uploadCayenne() {
  float sensor_volt;
  float sensorValue;
  // Wait a few seconds between measurements.
  //delay(5000);
  sensorValue = analogRead(A0);
  sensor_volt = sensorValue/1024*5.0;

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    //Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  /*Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.print(" *F");
  Serial.print("sensor_volt = ");
  Serial.print(sensor_volt);
  Serial.println("V");*/
  Cayenne.loop();
    //Publish data every 10 seconds (20000 milliseconds). Change this value to publish at a different interval.
  //if (millis() - lastMillis > 60000) {
    //lastMillis = millis();
    //Write data to Cayenne here. This example just sends the current uptime in milliseconds.
    //Cayenne.virtualWrite(0, lastMillis);
    //Some examples of other functions you can use to send data.
    //Cayenne.celsiusWrite(1, 22.0);
    Cayenne.virtualWrite(3, f , TYPE_TEMPERATURE, UNIT_FAHRENHEIT);
    Cayenne.virtualWrite(2, h , TYPE_RELATIVE_HUMIDITY, UNIT_PERCENT);
    Cayenne.virtualWrite(4, sensor_volt);
    //Cayenne.virtualWrite(3, 50, TYPE_PROXIMITY, UNIT_CENTIMETER);
    //Serial.println("Sent to Cayenne");
  //}
}

/*
//Default function for processing actuator commands from the Cayenne Dashboard.
//You can also use functions for specific channels, e.g CAYENNE_IN(1) for channel 1 commands.
//CAYENNE_IN_DEFAULT()
CAYENNE_IN(6)
{
  Serial.println("Testing input from Cayenne");
  CAYENNE_LOG("CAYENNE_IN_DEFAULT(%u) - %s, %s", request.channel, getValue.getId(), getValue.asString());
  //Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}
*/
//Driving the rover functions
void turn(int speed,char side){
  digitalWrite(STBY, HIGH);//enable the standby pin
  if(side == 'L'){ //left turn
    digitalWrite(AIN1, HIGH);
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, speed);
    digitalWrite(BIN1, LOW); //Stop motor B
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, speed);
  }else{ //Right turn
    digitalWrite(AIN1, LOW);//Stop motor A
    digitalWrite(AIN2, LOW);
    analogWrite(PWMA, speed);
    digitalWrite(BIN1, HIGH);
    digitalWrite(BIN2, LOW);
    analogWrite(PWMB, speed);
  }

}

void move(int speed,char moveDirection){
digitalWrite(STBY, HIGH);//enable the standby pin
  if(moveDirection =='B'){
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, HIGH);
      analogWrite(PWMA, speed);
      digitalWrite(BIN1, LOW);
      digitalWrite(BIN2, HIGH);
      analogWrite(PWMB, speed);

  }else{
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
      analogWrite(PWMA, speed);
      digitalWrite(BIN1, HIGH);
      digitalWrite(BIN2, LOW);
      analogWrite(PWMB, speed);
  }
}

void stop(){
//enable standby
  digitalWrite(STBY, LOW);
}
