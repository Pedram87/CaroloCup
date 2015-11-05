#include <Wire.h>
#include <Servo.h>
#include <CaroloCup.h>
#include <Netstrings.h>
#include "CarVars.h"

#include <pb_encode.h>
#include <pb_decode.h>
#include <messageproto.pb.h>

    const int flagEND = 19;
    const int flagESC = 125;
    const int varXOR  = 32;

    uint8_t enc_buffer[32];
    uint8_t dec_buffer[32];
    size_t message_length;
    bool status;
    String temp = "";

Car car(SERVO_PIN, ESC_PIN); //steering, esc
SRF08 frontSonar, rearSonar;
Sharp_IR rearLeftIR, rearRightIR, middleRearIR, middleFrontIR;
Odometer encoderLeft, encoderRight;

const unsigned short COM_FREQ = 60;
unsigned long previousTransmission = 0;

unsigned long prevCheck = 0; //for the LEDs
const unsigned short LEDrefreshRate = 200;

const unsigned short OVERRIDE_TIMEOUT = 3000;
unsigned long overrideRelease = 0; //variable to hold WHEN the override should be lifted
volatile boolean overrideTriggered = false; //volatile since we are accessing it inside an ISR

volatile unsigned long throttleSignalStart = 0;
volatile boolean throttleSignalPending = false;
volatile unsigned int throttleSignalFreq = 0;

volatile unsigned long steeringSignalStart = 0;
volatile boolean steeringSignalPending = false;
volatile unsigned int steeringSignalFreq = 0;

volatile byte qualityControl = 0; //if this byte is 11111111, that means the measurements we received were of good quality (controller is turned on)
const unsigned short MINIMUM_FREQUENCY = 900; //frequencies below this will be disregarded
unsigned int throttleFreq = 0;
unsigned int servoFreq = 0;

void setup() {
  car.begin();
  frontSonar.attach(US_FRONT_ADDRESS);
  frontSonar.setGain(US_GAIN);
  frontSonar.setRange(US_RANGE);
  frontSonar.setPingDelay(US_DELAY);
  rearSonar.attach(US_REAR_ADDRESS);
  rearSonar.setGain(US_GAIN);
  rearSonar.setRange(US_RANGE);
  rearSonar.setPingDelay(US_DELAY);
  rearLeftIR.attach(IR_REAR_LEFT_PIN);
  rearRightIR.attach(IR_REAR_RIGHT_PIN);
  middleRearIR.attach(IR_MIDDLE_REAR_PIN);
  middleFrontIR.attach(IR_MIDDLE_FRONT_PIN);
  encoderLeft.attach(ENCODER_LEFT_PIN);
  encoderLeft.begin();
  encoderRight.attach(ENCODER_RIGHT_PIN);
  encoderRight.begin();
  setupChangeInterrupt(OVERRIDE_THROTTLE_PIN);
  setupChangeInterrupt(OVERRIDE_SERVO_PIN);
  Serial.begin(115200); //to HLB
  Serial3.begin(9600); //to LED driver
}

void loop() {
  handleOverride(); //look for an override signal and if it exists disable serial input from the HLB
  handleInput(); //look for a serial input if override is not triggered and act accordingly
  updateLEDs(); //update LEDs depending on the mode we are currently in
  transmitSensorData(); //fetch and transmit the sensor data in the correct intervals if bluetooth is connected
}

