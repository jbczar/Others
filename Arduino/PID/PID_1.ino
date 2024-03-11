#include "TRSensors.h"
 
#define NUM_SENSORS 5
TRSensors trs = TRSensors();
unsigned int sensorValues[NUM_SENSORS];
 
double Kp = 0.05;
double Ki = 0;
double Kd = 0.01;
double error = 0;
double integral = 0;
double derivative = 0;
double lastError = 0;
int targetPosition = 2000;
int speed = 100;
int Lspeed;
int Rspeed;
 
void setup() {
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
 
  for (int i = 0; i < 400; i++) {
    trs.calibrate();
  }
}
 
void loop() {
  trs.readCalibrated(sensorValues);
  error = (int)(trs.readLine(sensorValues)) - targetPosition;
 
  integral += error;
  derivative = error - lastError;
  double pid = Kp * error + Ki * integral + Kd * derivative;
 
  digitalWrite(A1, HIGH);
  digitalWrite(A0, LOW);
  digitalWrite(A2, HIGH);
  digitalWrite(A3, LOW);
 
  Lspeed = constrain(speed - pid, 0, 255);
  Rspeed = constrain(speed + pid, 0, 255);
 
  analogWrite(5, Lspeed);
  analogWrite(6, Rspeed);
 
  lastError = error;
}