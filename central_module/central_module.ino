#include <LiquidCrystal_I2C.h>
#include <Wire.h>

#include <Servo.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

// RFID
#define sda D4
#define rst D3
#include <KRrfid.h>

// LASER
#define PIN_LSR_RECV A0
#define LSR_THRESH 50

#define PIN_SRVO D8

const char* ssid = "Millennianno's Ideapad Slim 3";
const char* password = "mille1219";

LiquidCrystal_I2C lcd(0x27, 16, 2);

ESP8266WebServer server(8888);   //instantiate server at port 80 (http port)

bool open_gate[2];

String saved_tag;
int time_in[3];

char* txt;
uint8_t buf[12];
uint8_t buflen;

int hours, minutes, seconds;

int avail = 1;
int total = 1;

int old_D0;
int read_D0;

String page = "";

Servo palang;

float read_laser() {
  static float laser;
  laser = map(analogRead(PIN_LSR_RECV), 0, 1024, 0, 100);
  return laser;
}

void setup() {

  Serial.begin(115200);

  pinMode(D0, INPUT);

  // LCD initialize
  lcd.begin(16,2);
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0,0);
  lcd.print("SILAKAN");
  lcd.setCursor(0,1);
  lcd.print("TAP KARTU");

  // RFID initialize
  rfidBegin();

  palang.attach(PIN_SRVO);
  palang.write(0);

  Serial.begin(115200);
  WiFi.begin(ssid, password); //begin WiFi connection
  Serial.println("");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", [](){
    page = "<h1>Smart Parking Kel 1</h1><h3>Parkir:"+String(avail)+"/"+String(total)+"</h3>";
    server.send(200, "text/html", page);
  });

  server.begin();
     
}

void loop() {

  //Server.println("Tes loop");

  getTAG(); 

  if(TAG!="") {
    Serial.print("TAG: ");
    Serial.println(TAG);
    Serial.print("Recieve laser: ");
    Serial.println(read_laser());
    if(saved_tag != TAG){
      open_gate[0] = true;
      time_in[0] = hours;
      time_in[1] = minutes;
      time_in[3] = seconds;
      saved_tag = TAG;
    }
    else {
      open_gate[1] = true;
      saved_tag = "";
    }
    delay(3000);
    TAG = "";
  }

  // in gate
  if(open_gate[0] && (read_laser() > LSR_THRESH)) {
    palang.write(90);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("SILAKAN MASUK");
    lcd.setCursor(0,1);
    lcd.print("TARIF: 2000/JAM");

    delay(2000);
  }
  else if (open_gate[0] && (read_laser() < LSR_THRESH)) {
    delay(2000);

    palang.write(0);
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("SILAKAN");
    lcd.setCursor(0,1);
    lcd.print("TAP KARTU");
    open_gate[0] = false;

  }

  // out gate
  if(open_gate[1]) {

    palang.write(90);
    
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("DURASI:");
    lcd.setCursor(8,0);
    lcd.print(hours-time_in[0]);
    lcd.setCursor(10,0);
    lcd.print("h");
    lcd.setCursor(12,0);
    lcd.print(minutes-time_in[1]);
    lcd.setCursor(14,0);
    lcd.print("m");
    lcd.setCursor(0,1);
    lcd.print("TARIF:");
    lcd.setCursor(8,0);
    lcd.print(2000*(hours-time_in[0]));

    delay(3000);

    palang.write(0);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("SILAKAN");
    lcd.setCursor(0,1);
    lcd.print("TAP KARTU");
    open_gate[1] = false;
  }

  read_D0 = digitalRead(D0);

  if(old_D0 != read_D0) {
    if(read_D0) {
      avail--;
    }
    else {
      avail++;
    }
  }

  delay(200);

  old_D0 = read_D0;

  server.handleClient();
}
