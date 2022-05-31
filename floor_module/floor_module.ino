/*
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
*/

// == PINS == //
#define PIN_IR A0

#define PIN_TRIG_US 9
#define PIN_ECHO_US 10

#define PIN_BUZ 6

// == THRESHOLDS == //
#define DIST_THRES 15
#define MIN_DIST_THRES 4

#define LIGHT_THRES 500

//RH_ASK driver(2000, 11, 12, 11);

int reading;
// defines variables
//long duration;
int distance;
int freq;

bool old_lock_cond = false;
bool lock_cond = false;

// == FILTER == //
float kalman(double U){
  static const float R = 40;
  static const float H = 1.00;
  static float Q = 10;
  static float P = 0;
  static float U_hat = 0;
  static float K = 0;
  K = P*H/(H*P*H+R);
  U_hat += + K*(U-H*U_hat);
  P = (1-K*H)*P+Q;
  return U_hat;
}

void us_init() {
  pinMode(PIN_TRIG_US, OUTPUT); // Sets the PIN_TRIG_US as an Output
  pinMode(PIN_ECHO_US, INPUT); // Sets the PIN_ECHO_US as an Input
}

void us_read() {
  static long duration;
  digitalWrite(PIN_TRIG_US, LOW);
  delay(10);
  // Sets the PIN_TRIG_US on HIGH state for 10 micro seconds
  digitalWrite(PIN_TRIG_US, HIGH);
  digitalWrite(PIN_TRIG_US, LOW);
  // Reads the PIN_ECHO_US, returns the sound wave travel time in microseconds
  duration = pulseIn(PIN_ECHO_US, HIGH);
  // Calculating the distance
  distance = kalman(duration * 0.034 / 2);
}

void ir_read() {
  reading = analogRead(PIN_IR);
}

void setup() {
  us_init();
  noTone(PIN_BUZ);
  Serial.begin(9600); // Starts the serial communication

  pinMode(5, OUTPUT);

  //driver.init();

}

void loop() {
  us_read();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  ir_read();
  Serial.print("Light: ");
  Serial.println(reading);


  if ((distance < DIST_THRES) && (distance > MIN_DIST_THRES)) {
    freq = map(freq, DIST_THRES, MIN_DIST_THRES, 0.1, 1);
    tone(PIN_BUZ, freq);
  }
  else {
    noTone(PIN_BUZ);
  }

  if ((distance < MIN_DIST_THRES) && (reading < LIGHT_THRES)) {
    noTone(PIN_BUZ);
    lock_cond = true;
  }
  else if ((distance < DIST_THRES) && (reading < LIGHT_THRES)) {
    lock_cond = false;
  }

  if (old_lock_cond && !lock_cond) {
    digitalWrite(5, LOW);
    //radio_send("park-");
    Serial.println("Park open");
  }
  else if (!old_lock_cond && lock_cond) {
    digitalWrite(5, HIGH);
    //radio_send("park+");
    Serial.println("Park closed");
  }

  old_lock_cond = lock_cond;

  delay(300);
}

/*
void radio_send(const char *msg) {
    driver.send((uint8_t *)msg, strlen(msg));
    driver.waitPacketSent();
}
*/