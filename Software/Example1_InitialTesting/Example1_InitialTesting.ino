#include <SPI.h>

#define SPI_MODE SPI_MODE1
#define SPI_CLK 100000
#define SPI_ORDER MSBFIRST
#define CS_PIN 2


void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);

  Serial.println("Hello, I am the battery balancer");

  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  SPI.begin();

  SPI.beginTransaction(SPISettings(SPI_CLK, SPI_ORDER, SPI_MODE));
  SPI.transfer(0x00);
  SPI.endTransaction();

  
  
}

void loop() {
  // put your main code here, to run repeatedly:

  static int count = 0;
  delay(1000);
//  Serial.println(count++);

//  digitalWrite(CS_PIN, LOW);
//  SPI.beginTransaction(SPISettings(SPI_CLK, SPI_ORDER, SPI_MODE));
//
//  Serial.print(SPI.transfer( 0x00 ));
//  Serial.print(SPI.transfer( 0x00 ));
//  Serial.print(SPI.transfer( 0x00 ));
//  Serial.print(SPI.transfer( 0x00 ));
//  
//  SPI.endTransaction();
//  digitalWrite(CS_PIN, HIGH);

  Serial.print(readRegister(0x0E), HEX);

  Serial.println();

}

void writeByte( uint8_t reg, uint8_t data ){
  uint32_t command = 0;

  // device address (remember to store this LSB first if non-zero
  command |= (0x00 & 0b00011111) << 27;

  // register address
  command |= (reg & 0b00111111) << 21;

  // register data
  command |= (data) << 13;

  // // address all parts
  // nothng to do here, leaving as 0 addresses only a single part

  // Compute the 8-bit CRC!
  uint8_t crc = 0x00;
  command |= (crc) << 3;

  // end bit pattern
  command |= 0b010;
  
  
  
  digitalWrite(CS_PIN, LOW);
  SPI.beginTransaction(SPISettings(SPI_CLK, SPI_ORDER, SPI_MODE));

  SPI.transfer( (command >> 24) & 0xFF );
  SPI.transfer( (command >> 16) & 0xFF );
  SPI.transfer( (command >> 8) & 0xFF );
  SPI.transfer( (command >> 0) & 0xFF );
  
  SPI.endTransaction();
  digitalWrite(CS_PIN, HIGH);
}


uint16_t readConversion( uint8_t channel ){
  uint32_t command = 0;

  // device address (remember to store this LSB first if non-zero
  command |= (0x00 & 0b00011111) << 27;

  // channel address
  command |= (channel & 0b00001111) << 23;

  // // conversion data
  // presumably filled out by the device

  // // Write acknowledge 
  // also presumably not my job

  // // 8-bit CRC
  // Not my job

  // Reserved
  
  
}

uint8_t readRegister( uint8_t reg ){
  uint32_t command = 0;
  
  // device address (remember to store this LSB first if non-zero
  command |= (0x00 & 0b00011111) << 27;

  // register address
  command |= (reg & 0b00111111) << 21;

  // register data

  // reserved

  // write acknowledge

  // 8-bit crc

  // reserved


  // Copy the command into a buffer to transfer
  uint8_t buff[4];
  buff[0] = (command >> 24) & 0xFF;
  buff[1] = (command >> 16) & 0xFF;
  buff[2] = (command >> 8) & 0xFF;
  buff[3] = (command >> 0) & 0xFF;

  
  digitalWrite(CS_PIN, LOW);
  SPI.beginTransaction(SPISettings(SPI_CLK, SPI_ORDER, SPI_MODE));

  SPI.transfer(buff, 4); // Use the buffer to both send out the data and collect the return data
  
  SPI.endTransaction();
  digitalWrite(CS_PIN, HIGH);

  // extract and return the register data
  uint32_t result = ((buff[0] << 24) | (buff[1] << 16) | (buff[2] << 8) |(buff[3] << 0));
  uint8_t regval = ((result >> 13) & 0xFF);
}
