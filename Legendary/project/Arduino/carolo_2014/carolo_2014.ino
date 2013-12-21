// Need the Servo library
#include <Servo.h>
#define MAX_STEERING_ANGLE_R 33
#define MAX_STEERING_ANGLE_L -33
#define MIN_MOTOR_SPEED 1400
#define MAX_MOTOR_SPEED 1623
#define BRAKE_SPEED_MAX 1530
#define BRAKE_SPEED_MIN 1150
#define INIT_MOTOR_SPEED 1520 

// This is our motor.
Servo myMotor;
Servo mySteering;
Servo myCamera;
Servo replicateSteering;
Servo replicateMotor;

int motorPin = 6;
int steeringPin = 9;
int cameraPin = 10;
int steeringLedPin = 4;
int motorLedPin = 7;
int ledPin1 = 51;
int ledPin2 = 53;

long int lastTimeStamp;

boolean run = true;
int speed = INIT_MOTOR_SPEED;
int angle = 0;
int freq = 0;
int setFreq = 0;
int acumFreqError = 0;
int propGain = 1;
int intGain = 5;
int camAngle = 0;
int read;
int readbyte;
boolean first = true;
boolean motor = false;
boolean steering = false;
boolean brake = false;
boolean camera = false;
boolean applybrake = false;
boolean cruiseCtrl = false;
boolean applyCruiseCtrl = false;
int multiplier = 1;
int camMultiplier = 1;
boolean takeOver = false;

int ms;
unsigned long time;
unsigned long int cnt=0, cntOld = 0;
float carSpeed = 0;

int reading = 0;

void setup()
{
  // Setup the servos
  myMotor.attach(motorPin);
  mySteering.attach(steeringPin);
  replicateSteering.attach(steeringLedPin);
  replicateMotor.attach(motorLedPin);
  myCamera.attach(cameraPin);
  // Set a startup speed
  myMotor.writeMicroseconds(975);
  mySteering.write(90);
  myCamera.write(90);
  
  time = 0;

  attachInterrupt(3, countRotations, FALLING);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  // initialize serial communication
  Serial.begin(115200);
  Serial.println("initialized");

  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
}

void loop()
{
  if(run) {
    myMotor.writeMicroseconds(speed);
    Serial.print("speed: ");
    Serial.println(speed);
    Serial.print("angle: ");
    Serial.println(90);
    speed = 0;
    run = false;
  }
  evaluateReceiver();
  //Serial.print("speed reading: ");
  if(!takeOver) {
    if((read = Serial.available()) > 0) {
      //Read data
      while(read > 0) {
        readbyte = Serial.read();
        //Serial.println(readbyte);
        if(readbyte == 47) {
          break;
        }

        if (!first && motor) {
          speed = speed * 10 + (readbyte - 48);
        }
        if (!first && steering){
          if(readbyte < 48) {
            multiplier = -1;
          } 
          else {
            angle = angle * 10 + (readbyte - 48);
          }
        }
        if(!first && camera){
          if(readbyte < 48) {
            camMultiplier = -1;
          } 
          else {
            camAngle = camAngle * 10 + (readbyte - 48);
          }
        }
        if(!first && brake) {
          if(readbyte == 43) {
            applybrake = true;
          } 
          else {
            applybrake = false;
          }
        }
        if (!first && cruiseCtrl) {
          if (readbyte == 43) {
            applyCruiseCtrl = true;
          } else if (readbyte == 45) {
            applyCruiseCtrl = false;
            speed = 950;
            controlMotor();
          } else {
            applyCruiseCtrl = true;
            setFreq = setFreq * 10 + (readbyte - 48);
            acumFreqError = 0;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
          }
        }
        if (first && readbyte == 109) {
          motor = true;
          first = false;
          speed = 0;
        }
        if(first && readbyte == 115) {
          steering = true;
          first = false;
          angle = 0;
          multiplier = 1;
        }
        if(first && readbyte == 98) {
          brake = true;
          first = false;
        }
        if (first && readbyte == 102) {
          cruiseCtrl = true;
          first = false;
          setFreq = 0;
        }
        if(first && readbyte == 99) {
          camera = true;
          first = false;
          camAngle = 0;
          camMultiplier = 1;
        }
        read = read - 1;
      }
      //Process data
      if(motor) {
        //Serial.println(speed);
        if(speed > MIN_MOTOR_SPEED && speed < MAX_MOTOR_SPEED) {
          controlMotor();
        }
      }
      if(steering) {
        //Serial.println(angle);
        if(angle > MAX_STEERING_ANGLE_L && angle < MAX_STEERING_ANGLE_R) {
          controlSteering();
        }
      }
      if(camera) {
        Serial.println(camAngle);
        if(camAngle > MAX_STEERING_ANGLE_L && camAngle < MAX_STEERING_ANGLE_R) {
          controlCamera();
        }
      }
      if(brake) {
        if(applybrake) {
          digitalWrite(ledPin1, HIGH);
          replicateMotor.writeMicroseconds(1000);
        } 
        else {
          digitalWrite(ledPin1, LOW);
          replicateMotor.writeMicroseconds(1530);
        }
      }

      first = true;
      motor = false;
      steering = false;
      brake = false;
      cruiseCtrl = false;
      camera = false;
      multiplier = 1;
      camMultiplier = 1;
    }
  } 
  else {
    multiplier = 1;
    Serial.println(angle);
    Serial.println(speed);
    controlMotor();
    controlSteering();
  }
  ms = ms + 1;
  //Calculate car speed
  if(ms > 3) {
    unsigned long curr = millis();
    //Serial.println((curr - time));
    int diff = cnt * 1000 / (curr - time);
    float newSpeed = diff * 0.03;
    if(newSpeed != carSpeed) {
      Serial.print("Car speed:");
      Serial.println(newSpeed);
      carSpeed = newSpeed;
    }
    freq = int(diff*1.2);
    if (applyCruiseCtrl) {
      //Serial.println("Enter cruise control");
      //Serial.print("Frequency: ");
      //Serial.println(freq);
      int error = setFreq - freq;
      Serial.print("Error: ");
      Serial.println(error);
      int errorSign = error < 0 ? -1 : +1;
      if (abs(error) > 12) {
        speed += 5*errorSign;
      }
      else if (abs(error) > 6) {
        speed += 3*errorSign;
      } else if (abs(error) > 3) {
        speed += errorSign;
      }
      speed = constrain(speed, 1547, 1623);
      //Serial.print("Speed :");
      //Serial.println(speed);
      controlMotor();
      /*
      speed = 1520 + propGain*error + intGain*acumFreqError;
      */
    }
    //cntOld = cnt;
    cnt = 0;
    ms = 0;
    time = curr;
  }
  delay(10);
}

