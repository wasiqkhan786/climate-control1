// Climate Control

//OneWire library is downloaded from: https://github.com/PaulStoffregen/OneWire
#include <OneWire.h>
#include <Servo.h>

OneWire  ds(10);  // temperature sensor on pin 10
Servo myservo;  // create servo object to control a servo

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  myservo.attach(9);  // attach the servo on pin 9 to the servo object
  pinMode(13, OUTPUT); // declare pin 13 to be an output red LED
  pinMode(12, OUTPUT); // declare pin 12 to be an output green LED
  myservo.write(90);  // servo initial position 90 degrees
}

// the loop routine runs over and over again forever:
void loop() {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit, dcelsius, dfahrenheit; // assigning temperature in celsius and fahrenheit degrees
  int sensorValue;

  // read the input on analog pin 0:
  sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a temperature (10 - 60 Celsius):
  dcelsius = map(sensorValue,0,1023,10,60); // will show values in celsius degrees
  dfahrenheit = map(sensorValue,0,1023,50,140); // will show values in fahrenheit degrees
  
  if ( !ds.search(addr)) { // //condition if  desired temperature is not found reset it and search again
    ds.reset_search();
    delay(250);
    return;
  }
 
  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
 
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  
  delay(1000);     
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

   for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;

  // Cooling loop
  if (celsius > dcelsius+1){
    digitalWrite(13,LOW);            // red LED is off
    digitalWrite(12,HIGH);           // green LED is on
    myservo.write(0);              // tell servo to go to 0 degrees
    delay(15);                       // waits 15ms for the servo to reach the position
    Serial.println(" ❄❄❄❄❄❄ COOLING ❄❄❄❄❄❄");
  }
  // Heating loop
  else if (celsius < dcelsius-1){
    digitalWrite(13, HIGH);          // red LED is on
    digitalWrite(12, LOW);           // green LED is off
    myservo.write(180);              // tell servo to go to 180 degrees
    delay(15);                       // waits 15ms for the servo to reach the position
    Serial.println(" ☼☼☼☼☼☼ HEATING ☼☼☼☼☼☼");
  }
  // Current Temperature = Desired Temperature
  else{
    myservo.write(90);              // servo returns to the initial position
    digitalWrite(13, LOW);          // both LEDs are off
    digitalWrite(12, LOW);
  }
  // print out the value you read:
  Serial.print(" Desired Temperature = ");
  Serial.print(dcelsius);
  Serial.print(" Celsius, ");
  Serial.print(dfahrenheit);
  Serial.println(" Fahrenheit");
  
  Serial.print(" Current Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
  Serial.println();
}
