# Lab5: I2C driver Development for wishbond buses


## Goal:
Use Motorola 68k softcore process on DE1-SoC FPGA Board, 
1. Develop I2C driver using embedded C so that I2C controller can read/write from I2C slave devices. This is done by setting Control, Transmit, Receive, Command, Status Registers for Wishbone Buses. </br>
    * Wishbone is an open standard bus designed specifically for FPGAs. 
</br></br>
2. Read/write a single Byte or Block up to 128k at any address in the external EEPROM chip through I2C connection
    * unlike flash, EEProme allows byte-by-byte erasing and programming
</br></br>

3. Read analog data from ADC/DAC converter through I2C connection.

## Hardware:
* FPGA:
    * DE1-SOC
    * Download motorola68k softcore + I2C controller VHDL code
* EEProm : 
    * Microchip's 24AA1025, 128k bytes EEProm
* ADC/DAC converter: 
    * NXP's PCF8591 is soldered to a tiny PCB with three sensors (Potentiometer, Light and Temperature)

## Block diagram :

<img src="image/blockdiagram.png" >
</br></br>

## I2C read, write:
READ
    1. generate start command
    2. write slave address + write bit
    3. receive acknowledge from slave
    4. write data
    5. receive acknowledge from slave
    6. generate stop command

WRITE
    1. generate start signal
    2. write slave address + write bit
    3. receive acknowledge from slave
    4. write memory location
    5. receive acknowledge from slave
    6. generate repeated start signal
    7. write slave address + read bit 
    8. receive acknowledge from slave
    9. read byte from slave
    10. write no acknowledge (NACK) to slave, indicating end of transfer
    11. generate stop signal