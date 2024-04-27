class stepperClass {
  public:
  uint8_t stepPin;
  uint8_t dirPin;
  uint8_t ms1Pin;
  uint8_t ms2Pin;
  uint8_t ms3Pin;
  uint8_t enaPin;
  uint8_t micro=0;
  uint16_t speed=10;
  float feed=50;
  uint16_t timeUnitForSpeed=60;
  uint16_t timeUnitForFeed=60;
  uint16_t steps=200;
  float length=5;
  uint32_t stepWidth;
  bool ena=1;
  bool step=0;
  bool dir=0;
  bool busy=0;
  int currentPos=0;
  int futurePos=0;
  int posStore[32]={0};
  bool posIf[32]={0};
  bool loopActive=0;
  void (*loopPointer)()=nullptr;

  void begin(
    uint8_t _stepPin,
    uint8_t _dirPin,
    uint8_t _ms1Pin=0,
    uint8_t _ms2Pin=0,
    uint8_t _ms3Pin=0,
    uint8_t _enaPin=0) {
      stepPin=_stepPin;
      dirPin=_dirPin;
      ms1Pin=_ms1Pin;
      ms2Pin=_ms2Pin;
      ms3Pin=_ms3Pin;
      enaPin=_enaPin;
      if (stepPin && dirPin) {
        pinMode(stepPin,OUTPUT);
        pinMode(dirPin,OUTPUT);
        digitalWrite(stepPin,step);
        digitalWrite(stepPin,dir);
      }
      if (ms1Pin && ms2Pin && ms3Pin) {
        pinMode(ms1Pin,OUTPUT);
        pinMode(ms2Pin,OUTPUT);
        pinMode(ms3Pin,OUTPUT);
        setMicro(micro);
      }
      if (enaPin) {
        pinMode(enaPin,OUTPUT);
        digitalWrite(enaPin,ena);
      }
    }
  
  void turn(int value) {
    calcStepWidthFromSpeed();
    futurePos=currentPos+value;
    busy=true;
  }

  void go(float value) {
    calcStepWidthFromFeed();
    value*=steps/length;
    futurePos=currentPos+value;
  }

  void stop() {
    futurePos=currentPos;
  }

  void wait() {
    while (busy) {
      worker();
    }
  }

  void delay(uint32_t value) {
    uint64_t timer=millis()+value;
    while (millis()<timer) {
      worker();
    }
  }

  void setSteps(uint16_t value) {
    steps=value;
    calcStepWidthFromSpeed();
  }

  void setSpeed(uint16_t value1,uint16_t value2=60) {
    speed=value1;
    timeUnitForSpeed=value2;
    calcStepWidthFromSpeed();
  }

  void setLength(float value) {
    length=value;
    speed=feed/length;
    calcStepWidthFromFeed();
  }

  void setFeed(float value1,uint16_t value2=60) {
    feed=value1;
    timeUnitForFeed=value2;
    calcStepWidthFromFeed();
  }

  void calcStepWidthFromSpeed() {
    stepWidth=((uint64_t)timeUnitForSpeed*1000*1000)/(steps*2*speed);
  }

  void calcStepWidthFromFeed() {
    speed=feed/length;
    stepWidth=((uint64_t)timeUnitForFeed*1000*1000)/(steps*2*speed);
  }

  void storePos(uint8_t place=0,int value=-1) {
    if (value==-1) {
      posStore[place]=currentPos%steps;
    } else {
      posStore[place]=value%steps;
    }
    posIf[place]=1;
  }

  void setZero() {
    futurePos-=currentPos;
    currentPos=0;
  }

  void prune() {
    currentPos%=steps;
    futurePos%=steps;
  }

  int toPrune(int value) {
    return value%steps;
  }

  bool ifPos(uint8_t place=0) {
    return posIf[place];
  }

  void turnPos(uint8_t place=0) {
    calcStepWidthFromSpeed();
    futurePos=currentPos+(posStore[place]-(currentPos%steps));
    busy=true;
  }

  void turnAbs(int value1,int value2=-1) {
    if (value2==-1) {
      value2=steps;
    }
    calcStepWidthFromSpeed();
    futurePos=currentPos+((steps*value1/value2)-(currentPos%steps));
    busy=true;
  }

  void setMicro(uint8_t value) {
    if (ms1Pin && ms2Pin && ms3Pin) {
      digitalWrite(ms1Pin,value & 1);
      digitalWrite(ms2Pin,(value & 2)>>1);
      digitalWrite(ms3Pin,(value & 4)>>2);
    }
    micro=value;
  }

  void enable() {
    if (enaPin) {
      digitalWrite(enaPin,0);
    }
    ena=0;
  }

  void disable() {
    if (enaPin) {
      digitalWrite(enaPin,1);
    }
    ena=1;
  }

  void setLoop(void (*value)()) {
    loopPointer=value;
  }

  void setInputs(int value=0, ...) {
    va_list args;
    va_start(args,value);
    for (int i=0;i<value;i++) {
      pinMode(va_arg(args,int),INPUT_PULLUP);
    }
    va_end(args);
  }

  bool input(uint8_t value) {
    return !digitalRead(value);
  }

  void worker() {
    if (loopPointer!=nullptr && loopActive==false) {
      loopActive=true;
      loopPointer();
      loopActive=false;
    }
    static uint64_t timer=micros();
    if (micros()>=timer) {
      timer=micros()+stepWidth;
      if (ena) {
        return;
      }
      if (step) {
        step=false;
        digitalWrite(stepPin,false);
        return;
      } else {
        if (futurePos>currentPos) {
          dir=true;
          step=true;
          digitalWrite(dirPin,true);
          digitalWrite(stepPin,true);
          currentPos++;
          busy=true;
        }
        else if (futurePos<currentPos) {
          dir=false;
          step=true;
          digitalWrite(dirPin,false);
          digitalWrite(stepPin,true);
          currentPos--;
          busy=true;
        } else {
          busy=false;
        }
      }
    }
  }
};