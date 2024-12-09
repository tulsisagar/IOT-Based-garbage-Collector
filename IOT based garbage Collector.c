#include <ESP8266WiFi.h>        // Include ESP8266 WiFi library
#include <BlynkSimpleEsp8266.h> // Include Blynk library
#include <Servo.h>              // Include Servo library

BlynkTimer timer;               // Create a Blynk timer instance
Servo intensity1;               // Create a Servo object for controlling intensity

// Blynk authentication token
char auth[] = "wVLV66Z01dmfJANaHhSVFfBXHuBe3QXJ";

// WiFi credentials
char ssid[] = "android";
char pass[] = "android121";

// Pin definitions
const int trigP1 = D1;          // Ultrasonic sensor 1 trigger pin
const int echoP1 = D2;          // Ultrasonic sensor 1 echo pin
const int trigP2 = D5;          // Ultrasonic sensor 2 trigger pin
const int echoP2 = D6;          // Ultrasonic sensor 2 echo pin
const int led1_pin = D3;        // Servo pin
const int buzz = D7;            // Buzzer pin
const int AO = A0;              // Analog pin for temperature sensor

// Variables for distance measurement
long duration1, duration2;
int distance1, distance2;

void setup() {
  // Pin mode setup
  pinMode(trigP1, OUTPUT);
  pinMode(echoP1, INPUT);
  pinMode(trigP2, OUTPUT);
  pinMode(echoP2, INPUT);
  pinMode(buzz, OUTPUT);
  pinMode(AO, INPUT);

  // Serial communication setup
  Serial.begin(9600);

  // Blynk initialization
  Blynk.begin(auth, ssid, pass);

  // Servo initialization
  intensity1.attach(led1_pin);

  // Timer intervals
  timer.setInterval(1000L, sensor_read1);
  timer.setInterval(1000L, sensor_read2);
  timer.setInterval(1000L, temp_sensor);
}

// Function to activate buzzer
void buzzzer() {
  digitalWrite(buzz, HIGH); // Turn buzzer on
  delay(2000);
  digitalWrite(buzz, LOW);  // Turn buzzer off
}

// Function to read temperature sensor and send data to Blynk
void temp_sensor() {
  float analogValue = 0.0039 * analogRead(AO); // Convert analog reading to voltage
  float temp = analogValue * 100;             // Convert voltage to temperature

  Serial.println(temp - 10);                  // Print temperature for debugging
  Blynk.virtualWrite(V1, temp - 10);          // Send temperature data to Blynk
}

// Blynk virtual write handler for intensity control
BLYNK_WRITE(V0) {
  intensity1.write(param.asInt());
  Serial.println(param.asInt());
}

// Function to read distance from ultrasonic sensor 1
void sensor_read1() {
  digitalWrite(trigP1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigP1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigP1, LOW);

  duration1 = pulseIn(echoP1, HIGH);          // Measure pulse duration
  distance1 = duration1 * 0.034 / 2;         // Calculate distance in cm

  Serial.print("Distance1 = ");
  Serial.println(distance1);

  if (distance1 >= 0 && distance1 <= 10) {
    Serial.println("Someone is near sensor 1");
    intensity1.write(180);
    delay(3000);
    for (int i = 180; i >= 0; i -= 30) {
      intensity1.write(i);
      delay(70);
    }
    Blynk.notify("Dustbin Full at Sensor 1");
  }

  Blynk.virtualWrite(V2, distance1);          // Send distance data to Blynk
}

// Function to read distance from ultrasonic sensor 2
void sensor_read2() {
  digitalWrite(trigP2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigP2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigP2, LOW);

  duration2 = pulseIn(echoP2, HIGH);          // Measure pulse duration
  distance2 = duration2 * 0.034 / 2;         // Calculate distance in cm

  Serial.print("Distance2 = ");
  Serial.println(distance2);

  int distance = map(distance2, 0, 27, 100, 0); // Map distance to a percentage
  Blynk.virtualWrite(V3, distance);             // Send distance data to Blynk

  if (distance2 >= 0 && distance2 <= 10) {
    Serial.println("Dustbin Full at Sensor 2");
    buzzzer();
    Blynk.notify("Dustbin Full at Sensor 2");
  }
}

// Main loop
void loop() {
  Blynk.run();      // Run Blynk tasks
  timer.run();      // Run timer tasks
}
