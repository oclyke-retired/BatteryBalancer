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

  delay(500);

  initialize();


  Serial.println(crcBits(0b00000001101000011000000000000000, 21), HEX);
  Serial.println(crcBits(0b10000001101000011000000000000000, 21), HEX);
  Serial.println(crcBits(0b00000001110000101000011001101000, 22), HEX);
  
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

//  Serial.print(readRegister(0x0E), HEX);

  initialize();

  Serial.println();

}

void writeByte( uint8_t reg, uint8_t data ){
  uint32_t command = 0;

  // device address (remember to store this LSB first if non-zero
  command |= ((uint32_t)0x00 & 0b00011111) << 27;

  // register address
  command |= ((uint32_t)reg & 0b00111111) << 21;

  // register data
  command |= (uint32_t)(data) << 13;

  // // address all parts
  // nothng to do here, leaving as 0 addresses only a single part

  // Compute the 8-bit CRC!
  uint8_t crc = 0x00;
  command |= (uint32_t)(crc) << 3;

  // end bit pattern
  command |= (uint32_t)0b010;
  
  
  
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
  command |= ((uint32_t)0x00 & 0b00011111) << 27;

  // channel address
  command |= ((uint32_t)channel & 0b00001111) << 23;

  // // conversion data
  // presumably filled out by the device

  // // Write acknowledge 
  // also presumably not my job

  // // 8-bit CRC
  // Not my job

  // Reserved

  xfer32(command);
  
  
}

uint8_t readRegister( uint8_t reg ){
  uint32_t command = 0;
  
  // device address (remember to store this LSB first if non-zero
  command |= ((uint32_t)0x00 & 0b00011111) << 27;

  // register address
  command |= ((uint32_t)reg & 0b00111111) << 21;

  // register data

  // reserved

  // write acknowledge

  // 8-bit crc

  // reserved

  // extract and return the register data
  uint32_t result = xfer32(command);
  uint8_t regval = ((result >> 13) & 0xFF);
}

uint32_t xfer32( uint32_t out ){

  // Copy the command into a buffer to transfer
  uint8_t buff[4];
  buff[0] = (out >> 24) & 0xFF;
  buff[1] = (out >> 16) & 0xFF;
  buff[2] = (out >> 8) & 0xFF;
  buff[3] = (out >> 0) & 0xFF;

  
  digitalWrite(CS_PIN, LOW);
  SPI.beginTransaction(SPISettings(SPI_CLK, SPI_ORDER, SPI_MODE));
  SPI.transfer(buff, 4); // Use the buffer to both send out the data and collect the return data
  SPI.endTransaction();
  digitalWrite(CS_PIN, HIGH);

  // Reassemble the result and return it 
  uint32_t res = ((buff[0] << 24) | (buff[1] << 16) | (buff[2] << 8) |(buff[3] << 0));

  return res;
}

void initialize( void ){

  Serial.print("Initializing: \n");

  uint32_t step1 = xfer32(0x01C2B6E2);  // A single command should be sent to all devices in the
  Serial.print("Step 1: 0x");           // chain to assert the lock device address bit (D2), to deassert
  Serial.print(step1, HEX);             // the increment device address bit (D1), and to assert the
  Serial.println();                     // daisy-chain register readback bit (D0). The 32-bit write
                                        // command is 0x01C2B6E2. 
 
  uint32_t step2 = xfer32(0x038716CA);  // A second command should be sent to all devices in the
  Serial.print("Step 2: 0x");           // chain to write the address of the lower byte of the control
  Serial.print(step2, HEX);             // register, 0x0E, to the read register on all devices. The 32-bit
  Serial.println();                     // write command is 0x038716CA.

  const uint8_t num_devices = 1;
  for(uint8_t indi = 0; indi < (num_devices+1); indi++){    // To verify that all AD7280As in the chain have received and
    uint32_t step3 = xfer32(0xF800030A);                    // locked their unique device address, a daisy-chain register read
    Serial.print("Step 3: 0x");                             // should be requested from all devices. This can be done by
    Serial.print(step3, HEX);                               // continuing to apply sets of 32 SCLKs framed by CS until
    Serial.println();                                       // the lower byte of the control register of each device in the
  }                                                         // daisy chain has been read back. The user should confirm
                                                            // that all device addresses are in sequence. The 32-bit write
                                                            // command is 0xF800030A.
  Serial.println();
}

uint8_t crcWrite(uint32_t data){
  
}

uint8_t crcRead(uint32_t data){
  
}

uint8_t crcBits(uint32_t data, uint8_t bits){

  // calculates the crc from the first 'bits' bits of the 'data' (from MSB toward LSB)
  
  uint8_t xor_5 = 0x00;
  uint8_t xor_4 = 0x00;
  uint8_t xor_3 = 0x00;
  uint8_t xor_2 = 0x00;
  uint8_t xor_1 = 0x00;

  uint8_t crc_7 = 0x00;
  uint8_t crc_6 = 0x00;
  uint8_t crc_5 = 0x00;
  uint8_t crc_4 = 0x00;
  uint8_t crc_3 = 0x00;
  uint8_t crc_2 = 0x00;
  uint8_t crc_1 = 0x00;
  uint8_t crc_0 = 0x00;
  
  
  
  for( uint8_t i = 0; i < bits; i++){
    uint8_t b = ((data >> (31-i)) & 0x01);

    xor_5 = (crc_4 ^ crc_7);
    xor_4 = (crc_2 ^ crc_7);
    xor_3 = (crc_1 ^ crc_7);
    xor_2 = (crc_0 ^ crc_7);
    xor_1 = (b ^ crc_7);

    crc_7 = crc_6;
    crc_6 = crc_5;
    crc_5 = xor_5;
    crc_4 = crc_3;
    crc_3 = xor_4;
    crc_2 = xor_3;
    crc_1 = xor_2;
    crc_0 = xor_1; 
  }

  uint8_t crc =((crc_7 << 7) | \
                (crc_6 << 6) | \
                (crc_5 << 5) | \
                (crc_4 << 4) | \
                (crc_3 << 3) | \
                (crc_2 << 2) | \
                (crc_1 << 1) | \
                (crc_0 << 0) );
                
  return crc;
}

  
