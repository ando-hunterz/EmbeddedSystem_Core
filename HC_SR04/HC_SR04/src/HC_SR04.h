/**
*   HC-SR04 Library By Julando Omar
*
**/
#ifndef HC_SR04_h
#define HC_SR04_h

#include <Arduino.h>

class HC_SR04{
    public: 
    HC_SR04(int trig, int echo);
    void setMinMotionDistance(int minMotionDistance);
    bool performSelfTest();
    int calculateDistance();
    bool isMotionPresent();

    private:
    int trig;
    int echo;
    int minMotionDistance;

};

#endif
