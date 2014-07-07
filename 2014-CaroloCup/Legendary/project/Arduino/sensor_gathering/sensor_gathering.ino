#include <Wire.h>
#include <stdio.h>
#define ADDRESSBACK 115
#define ADDRESSFRONT 112
//#define ADDRESSMIDDLE 114

int irpin1 = 0;                 // analog pin used to connect the sharp sensor
int irpin2 = 1;                 // analog pin used to connect the sharp sensor
int irpin3 = 2;                 // analog pin used to connect the sharp sensor
int irpin4 = 3;                 // analog pin used to connect the sharp sensor
int infraCount = 0;
int uf = -1;
int ub = -1;

int val = 0,cm = 0;                 // variable to store the values from sensor(initially zero)
 // A normal distribution
 // out[] holds the values wanted in cm
int out[] = { 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3  };  // 28
  // note: the in array should have increasing values
// in[] holds the measured analogRead() values for defined distances
int in[]  = {27,31,35,39,43,48,56,65,77,85,93,101,113,121,135,150,165,181,203,220,245,275,310,350,400,460,560,630}; 
int size = 28;

int reading = 0;

unsigned long time;
/*unsigned long int cntF=0, cntR=0, cntOldF = 0, cntOldR = 0;
int ms = 0;*/

void setup()
{
  Serial.begin(115200);               // starts the serial monitor
  Wire.begin();
  
  //attachInterrupt(0, countRotationsF, FALLING);
  //attachInterrupt(1, countRotationsR, FALLING);
}

int getDistance(int val)
{
  // take care the value is within range
  // val = constrain(val, _in[0], _in[size-1]);
  if (val <= in[0]) return -1;
  if (val >= in[size-1]) return -1;
  
  // search right interval
  uint8_t pos = 1;  // _in[0] allready tested
  while(val > in[pos]) pos++;

  // this will handle all exact "points" in the _in array
  if (val == in[pos]) return out[pos];

  // interpolate in the right segment for the rest
  return map(val, in[pos-1], in[pos], out[pos-1], out[pos]);
}