void handleOverride() {
  noInterrupts();
  boolean qualityCheck = (qualityControl == B11111111); //true if the last 8 measurements were valid
  interrupts();
  if (qualityCheck) { //good quality, means that the RC controller is turned on, therefore we should go on override mode
    overrideTriggered = true;
    overrideRelease = millis() + OVERRIDE_TIMEOUT; //specify the moment in the future to re-enable Serial communication
  } else { //this means that at least one of the last 8 measurements was not valid, therefore we consider the signal not to be of good quality (RC controller is turned off)
    noInterrupts();
    throttleSignalPending = false; //indicate that loop() has processed/disregarded the throttle signal
    steeringSignalPending = false; //indicate that loop() has read/disregarded the servo signal
    interrupts();
  }
  if (overrideTriggered) { //if override is triggered, then you can consider signals from the channels
    noInterrupts();
    boolean _throttleSignalPending = throttleSignalPending;
    interrupts();
    if (_throttleSignalPending) {
      noInterrupts();
      throttleFreq = throttleSignalFreq; //save the throttle's frequency
      throttleSignalPending = false; //indicate that loop() has processed the throttle signal
      interrupts();
    }
    noInterrupts();
    boolean _steeringSignalPending = steeringSignalPending;
    interrupts();
    if (_steeringSignalPending) { //if there is something to be processed
      noInterrupts();
      servoFreq = steeringSignalFreq; //save the steering's frequency
      steeringSignalPending = false; //indicate that loop() has read the servo signal
      interrupts();
    }
  }

}

void handleInput() {
  
  char c;
  if (!overrideTriggered || (millis() > overrideRelease)) {
    if (overrideTriggered) { //this state is only entered when the OVERRIDE_TIMEOUT is over
      overrideTriggered = false;
      car.setSpeed(0); //after going out of the override mode, set speed and steering to initial position
      car.setAngle(0);
    }
    //---------------------------------------------------------------------------------------------------------
    if (Serial.available()) {
      String input = Serial.readStringUntil(flagEND);
      for(int i = 0, pos = 0; i< input.length(); i++,pos++){
        if(input[i] == flagESC){
          i++;
          dec_buffer[pos]=(int)input[i]^varXOR;
        }else{
          dec_buffer[pos]= input[i];
        }
      }
      processProto(32);
    }
    //---------------------------------------------------------------------------------------------------------
  } else { //we are in override mode now
    //handle override steering
    if (servoFreq) { //if you get 0, ignore it as it is not a valid value
      short diff = servoFreq - NEUTRAL_FREQUENCY;
      if (abs(diff) < OVERRIDE_FREQ_TOLERANCE) { //if the signal we received is close to the idle frequency, then we assume it's neutral
        car.setAngle(0);
      } else { //if the difference between the signal we received and the idle frequency is big enough, only then move the servo
        if (servoFreq > NEUTRAL_FREQUENCY) { //turn right if the value is larger than the idle frequency
          car.setAngle(OVERRIDE_STEER_RIGHT);
        } else {
          car.setAngle(OVERRIDE_STEER_LEFT);//turn left if the value is smaller than the idle frequency
        }
      }
    }
    //handle override throttle
    if (throttleFreq) {
      short diff = throttleFreq - NEUTRAL_FREQUENCY;
      if (abs(diff) < OVERRIDE_FREQ_TOLERANCE) { //if the signal we received is close to the idle frequency, then we assume it's neutral
        car.setSpeed(0);
      } else {
        if (throttleFreq > NEUTRAL_FREQUENCY) { //turn right if the value is larger than the idle frequency
          car.setSpeed(OVERRIDE_FORWARD_SPEED);
        } else {
          car.setSpeed(OVERRIDE_BACKWARD_SPEED);
        }
      }
    }
    while (Serial.read() != -1); //discard incoming data while on override
  }
}
void processProto(int length){
      bool protoDec;
      
      Control message;
      pb_istream_t instream = pb_istream_from_buffer(dec_buffer, length);
      protoDec = pb_decode(&instream, Control_fields, &message);
      
     if(protoDec){
      car.setSpeed(message.acceleration);
      car.setAngle(message.steering);
      
     }else{      
 /*     Serial.print("APL:");
      Serial.print(proto.length());
      Serial.println("Proto failed");*/
     }
 //       car.stop();
}
void updateLEDs() {
  if (millis() - prevCheck > LEDrefreshRate) {
    if (overrideTriggered) { //if override is triggered
      Serial3.print('m');
    } else {  //if override is NOT triggered
      if (!car.getSpeed()) { //if car is immobilized
        Serial3.print('s');
      } else { //if car is running
        if (car.getAngle() > 0) { //if car turns right
          Serial3.print('r');
        } else if (car.getAngle() < 0) { //if car turns left
          Serial3.print('l');
        } else { //if car goes straight
          Serial3.print('i');
        }
      }
    }
    prevCheck = millis();
  }
}