void controlMotor() {
  Serial.print("speed: ");
  Serial.println(speed);
  myMotor.writeMicroseconds(speed);
  if(speed < BRAKE_SPEED_MAX && speed > BRAKE_SPEED_MIN) {
    digitalWrite(ledPin1, HIGH);
    replicateMotor.writeMicroseconds(1000);
  } 
  else {
    digitalWrite(ledPin1, LOW);
    replicateMotor.writeMicroseconds(1530);
  }
}

void controlSteering() {
  Serial.print("angle: ");
  int inpAngle = angle;
  angle = 90 + angle * multiplier;
  Serial.println(angle);
  mySteering.write(angle);
  if(angle > 100) { 
    replicateSteering.write(120);
  } 
  else if(angle < 80){
    replicateSteering.write(60);
  } 
  else {
    replicateSteering.write(97);
  }
}

void controlCamera() {
  Serial.print("cam angle: ");
  int inpAngle = camAngle;
  camAngle = 90 + camAngle * camMultiplier;
  Serial.println(camAngle);
  myCamera.write(camAngle);
}

void evaluateReceiver()
{
  int receiverSpeed = pulseIn(2, HIGH, 25000);
  receiverSpeed = map(receiverSpeed, 1200,2600,1159,1759);
  int receiverSteer = pulseIn(3, HIGH, 25000);
  //Serial.println(receiverSpeed);
  //Serial.println(receiverSteer);
  receiverSteer = map(receiverSteer, 1600,2300,819,2219);
  if(receiverSpeed > 670 && receiverSpeed < 1370 && !takeOver) {
    takeOver = true;
    digitalWrite(ledPin2, LOW);
    lastTimeStamp = millis();
  }
  if(takeOver) {
    speed = receiverSpeed;
    receiverSteer = (receiverSteer - 1519)/10;
    angle = receiverSteer * (-1);
    if (millis() - lastTimeStamp >= 500) {
      digitalToggle(ledPin2);
      lastTimeStamp = millis();
    }
  }
  if((angle > 150 || angle < -150) && takeOver) {
    digitalWrite(ledPin2, LOW);
    takeOver = false;
  }
}

void digitalToggle(int pin) {
  if (digitalRead(pin) == HIGH)
    digitalWrite(pin, LOW);
  else
    digitalWrite(pin, HIGH);
}

void countRotations() {
  cnt++;
}
