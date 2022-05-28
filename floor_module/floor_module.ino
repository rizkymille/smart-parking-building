#include <Servo.h>

#define pin_read_IR A0
#define pin_trig_us 9
#define pin_echo_us 10
#define servo_pin 6

Servo servo_palang;  // create servo object to control a servo

int reading;
// defines variables
long duration;
int distance;
int pos = 0;

void us_init() {
  pinMode(pin_trig_us, OUTPUT); // Sets the pin_trig_us as an Output
  pinMode(pin_echo_us, INPUT); // Sets the pin_echo_us as an Input
}

float us_read() {
  digitalWrite(pin_trig_us, LOW);
  delay(10);
  // Sets the pin_trig_us on HIGH state for 10 micro seconds
  digitalWrite(pin_trig_us, HIGH);
  digitalWrite(pin_trig_us, LOW);
  // Reads the pin_echo_us, returns the sound wave travel time in microseconds
  duration = pulseIn(pin_echo_us, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2;
  return distance;
}

void setup() {
  us_init();

  servo_palang.attach(servo_pin);
  
  Serial.begin(9600); // Starts the serial communication
}

void loop() {
  distance = us_read();
  Serial.print("Distance: ");
  Serial.println(distance);
  Serial.print(" cm");

  reading = analogRead(pin_read_IR);
  Serial.println(reading);

  if ((distance < 100) && (reading < 255)) {
    servo_palang.write(90);
  }
  else {
    servo_palang.write(0);
  }
  delay(500);
}