void transmitSensorData() {
  if (millis() - previousTransmission > COM_FREQ) {
    Sensors message;
    message.usFront = frontSonar.getDistance();
    message.usRear = rearSonar.getDistance();
    message.irFrontRight = middleFrontIR.getDistance();
    message.irRearRight = middleRearIR.getDistance();
    message.irBackLeft = rearLeftIR.getDistance();
    message.irBackRight = rearRightIR.getDistance();
    message.wheelFrontLeft = encoderLeft.getDistance();
    message.wheelRearRight = encoderRight.getDistance();
    pb_ostream_t outstream = pb_ostream_from_buffer(enc_buffer, sizeof(enc_buffer));
    status = pb_encode(&outstream, Sensors_fields, &message);
    message_length = outstream.bytes_written;
    if(status){
    //send proto over serial
    for(int i = 0; i< message_length; i++){
      if(enc_buffer[i] == flagEND || enc_buffer[i] == flagESC){
        Serial.write(flagESC);
        Serial.write(enc_buffer[i]^varXOR);
        }else{
          Serial.write(enc_buffer[i]);
        }
      }
    Serial.write(flagEND);
    }
    previousTransmission = millis();
  }
}

void setupChangeInterrupt(const unsigned short pin) { //a method to setup change interrupts on non external interrupt pins
  pinMode(pin, INPUT); //set the pin as input
  *digitalPinToPCMSK(pin) |= bit (digitalPinToPCMSKbit(pin));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(pin)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(pin)); // enable interrupt for the group
}

//the interrupt service routine for pins A8 until A15 on Arduino mega
ISR (PCINT2_vect) {
  unsigned short throttle = digitalRead(OVERRIDE_THROTTLE_PIN);
  unsigned short steering = digitalRead(OVERRIDE_SERVO_PIN);
  if (throttle) { //we are at the beginning of a throttle pulse
    if (!throttleSignalStart) { //it's the beginning of a pulse, if it is HIGH and we have not already started measuring (throttleSignalStart == 0)
      throttleSignalStart = micros(); //log down the microseconds at the beginning of the pulse
    }
  } else { //we are at the end of the throttle pulse
    if (throttleSignalStart && !throttleSignalPending) { //if the throttle signal has been started AND there is no throttle signal pending to be processed by loop()
      throttleSignalFreq = micros() - throttleSignalStart; //calculate the throttle signal's period
      throttleSignalStart = 0; //initialize the starting point of the measurement, so we do not go in here again, while the pulse is low
      throttleSignalPending = true; //signal loop() that there is a signal to handle
      qualityControl = qualityControl << 1;
      if (throttleSignalFreq < MINIMUM_FREQUENCY) { //since we are using an analog RC receiver, there is a lot of noise, usually under the frequency of 900
        qualityControl |= 0; //put a 0 bit in the end of qualityControl byte
      } else { //this means that is a valid looking signal
        qualityControl |= 1; //put a 1 bit in the end of qualityControl byte
      } //we do not need to do this for both the channels we have
    }
  }
  if (steering) { //we could be at the beginning of a steering pulse
    if (!steeringSignalStart) { //if we are already measuring something, that means this is NOT the beginning of a pulse
      steeringSignalStart = micros(); //get the current time in microseconds, ONLY IF this is really the beginning of a pulse and we weren't already measuring
    }
  } else { //we are at the end of a steering pulse
    if (steeringSignalStart && !steeringSignalPending) { //if the steering signal for the servo has started aAND there is no servo signal pending to be processed by loop()
      steeringSignalFreq = micros() - steeringSignalStart;
      steeringSignalStart = 0; //initialize the starting point of the measurement, so we do not go in here again, while the pulse is low
      steeringSignalPending = true; //signal loop() that there is a signal to handle
    }
  }
}
