#include <Arduino.h>

#ifndef BAUD_RATE
#define BAUD_RATE 38400
#endif

#define INIT_TIMEOUT unsigned long _t_start = millis(), _t_tick;
#define RESET_TIMEOUT _t_start=millis()

// cheap timeout detection, when wrap around, the actual timeout may double
#define IS_TIMEOUT(_timeout) \
    (_t_tick=millis(),\
     (_t_tick>=_t_start&&(_t_tick-_t_start)>=_timeout) || \
        (_t_tick<_t_start&&_t_tick>=_timeout))

// accurate timeout detection
#define IS_TIMEOUT2(_timeout) \
    (_t_tick=millis(),\
     (_t_tick>=_t_start&&(_t_tick-_t_start)>=_timeout) || \
        (_t_tick<_t_start&&((0xffffffff-_t_start)+_t_tick)>=_timeout))


// #define DELAY_START 10000

#define ENABLE_SHELL

#ifdef ENABLE_SHELL
#   include <bitlash.h>
#endif

#define noop do{}while(0)

#define STEPPER_DIR 7
#define STEPPER_STEP 8
#define STEPPER_SLP 9

#define SERVO_PIN 10
#include "servo.h"

unsigned stepCount;
unsigned stepDelay;

int st(unsigned dir, unsigned count, unsigned delay) {
    digitalWrite(STEPPER_DIR,dir==1?HIGH:LOW);

    stepDelay = delay;
    stepCount = count;

    printInteger(dir,0,0);
    sp(",");
    printInteger(stepCount,0,0);
    sp(",");
    printInteger(stepDelay,0,0);
    speol();

    digitalWrite(STEPPER_SLP,stepCount?HIGH:LOW);
}

void loopSt() {
    if(!stepCount) return;
    digitalWrite(STEPPER_STEP,HIGH);
    delay(stepDelay);
    digitalWrite(STEPPER_STEP,LOW);
    delay(stepDelay);
    if(--stepCount == 0)
        digitalWrite(STEPPER_SLP,LOW);
}

#ifdef ENABLE_SHELL

numvar stCmd() {
    return st(getarg(1),getarg(2),getarg(3));
}

void setupShell() {
    initBitlash(BAUD_RATE);
	addBitlashFunction("st", (bitlash_function) stCmd);
	addBitlashFunction("sv", (bitlash_function) svCmd);
}

void loopShell() {
    runBitlash();
}
#else
#   define setupShell() Serial.begin(BAUD_RATE)
#   define loopShell() noop
#endif

void setup() {
    pinMode(STEPPER_DIR,OUTPUT);
    pinMode(STEPPER_STEP,OUTPUT);
    pinMode(STEPPER_SLP,OUTPUT);
    digitalWrite(STEPPER_SLP,LOW);
    setupShell();
}

void loop() {
    loopShell();
    loopSt();
    loopServo();
}

