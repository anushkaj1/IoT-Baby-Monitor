#include "Servo.h"
#include "HTTPClient.h"
#include <TimeLib.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#define BLYNK_TEMPLATE_ID "TMPLEraxSp-D"
#define BLYNK_DEVICE_NAME "IOT Project"
#define BLYNK_AUTH_TOKEN "1ZzQ7hR1zpBmOzs-15g5X78HzFEVqra8"
#define BLYNK_PRINT Serial

#define PIRPinL 35 // Lower PIR sensor
#define PIRPinH 19 // Higher PIR sensor 
#define LED_PIN 21 // LED pin
#define SERVO_PIN 18
#define TRIGGER 33
#define ECHO 32
#define SOUND_SPEED 0.034 //in cm/micro second
#define safeDistance 20

String cse_ip = "192.168.1.103";
String cse_port = "8080";
String server = "http://" + cse_ip + ":" + cse_port + "/~/in-cse/in-name/";
String ae = "BabyMonitor";
String cnt = "node1";

char *auth = BLYNK_AUTH_TOKEN;
const char* ssid = "AegiFi"; // your network SSID (name)
const char* password = "Akshit@2003"; // your network password
//const char* ssid = "Mmmm"; // your network SSID (name)
//const char* password = "plus8pro"; // your network password

String header; //https request
String outData = "";
String timeToPrint = "";

int pirInpL = 0; // Input from lower PIR sensor
int pirInpH = 0; // Input from higher PIR sensor
int freq_inp = 1000;
int servo_default = 0; // Angle of servo motor in default state
int servo_latch = 120; // Angle of servo motor in latched state
int output = 0;

unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;

Servo servo_motor;
BlynkTimer timer;

void createCI(String val){
  HTTPClient http;
  http.begin(server + ae + "/" + cnt + "/");
  http.addHeader("X-M2M-Origin", "admin:admin");
  http.addHeader("Content-Type", "application/json;ty=4");
  int code = http.POST("{\"m2m:cin\": {\"cnf\":\"application/json\",\"con\": " + val + "}}");
  Serial.println("{\"m2m:cin\": {\"cnf\":\"application/json\",\"con\": " + val + "}}");
  if (code == -1) {
  Serial.println("UNABLE TO CONNECT TO THE SERVER");
  }
  http.end();
}

void setup() {
  Serial.begin(115200);
  Blynk.begin(auth, ssid, password);
  pinMode(LED_PIN, OUTPUT);
  pinMode(PIRPinL, INPUT);
  pinMode(PIRPinH, INPUT);
  pinMode(32, INPUT);
  pinMode(TRIGGER, OUTPUT);
  digitalWrite(TRIGGER, LOW);
  pinMode(ECHO, INPUT);

  digitalWrite(LED_PIN, LOW);

  // Connect to WiFi network
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

    // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  
  servo_motor.attach(SERVO_PIN);
  servo_motor.write(servo_default);
}

double getDistance()
{
  // Clears the trigPin
  digitalWrite(TRIGGER, LOW);
 // delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIGGER, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER, LOW);

  digitalWrite(LED_PIN, LOW);
  
  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(ECHO, HIGH,40000);

  double distanceCm;
  
  // Calculate the distance
  if(duration == 0)
    Serial.println("No obstacle detected");
  else
  {
    distanceCm = duration * SOUND_SPEED/2;

    // Prints the distance in the Serial Monitor
    Serial.print("Distance (cm): ");
    Serial.println(distanceCm);
  }
  
  delay(1000);

  return distanceCm;
}

void loop() {

  Blynk.run();

  pirInpL = digitalRead(PIRPinL);
  pirInpH = digitalRead(PIRPinH);

  Serial.print("Low Input: ");
  Serial.println(pirInpL);

  Serial.print("High Input: ");
  Serial.println(pirInpH);

  // Motion detected at lower level but not at higher.
  // This implies that baby's presence is detected.

  Blynk.virtualWrite(V0, pirInpL);
  Blynk.virtualWrite(V1, pirInpH);

  output = pirInpL && !pirInpH;
  Blynk.virtualWrite(V4, output);

//  createCI(String(getDistance()));
  
  if(pirInpL && !pirInpH) 
  {
    outData = "Motion Detected"; 
     // Rings buzzer
    servo_motor.write(servo_latch); // Rotate servo motor to latch door
    digitalWrite(LED_PIN, HIGH);

    // Note the current time
    timeToPrint = printLocalTime();

//    timeToPrint.replace(" ","%20");

    createCI(timeToPrint.substring(14,16));
    
    // Wait for 2 seconds
    delay(2000);

    // Then, wait for baby to go atleast 3 meters away
    while (getDistance() < safeDistance) 
    {
      Serial.println("baby still in range!");
      delay(2000);  // Wait 5 seconds before checking again
    }

    // Reset servo motor to original angle
    // This unlatches the door
     servo_motor.write(servo_default);
     // Turn off buzzer
     digitalWrite(LED_PIN, LOW);
  }
  
  delay(1000);
}


String printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return "";
  }
  Serial.println(&timeinfo);

  return asctime(&timeinfo);
}