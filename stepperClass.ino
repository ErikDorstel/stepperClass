#include "stepperClass.h"

stepperClass myStepper;

void myStepperLoop() {
  if (myStepper.input(32) && myStepper.ifPos(0)==false) {
    myStepper.stop();
    myStepper.setZero();
    myStepper.storePos(0);
  }
  if (myStepper.input(33) && myStepper.ifPos(0)==true) {
    myStepper.turn(10);   // +10 steps
  }
  if (myStepper.input(25) && myStepper.ifPos(0)==true) {
    myStepper.go(-0.5);   // -0.5 mm
  }
  if (myStepper.input(26) && myStepper.ifPos(0)==true) {
    myStepper.turnAbs(90,360);
    myStepper.wait();
    myStepper.delay(1000);
    myStepper.goAbs(3.75);
  }
  if (myStepper.input(27) && myStepper.ifPos(0)==true) {
    myStepper.turnPos(0);
  }
  if (myStepper.busy==false && myStepper.ifPos(0)==false) {
    myStepper.turn(myStepper.steps);
  }
}

void setup() {
  myStepper.begin(16,15,19,18,17,23);
  myStepper.setInputs(5,25,26,27,32,33);
  myStepper.setLoop(myStepperLoop);
  myStepper.setSpeed(20,60); // Umdrehungen pro Minute
  myStepper.setSteps(200);   // Schritte pro Umdrehung
  myStepper.setLength(5);    // mm pro Umdrehung
  myStepper.setFeed(100,60); // mm pro Minute
  myStepper.setMicro(0);     // Microsteps 0-7
  myStepper.enable();        // enable Motor
}

void loop() {
  myStepper.worker();
}