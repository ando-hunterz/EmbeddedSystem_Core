/**
*   HC-SR04 Library By Julando Omar
*
**/


#include <Arduino.h>
#include "HC_SR04.h"

HC_SR04::HC_SR04(int trig, int echo){
    this->trig = trig;
	this->echo = echo;
    pinMode(this->trig, OUTPUT); // Sets the trigPin as an OUTPUT
    pinMode(this->echo, INPUT); // Sets the echoPin as an INPUT
}

void HC_SR04::setMinMotionDistance(int minMotionDistance){
    this->minMotionDistance = minMotionDistance;
}

bool HC_SR04::performSelfTest(){
  int distance;
  long duration;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2; 
  if(distance == 0){
    return false;  
  }
  return true;
}

int HC_SR04::calculateDistance(){
  int distance;
  long duration;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2; 
  return distance;
}

int HC_SR04::isMotionPresent(){
  int distance;
  long duration;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2; 
  if(distance <= this->minMotionDistance){
      return false;
  }
  return true;
}