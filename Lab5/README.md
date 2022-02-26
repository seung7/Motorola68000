# Lab5: I2C driver Development for wishbond buses


## Goal:
Use Motorola 68k softcore process on DE1-SoC FPGA Board, 
1. Develop I2C driver using embedded C so that I2C controller can read/write from I2C slave devices. This is done by setting Control, Transmit, Receive, Command, Status Registers for Wishbond Buses. 

2. Read/write a single Byte or Block up to 128k at any address in the external EEPROM chip through I2C connection

3. Read analog data from ADC/DAC converter through I2C connection.

## Hardware:
* EEProm : 
    * Microchip's 24AA1025, 128k bytes EEProm
* ADC/DAC converter: 
    * NXP's PCF8591 is soldered to a tiny PCB with three sensors (Potentiometer, Light and Temperature)

## Block diagram :

<img src="image/blockdiagram.png" width="500">

