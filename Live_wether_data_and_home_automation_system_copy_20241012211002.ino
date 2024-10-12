#include <ESP8266WiFi.h>
#include <string.h>
#include "DHT.h"

#define DHTPIN D4
#define DHTTYPE DHT11  
DHT dht(DHTPIN, DHTTYPE);

const char* ssid = "Naveen";
const char* password = "12345678";

int l1 = D5; 
int l2 = D6; 
int l3 = D7; 
int l4 = D8;

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(l1, OUTPUT);
  pinMode(l2, OUTPUT);
  pinMode(l3, OUTPUT);
  pinMode(l4, OUTPUT);

  digitalWrite(l1, LOW);
  digitalWrite(l2, LOW);
  digitalWrite(l3, LOW);
  digitalWrite(l4, LOW);

  // Connect to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi connected");
  server.begin();
  Serial.println("Server started");
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  
  dht.begin();
}

void loop() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" % - Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  delay(1000);

  WiFiClient client = server.available();
  if (!client) return;

  Serial.println("New client");
  while(!client.available()) delay(1);

  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();

  // Control lights based on requests
  if (request.indexOf("/LED1=ON") != -1) digitalWrite(l1, HIGH);
  if (request.indexOf("/LED1=OFF") != -1) digitalWrite(l1, LOW);
  if (request.indexOf("/LED2=ON") != -1) digitalWrite(l2, HIGH);
  if (request.indexOf("/LED2=OFF") != -1) digitalWrite(l2, LOW);
  if (request.indexOf("/LED3=ON") != -1) digitalWrite(l3, HIGH);
  if (request.indexOf("/LED3=OFF") != -1) digitalWrite(l3, LOW);
  if (request.indexOf("/LED4=ON") != -1) digitalWrite(l4, HIGH);
  if (request.indexOf("/LED4=OFF") != -1) digitalWrite(l4, LOW);

  // Serve sensor data as JSON
  if (request.indexOf("/getData") != -1) {
    String json = "{\"temperature\": " + String(temperature) + ", \"humidity\": " + String(humidity) + "}";
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println();
    client.println(json);
    return;
  }

  // Serve HTML page
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println();
  client.println("<!DOCTYPE html>");
  client.println("<html>");
  client.println("<head>");
  client.println("<title>ESP IoT Dashboard</title>");
  client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<style>");
  client.println("body {font-family: Arial; text-align: center; background-color: #0A1128; color: #fff;}");
  client.println(".content {padding: 20px;}");
  client.println(".card {background-color: white; margin: 20px; padding: 20px; border-radius: 10px;}");
  client.println(".button {margin: 10px;}");
  client.println("</style>");
  client.println("</head>");
  client.println("<body>");
  client.println("<h1>Home Automation System</h1>");
  client.println("<div class='card'>");
  client.println("<h2>Temperature: <span id='temperature'></span> &deg;C</h2>");
  client.println("<h2>Humidity: <span id='humidity'></span> %</h2>");
  client.println("</div>");
  client.println("<div class='content'>");
  client.println("<button class='button' onclick=\"location.href='/LED1=ON'\">Light 1 ON</button>");
  client.println("<button class='button' onclick=\"location.href='/LED1=OFF'\">Light 1 OFF</button>");
  client.println("<button class='button' onclick=\"location.href='/LED2=ON'\">Light 2 ON</button>");
  client.println("<button class='button' onclick=\"location.href='/LED2=OFF'\">Light 2 OFF</button>");
  client.println("<button class='button' onclick=\"location.href='/LED3=ON'\">Light 3 ON</button>");
  client.println("<button class='button' onclick=\"location.href='/LED3=OFF'\">Light 3 OFF</button>");
  client.println("<button class='button' onclick=\"location.href='/LED4=ON'\">Light 4 ON</button>");
  client.println("<button class='button' onclick=\"location.href='/LED4=OFF'\">Light 4 OFF</button>");
  client.println("</div>");
  client.println("<script>");
  client.println("setInterval(function() {");
  client.println("fetch('/getData').then(response => response.json()).then(data => {");
  client.println("document.getElementById('temperature').textContent = data.temperature;");
  client.println("document.getElementById('humidity').textContent = data.humidity;");
  client.println("});");
  client.println("}, 1000);");
  client.println("</script>");
  client.println("</body>");
  client.println("</html>");
  client.println();

  delay(1);
  Serial.println("Client disconnected");
}
