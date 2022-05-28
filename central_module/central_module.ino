#include <WiFi.h>
#include <Servo.h>
#include <RH_ASK.h>
#include <SPI.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

char* ssid = "Millennianno's Ideapad Slim 3";  
char* password = "mille1219";

// for IoT feature
WiFiServer server(8888);
WiFiClient client;

String header;

RH_ASK driver; // for RF reciever
Servo servo_parking; // for servo

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

int val;
int potpin = 0;

void setup() {
  Serial.begin(115200);
  SPI.begin();      // Initiate  SPI bus

  servo_parking.attach(0);

  lcd.init();
  lcd.backlight();
  
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

  mfrc522.PCD_Init();   // Initiate MFRC522
  server.begin();
}

void loop() {
  client = server.available();   // Listen for incoming clients
  
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    
    while (client.connected()) {            // loop while the client's connected
      
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            client.println("<!DOCTYPE html>\n");
            client.println("<html>\n");
            client.println("<body>\n");
            client.println("<center>\n");
            client.println("<h1 style=\"color:blue;\">ESP32 webserver</h1>\n");
            client.println("<h2 style=\"color:green;\">Hello World Web Sever</h2>\n");
            client.println("<h2 style=\"color:blue;\">Password protected Web server</h2>\n");
            client.println("</center>\n");
            client.println("</body>\n");
            client.println("</html>");
            break;            
          } 
          else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } 
        else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
  }

  // Clear the header variable
  header = "";
  // Close the connection
  client.stop();
  Serial.println("Client disconnected.");
  Serial.println("");


  // recieving RF message
  uint8_t buf[12];
  uint8_t buflen = sizeof(buf);
  if (driver.recv(buf, &buflen)) {
    int i;
    // Message with a good checksum received, dump it.
    Serial.print("Message: ");
    Serial.println((char*)buf);         
  }

  // print LCD
  lcd.setCursor(0,0);
  lcd.print("hello everyone");
  lcd.setCursor(0,1);
  lcd.print("konichiwaa");

  // servo
  val = analogRead(potpin);            // reads the value of the potentiometer (value between 0 and 1023)
  val = map(val, 0, 1023, 0, 180);     // scale it to use it with the servo (value between 0 and 180)
  servo_parking.write(val);                  // sets the servo position according to the scaled value

  // rfid reader
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  // Show UID on serial monitor
  Serial.print("UID tag :");
  String content= "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  if (content.substring(1) == "BD 31 15 2B") {
    Serial.println("Authorized access");
    Serial.println();
    delay(3000);
  }
 
 else {
    Serial.println(" Access denied");
    delay(3000);
  }
  
}
