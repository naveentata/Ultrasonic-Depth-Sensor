#include <ESP8266WiFi.h>
#include <CircularBuffer.h>
WiFiClient client;
CircularBuffer<int, 10> buffer;

#include <ESP8266HTTPClient.h>

String thingSpeakAddress= "http://api.thingspeak.com/update?";
String writeAPIKey;
String tsfield1Name;
String request_string;

HTTPClient http;

String MakerIFTTT_Key ;
;String MakerIFTTT_Event;
char *append_str(char *here, String s) {  int i=0; while (*here++ = s[i]){i++;};return here-1;}
char *append_ul(char *here, unsigned long u) { char buf[20]; return append_str(here, ultoa(u, buf, 10));}
char post_rqst[256];char *p;char *content_length_here;char *json_start;int compi;

char* ssid = "ips"; 
char* password = "123456789";

int trigPin = 4;   // Trigger D2
int echoPin = 0;   // Echo D3
int LED = 2;       // LED D4
long duration, depth, level;
long tankHeight = 40, threshold = 25;
int flag = 0;
float avg = 0.0;
int sum = 0;
void setup() {
  // UltraSonic and LED setup
  Serial.begin (9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(LED, OUTPUT);

  // WiFi Setup
   WiFi.disconnect();
   Serial.println("Start");
   delay(3000);
   WiFi.begin(ssid, password);
   while ((!(WiFi.status() == WL_CONNECTED))){
    delay(300);
  }
  Serial.println("Connected");
}
 
void loop() {
  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(100);
  digitalWrite(trigPin, LOW);
  
  //pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH); // duration is the time (in microseconds)
 
  // Convert the time into a distance
  depth = (duration/2) * 0.0343;     // Divide by 29.1 or multiply by 0.0343
  level = tankHeight - depth;

  // Thingspeak
  if (client.connect("api.thingspeak.com",80)) {
      request_string = thingSpeakAddress;
      request_string += "key=";
      request_string += "N777UBX1A7R13CS1";
      request_string += "&";
      request_string += "field1";
      request_string += "=";
      request_string += String(level);
      http.begin(request_string);
      http.GET();
      http.end();

    }

  
  if(level <= threshold && flag != 1)
  {
    Serial.print("Level of water in tank is low; Water level left in tank = ");
    Serial.print(level);
    Serial.println(" cm");
    if(!buffer.isFull())
    {
      buffer.push(level);
    }
    else
    {
      flag = 1;
    }

    //Toggle LED
    digitalWrite(LED, LOW);      // Turn the LED on (Note that LOW is the voltage level) 
    delay(1000);                // Wait for a second
    digitalWrite(LED, HIGH);   // Turn the LED off by making the voltage HIGH
    delay(1000);
     
  }
  else if(flag == 1)
  {
    //using index_t = decltype(buffer)::index_t;
    for(int k = 0;k<buffer.size();k++)
    {
      sum += buffer.shift();
    }
    avg = sum / buffer.size();
     if (client.connect("maker.ifttt.com",80)) {
    MakerIFTTT_Key ="nwwSl0jKutX5PneM3jgs9NRPORXehBHqiiN3VHJvEao";
    MakerIFTTT_Event ="notify";
      Serial.println("Mail");
      Serial.println(buffer.first());
    p = post_rqst; 
    p = append_str(p, "POST /trigger/");
    p = append_str(p, MakerIFTTT_Event);
    p = append_str(p, "/with/key/");
    p = append_str(p, MakerIFTTT_Key);
    p = append_str(p, " HTTP/1.1\r\n");
    p = append_str(p, "Host: maker.ifttt.com\r\n");
    p = append_str(p, "Content-Type: application/json\r\n");
    p = append_str(p, "Content-Length: ");
    content_length_here = p;
    p = append_str(p, "NN\r\n");
    p = append_str(p, "\r\n");
    json_start = p;
    p = append_str(p, "{\"value1\":\"");
    p = append_str(p, "PPC Roof Top");
    p = append_str(p, "\",\"value2\":\"");
    p = append_str(p, "User");
    p = append_str(p, "\",\"value3\":\"");
    p = append_str(p, String(level)); //circular buffer avg
    p = append_str(p, "\"}");

    compi= strlen(json_start);
    content_length_here[0] = '0' + (compi/10);
    content_length_here[1] = '0' + (compi%10);
    client.print(post_rqst);
    flag = 0;
  }  
   
  }
  else
  {
    Serial.print(level);
    Serial.println(" cm");
    digitalWrite(LED, HIGH);
  }
  delay(1000);
}