void loop()
{
  if(infraCount >= 2){
  int ir1, ir2, ir3, ir4;
  ir1 = getDistance(analogRead(irpin1));
  ir2 = getDistance(analogRead(irpin2));
  ir3 = getDistance(analogRead(irpin3));
  ir4 = getDistance(analogRead(irpin4)); 
  char irStr[13];
  sprintf(irStr, "i%2d,%2d,%2d,%2d.", ir1, ir2, ir3, ir4);
/*  Serial.print("i");
  Serial.print(ir1);
  Serial.print(",");
  Serial.print(ir2);
  Serial.print(",");
  Serial.print(ir3);
  Serial.print(",");
  Serial.print(ir4);
  Serial.println(".");*/
  Serial.println(irStr);
  infraCount = 0;
  }
  //instrumt sensor to read echoes
 Wire.beginTransmission(ADDRESSFRONT); // transmit to front sensor #112 (0x70)(It is actually 0xE0)
                               // the address specified in the datasheet is 224 (0xE0)
                               // but i2c adressing uses the high 7 bits so it's 112
  Wire.write(byte(0x00));      // sets register pointer to the command register (0x00)  
  Wire.write(byte(0x51));      // command sensor to measure in "inches" (0x50) // use 0x51 for centimeters // use 0x52 for ping microseconds
  Wire.write(byte(0x02));      // Write in location 2(Change Range)
  Wire.write(byte(0x18));      // Write the minimum value
  Wire.write(byte(0x01));      // write in location one (change gain)
  Wire.write(byte(0x05));      // Write the minimum value 
  Wire.endTransmission();      // stop transmitting
  
  /*Wire.beginTransmission(ADDRESSMIDDLE); // transmit to front sensor #112 (0x70)(It is actually 0xE0)
  Wire.write(byte(0x00));      // sets register pointer to the command register (0x00)  
  Wire.write(byte(0x51));      // command sensor to measure in "inches" (0x50) // use 0x51 for centimeters // use 0x52 for ping microseconds
  Wire.write(byte(0x02));      // Write in location 2(Change Range)
  Wire.write(byte(0x00));      // Write the minimum value
  Wire.write(byte(0x01));      // write in location one (change gain)
  Wire.write(byte(0x00));      // Write the minimum value                       
  Wire.endTransmission();      // stop transmitting
  */
  Wire.beginTransmission(ADDRESSBACK); // transmit to Back sensor #115 (0x73) (It is actually 0xE6)
  Wire.write(byte(0x00));      // sets register pointer to the command register (0x00)  
  Wire.write(byte(0x51));      // command sensor to measure in "inches" (0x50)   // use 0x51 for centimeters  // use 0x52 for ping microseconds
  Wire.write(byte(0x02));      // Write in location 2(Change Range)
  Wire.write(byte(0x18));      // Write the minimum value
  Wire.write(byte(0x01));      // write in location one (change gain)
  Wire.write(byte(0x05));      // Write the minimum value 
  Wire.endTransmission();      // stop transmitting
  // wait for readings to happen
  delay(8);   // datasheet suggests at least 65 milliseconds
  
  infraCount++;

////////////////////////////////
 // instrumt sensor to return a particular echo reading
  Wire.beginTransmission(ADDRESSFRONT); // transmit to device #112
  Wire.write(byte(0x02));      // sets register pointer to echo #1 register (0x02)
  Wire.endTransmission(); // stop transmitting
   /*Wire.beginTransmission(ADDRESSMIDDLE); // transmit to device #114
  Wire.write(byte(0x02));      // sets register pointer to echo #1 register (0x02)
  Wire.endTransmission();      // stop transmitting
 */
  Wire.beginTransmission(ADDRESSBACK); // transmit to device #115
  Wire.write(byte(0x02));      // sets register pointer to echo #1 register (0x02)
  Wire.endTransmission();      // stop transmitting

  // request reading from sensor
  Wire.requestFrom(ADDRESSFRONT, 2);    // request 2 bytes from slave device #112
  // step 5: receive reading from sensor

  if(2<= Wire.available())    // if two bytes were received
  {
    reading = Wire.read();  // receive high byte (overwrites previous reading)
    reading = reading << 8;    // shift high byte to be high 8 bits
    reading |= Wire.read(); // receive low byte as lower 8 bits
    if(reading < 1000){
    uf = reading;
    }
  }
  
 /*     Wire.requestFrom(ADDRESSMIDDLE, 2);    // request 2 bytes from slave device #114
  // step 5: receive reading from sensor
    int um = -1;
  
  if(2<= Wire.available())    // if two bytes were received
  {
    reading = Wire.read();  // receive high byte (overwrites previous reading)
    reading = reading << 8;    // shift high byte to be high 8 bits
    reading |= Wire.read(); // receive low byte as lower 8 bits
    //Serial.print("MIDDLE=");
    //Serial.println(reading);   // print the reading
    if(reading < 1000){
    um = reading;
    }
  }*/
  
   Wire.requestFrom(ADDRESSBACK, 2);    // request 2 bytes from slave device #112
  // step 5: receive reading from sensor

  if(2<= Wire.available())    // if two bytes were received
  {
    reading = Wire.read();  // receive high byte (overwrites previous reading)
    reading = reading << 8;    // shift high byte to be high 8 bits
    reading |= Wire.read(); // receive low byte as lower 8 bits
    //if(reading < 1000 && reading != -1){
    if(reading < 1000){

    ub = reading;
    }
  }

  char uStr[13];

  //sprintf(uStr, "u%3d,%3d,%3d.", uf,um,ub);
    sprintf(uStr, "u%3d,%3d.", uf,ub);
  /*Serial.print("u");
  Serial.print(uff);
  Serial.print(",");
  Serial.print(ubf);
  Serial.println(".");*/
  Serial.println(uStr);
  /*ms++;
  if(ms > 2) {
    //Calculate car speed
    unsigned long curr = millis();
    Serial.print(cntF);
    Serial.print(",");
    Serial.println(cntR);
    Serial.println((curr - time));
    int diffF = cntF * 1000 / (curr - time);
    int diffR = cntR * 1000 / (curr - time);
    float carSpeedF = diffF * 0.03;
    float carSpeedR = diffR * 0.03;
    Serial.print("h");
    Serial.print((int)(carSpeedF*100));
    Serial.print(",");
    Serial.print((int)(carSpeedR*100));
    Serial.println(".");
    ms=0;
    cntF = 0;
    cntR = 0;
    time = curr;
  }*/
}

/*void countRotationsF() {
  cntF++;
}

void countRotationsR() {
  cntR++;
}*/

