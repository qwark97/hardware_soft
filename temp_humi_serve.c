/* 
* ESP8266 NodeMCU DHT11 - Humidity Temperature Sensor Example
* https://circuits4you.com
* 
* References
* https://circuits4you.com/2017/12/31/nodemcu-pinout/
* 
*/

/*
 * Rui Santos
 * Complete project details at https://randomnerdtutorials.com  
*/

#include "DHTesp.h"

// Load Wi-Fi library
#include <ESP8266WiFi.h>

#define DHTpin 14    //D5 of NodeMCU is GPIO14
DHTesp dht;

// Replace with your network credentials
const char* ssid     = "<network ssid>";
const char* password = "<network pass>";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;
// constants won't change. Used here to set a pin number:
const int ledPin =  LED_BUILTIN;// the number of the LED pin

// Variables will change:
int ledState = LOW;             // ledState used to set the LED

void setup()
{
  Serial.begin(115200);

  // Connect to Wi-Fi network with SSID and password
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
  server.begin();
  
  dht.setup(DHTpin, DHTesp::DHT11); //for DHT11 Connect DHT sensor to GPIO 17
  pinMode(ledPin, OUTPUT);
}

float humidity = dht.getHumidity();
float temperature = dht.getTemperature();
long nextTempCheck = millis() + dht.getMinimumSamplingPeriod();

void loop()
{
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    digitalWrite(ledPin, LOW);
    currentTime = millis();
    if (currentTime > nextTempCheck) {
        humidity = dht.getHumidity();
        temperature = dht.getTemperature();
        nextTempCheck = millis() + dht.getMinimumSamplingPeriod();
    }
    
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println(); 

            if (header.indexOf("GET /temp") >= 0) {
              Serial.println("Temperature check");
              client.print(temperature);
            } else if (header.indexOf("GET /humidity") >= 0) {
              Serial.println("Humidity check");
              client.print(humidity);
            } else
            {
              Serial.println("Return both");
              client.print(temperature);
              client.println();
              client.print(humidity);
            }
            
           } else { // if you got a newline, then clear currentLine
            currentLine = "";
           }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    digitalWrite(ledPin, HIGH);
    Serial.println("");
  }
}
