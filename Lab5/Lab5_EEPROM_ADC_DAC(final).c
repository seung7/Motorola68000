#include <stdio.h>
#include <string.h>
#include <ctype.h>


//IMPORTANT
//
// Uncomment one of the two #defines below
// Define StartOfExceptionVectorTable as 08030000 if running programs from sram or
// 0B000000 for running programs from dram
//
// In your labs, you will initially start by designing a system with SRam and later move to
// Dram, so these constants will need to be changed based on the version of the system you have
// building
//
// The working 68k system SOF file posted on canvas that you can use for your pre-lab
// is based around Dram so #define accordingly before building

//#define StartOfExceptionVectorTable 0x08030000
#define StartOfExceptionVectorTable 0x0B000000

/**********************************************************************************************
**	Parallel port addresses
**********************************************************************************************/

#define PortA   *(volatile unsigned char *)(0x00400000)
#define PortB   *(volatile unsigned char *)(0x00400002)
#define PortC   *(volatile unsigned char *)(0x00400004)
#define PortD   *(volatile unsigned char *)(0x00400006)
#define PortE   *(volatile unsigned char *)(0x00400008)


/********************************************************************************************
**	Timer Port addresses
*********************************************************************************************/

#define Timer1Data      *(volatile unsigned char *)(0x00400030)
#define Timer1Control   *(volatile unsigned char *)(0x00400032)
#define Timer1Status    *(volatile unsigned char *)(0x00400032)

#define Timer2Data      *(volatile unsigned char *)(0x00400034)
#define Timer2Control   *(volatile unsigned char *)(0x00400036)
#define Timer2Status    *(volatile unsigned char *)(0x00400036)

#define Timer3Data      *(volatile unsigned char *)(0x00400038)
#define Timer3Control   *(volatile unsigned char *)(0x0040003A)
#define Timer3Status    *(volatile unsigned char *)(0x0040003A)

#define Timer4Data      *(volatile unsigned char *)(0x0040003C)
#define Timer4Control   *(volatile unsigned char *)(0x0040003E)
#define Timer4Status    *(volatile unsigned char *)(0x0040003E)

/*********************************************************************************************
**	RS232 port addresses
*********************************************************************************************/

#define RS232_Control     *(volatile unsigned char *)(0x00400040)
#define RS232_Status      *(volatile unsigned char *)(0x00400040)
#define RS232_TxData      *(volatile unsigned char *)(0x00400042)
#define RS232_RxData      *(volatile unsigned char *)(0x00400042)
#define RS232_Baud        *(volatile unsigned char *)(0x00400044)

/*********************************************************************************************
**	PIA 1 and 2 port addresses
*********************************************************************************************/

#define PIA1_PortA_Data     *(volatile unsigned char *)(0x00400050)         // combined data and data direction register share same address
#define PIA1_PortA_Control *(volatile unsigned char *)(0x00400052)
#define PIA1_PortB_Data     *(volatile unsigned char *)(0x00400054)         // combined data and data direction register share same address
#define PIA1_PortB_Control *(volatile unsigned char *)(0x00400056)

#define PIA2_PortA_Data     *(volatile unsigned char *)(0x00400060)         // combined data and data direction register share same address
#define PIA2_PortA_Control *(volatile unsigned char *)(0x00400062)
#define PIA2_PortB_data     *(volatile unsigned char *)(0x00400064)         // combined data and data direction register share same address
#define PIA2_PortB_Control *(volatile unsigned char *)(0x00400066)

/*********************************************************************************************
**	I2C Controller Registers
*********************************************************************************************/
#define PRERlo   *(volatile unsigned char *)(0x00408000)    // clock pre scaler low byte
#define PRERhi   *(volatile unsigned char *)(0x00408002)    // clock pre scaler high byte
#define CTR      *(volatile unsigned char *)(0x00408004)    // control register
#define TXR      *(volatile unsigned char *)(0x00408006)    // transmit register
#define RXR      *(volatile unsigned char *)(0x00408006)    // receive register
#define CR       *(volatile unsigned char *)(0x00408008)    // command register
#define SR       *(volatile unsigned char *)(0x00408008)    // status register

/*********************************************************************************************
**	I2C Addresses
*********************************************************************************************/
#define EEPROM_ADDR_WRITE0      0xA0                   // for bank 0 (B0 = 0)
#define EEPROM_ADDR_READ0       0xA1                   // for bank 0 (B0 = 0)
#define EEPROM_ADDR_WRITE1      0xA8                   // for bank 1 (B0 = 1)
#define EEPROM_ADDR_READ1       0xA9                   // for bank 1 (B0 = 1)

#define ADC_ADDR_WRITE          0x90
#define ADC_ADDR_READ           0x91

/*********************************************************************************************************************************
(( DO NOT initialise global variables here, do it main even if you want 0
(( it's a limitation of the compiler
(( YOU HAVE BEEN WARNED
*********************************************************************************************************************************/

unsigned int i, x, y, z, PortA_Count;
unsigned char Timer1Count, Timer2Count, Timer3Count, Timer4Count ;

/*******************************************************************************************
** Function Prototypes
*******************************************************************************************/
void Wait1ms(void);
void Wait3ms(void);
int sprintf(char *out, const char *format, ...) ;

void I2C_init(void);
// void I2C_start_command(void);
void I2C_stop_command(void);
void I2C_wait_for_transmit(void);
void I2C_wait_for_ACK(void);
void I2C_start_write(unsigned char byte);
void I2C_write(unsigned char byte);

void EEPROM_write_byte(int bank, unsigned char ADDR_high, unsigned char ADDR_low, unsigned char data);
unsigned char EEPROM_read_byte(int bank, unsigned char ADDR_high, unsigned char ADDR_low);
void EEPROM_write_block(int bank, unsigned char ADDR_high, unsigned char ADDR_low, unsigned int size);
void EEPROM_read_block(int bank, unsigned char ADDR_high, unsigned char ADDR_low, unsigned int size);

void DAC_LED(void);
void ADC_Read(void);
/*****************************************************************************************
**	Interrupt service routine for Timers
**
**  Timers 1 - 4 share a common IRQ on the CPU  so this function uses polling to figure
**  out which timer is producing the interrupt
**
*****************************************************************************************/

void Timer_ISR()
{
   	if(Timer1Status == 1) {         // Did Timer 1 produce the Interrupt?
   	    Timer1Control = 3;      	// reset the timer to clear the interrupt, enable interrupts and allow counter to run
   	    PortA = Timer1Count++ ;     // increment an LED count on PortA with each tick of Timer 1
   	}

  	if(Timer2Status == 1) {         // Did Timer 2 produce the Interrupt?
   	    Timer2Control = 3;      	// reset the timer to clear the interrupt, enable interrupts and allow counter to run
   	    PortC = Timer2Count++ ;     // increment an LED count on PortC with each tick of Timer 2
   	}

   	if(Timer3Status == 1) {         // Did Timer 3 produce the Interrupt?
   	    Timer3Control = 3;      	// reset the timer to clear the interrupt, enable interrupts and allow counter to run
   	}

   	if(Timer4Status == 1) {         // Did Timer 4 produce the Interrupt?
   	    Timer4Control = 3;      	// reset the timer to clear the interrupt, enable interrupts and allow counter to run
   	}
}

/*****************************************************************************************
**	Interrupt service routine for ACIA. This device has it's own dedicate IRQ level
**  Add your code here to poll Status register and clear interrupt
*****************************************************************************************/

void ACIA_ISR()
{}

/***************************************************************************************
**	Interrupt service routine for PIAs 1 and 2. These devices share an IRQ level
**  Add your code here to poll Status register and clear interrupt
*****************************************************************************************/

void PIA_ISR()
{}

/***********************************************************************************
**	Interrupt service routine for Key 2 on DE1 board. Add your own response here
************************************************************************************/
void Key2PressISR()
{}

/***********************************************************************************
**	Interrupt service routine for Key 1 on DE1 board. Add your own response here
************************************************************************************/
void Key1PressISR()
{}

/************************************************************************************
**   Delay Subroutine to give the 68000 something useless to do to waste 1 mSec
************************************************************************************/
void Wait1ms(void)
{
    int  i ;
    for(i = 0; i < 1000; i ++)
        ;
}

/************************************************************************************
**  Subroutine to give the 68000 something useless to do to waste 3 mSec
**************************************************************************************/
void Wait3ms(void)
{
    int i ;
    for(i = 0; i < 3; i++)
        Wait1ms() ;
}


/*********************************************************************************************
**  Subroutine to initialise the RS232 Port by writing some commands to the internal registers
*********************************************************************************************/
void Init_RS232(void)
{
    RS232_Control = 0x15 ; //  %00010101 set up 6850 uses divide by 16 clock, set RTS low, 8 bits no parity, 1 stop bit, transmitter interrupt disabled
    RS232_Baud = 0x1 ;      // program baud rate generator 001 = 115k, 010 = 57.6k, 011 = 38.4k, 100 = 19.2, all others = 9600
}

/*********************************************************************************************************
**  Subroutine to provide a low level output function to 6850 ACIA
**  This routine provides the basic functionality to output a single character to the serial Port
**  to allow the board to communicate with HyperTerminal Program
**
**  NOTE you do not call this function directly, instead you call the normal putchar() function
**  which in turn calls _putch() below). Other functions like puts(), printf() call putchar() so will
**  call _putch() also
*********************************************************************************************************/

int _putch( int c)
{
    while((RS232_Status & (char)(0x02)) != (char)(0x02))    // wait for Tx bit in status register or 6850 serial comms chip to be '1'
        ;

    RS232_TxData = (c & (char)(0x7f));                      // write to the data register to output the character (mask off bit 8 to keep it 7 bit ASCII)
    return c ;                                              // putchar() expects the character to be returned
}

/*********************************************************************************************************
**  Subroutine to provide a low level input function to 6850 ACIA
**  This routine provides the basic functionality to input a single character from the serial Port
**  to allow the board to communicate with HyperTerminal Program Keyboard (your PC)
**
**  NOTE you do not call this function directly, instead you call the normal getchar() function
**  which in turn calls _getch() below). Other functions like gets(), scanf() call getchar() so will
**  call _getch() also
*********************************************************************************************************/
int _getch( void )
{
    char c ;
    while((RS232_Status & (char)(0x01)) != (char)(0x01))    // wait for Rx bit in 6850 serial comms chip status register to be '1'
        ;

    return (RS232_RxData & (char)(0x7f));                   // read received character, mask off top bit and return as 7 bit ASCII character
}

/*********************************************************************************************************/
// Keyboard IO functions
// flush the input stream for any unread characters

void FlushKeyboard(void)
{
    char c ;

    while(1)    {
        if(((char)(RS232_Status) & (char)(0x01)) == (char)(0x01))    // if Rx bit in status register is '1'
            c = ((char)(RS232_RxData) & (char)(0x7f)) ;
        else
            return ;
     }
}

// converts hex char to 4 bit binary equiv in range 0000-1111 (0-F)
// char assumed to be a valid hex char 0-9, a-f, A-F

char xtod(int c)
{
    if ((char)(c) <= (char)('9'))
        return c - (char)(0x30);    // 0 - 9 = 0x30 - 0x39 so convert to number by sutracting 0x30
    else if((char)(c) > (char)('F'))    // assume lower case
        return c - (char)(0x57);    // a-f = 0x61-66 so needs to be converted to 0x0A - 0x0F so subtract 0x57
    else
        return c - (char)(0x37);    // A-F = 0x41-46 so needs to be converted to 0x0A - 0x0F so subtract 0x37
}

int Get2HexDigits(char *CheckSumPtr)
{
    register int i = (xtod(_getch()) << 4) | (xtod(_getch()));

    if(CheckSumPtr)
        *CheckSumPtr += i ;

    return i ;
}

int Get4HexDigits(char *CheckSumPtr)
{
    return (Get2HexDigits(CheckSumPtr) << 8) | (Get2HexDigits(CheckSumPtr));
}

int Get6HexDigits(char *CheckSumPtr)
{
    return (Get4HexDigits(CheckSumPtr) << 8) | (Get2HexDigits(CheckSumPtr));
}

int Get8HexDigits(char *CheckSumPtr)
{
    return (Get4HexDigits(CheckSumPtr) << 16) | (Get4HexDigits(CheckSumPtr));
}
/*********************************************************************************************************/


/*********************************************************************************************************************************
**  IMPORTANT FUNCTION
**  This function install an exception handler so you can capture and deal with any 68000 exception in your program
**  You pass it the name of a function in your code that will get called in response to the exception (as the 1st parameter)
**  and in the 2nd parameter, you pass it the exception number that you want to take over (see 68000 exceptions for details)
**  Calling this function allows you to deal with Interrupts for example
***********************************************************************************************************************************/

void InstallExceptionHandler( void (*function_ptr)(), int level)
{
    volatile long int *RamVectorAddress = (volatile long int *)(StartOfExceptionVectorTable) ;   // pointer to the Ram based interrupt vector table created in Cstart in debug monitor

    RamVectorAddress[level] = (long int *)(function_ptr);                       // install the address of our function into the exception table
}


/************************************************************************************
**   Initialize I2C controller
************************************************************************************/
void I2C_init(void)
{
    // set up prescaler to run the controller at 100kHz
    // first make sure the controller is disabled
    CTR = 0x00;

    // using the formula in user manual, to set I2C to run at 100kHz with 25MHz clock input, load 49 (dec) = 0x31 into PRER
    PRERlo = 0x31;
    PRERhi = 0x00;

    // now enable I2C controller and disable interrupts
    CTR = 0x80;
}

/************************************************************************************
**   Generate I2C Start Command
************************************************************************************/
/*
void I2C_start_command(void)
{
    // set STA bit
    CR |= 0x01 << 7;
}
*/

/************************************************************************************
**   Generate I2C Stop Command
************************************************************************************/
void I2C_stop_command(void)
{
    // set STO bit
    CR = 0x40;
}

/************************************************************************************
**   Wait for I2C controller to be ready (TX register has written previous command)
************************************************************************************/
void I2C_wait_for_transmit(void)
{
    // before trying to do anything, make sure the device is ready. Check status register TIP bit (1st bit)
    while( ((SR >> 1) & 0x01 ) == 1);

}

/************************************************************************************
**   Wait for ACK from I2C slave device
************************************************************************************/
void I2C_wait_for_ACK(void)
{
    // Check status register RxACK bit (7th bit)
    while( ((SR >> 7) & 0x01 ) == 1);

    //printf("ACK received!\r\n");
    //printf("Status Register = %x\r\n", SR);

}

/************************************************************************************
**   Write a byte of data to I2C transmit register (with start command)
************************************************************************************/
void I2C_start_write(unsigned char byte)
{
    //printf("starting I2C_start_write\r\n");
    // wait for I2C controller to be ready
    I2C_wait_for_transmit();

    // write byte to transmit register
    TXR = byte;

    // set STA bit and WR bit and IACK bit in command register (0b10010001 = 0x91)
    CR = 0x91;

    //printf("waiting for I2C_start_write - transmit\r\n");
    // wait for I2C controller to be ready
    I2C_wait_for_transmit();

    //printf("waiting for I2C_start_write - ACK\r\n");
    // wait for ACK from slave
    I2C_wait_for_ACK();

}

/************************************************************************************
**   Write a byte of data to I2C transmit register (no start command)
************************************************************************************/
void I2C_write(unsigned char byte)
{
    //printf("starting I2C_write\r\n");
    // wait for I2C controller to be ready
    I2C_wait_for_transmit();

    // write byte to transmit register
    TXR = byte;

    // set WR bit and IACK bit in command register (0b00010001 = 0x11)
    CR = 0x11;

    //printf("waiting for I2C_write - transmit\r\n");
    // wait for I2C controller to be ready
    I2C_wait_for_transmit();

    //printf("waiting for I2C_write - ACK\r\n");
    // wait for ACK from slave
    I2C_wait_for_ACK();

}

/************************************************************************************
**   Read a byte of data from I2C receive register (with start command)
************************************************************************************/
unsigned char I2C_read(int cont)
{
    unsigned char temp = 0x00;

    // wait for I2C controller to be ready ----- MIGHT NOT WANT THIS LINE
    I2C_wait_for_transmit();

    if (cont == 0) {    // read 1 byte and stop
        // set RD bit and ACK bit and IACK bit in command register (0b00101001 = 0x29)
        CR = 0x29;
   }

    else {              // read 1 byte and continue
        // set RD bit and IACK bit in command register (0b00100001 = 0x21)
        CR = 0x21;
    }

    // wait for read data to be ready
    while ( (SR & 0x01 ) != 0x01);
    temp = RXR;

    if (cont == 0) {    // read 1 byte and stop
        // set STO bit and IACK bit in command register (0b01000001 = 0x41)
        CR = 0x41;
    }

    else {              // read 1 byte and continue
        // set IACK bit without sending stop command (0b00000001 = 0x01)
        CR = 0x01;
    }

    return temp;
}

/************************************************************************************
**   Write a byte of data to EEPROM at specific EEPROM address
************************************************************************************/
void EEPROM_write_byte(int bank, unsigned char ADDR_high, unsigned char ADDR_low, unsigned char data)
{
    // write slave address
    if (bank == 1)
        I2C_start_write(EEPROM_ADDR_WRITE1);
    else
        I2C_start_write(EEPROM_ADDR_WRITE0);

    // write byte of data to specified address on EEPROM
    //printf("Writing ADDR high = %x\r\n", ADDR_high);
    I2C_write(ADDR_high);
    //printf("Writing ADDR low = %x\r\n", ADDR_low);
    I2C_write(ADDR_low);
    //printf("Writing data = %x\r\n", data);
    I2C_write(data);

    // generate stop command
    I2C_stop_command();
    //printf("Writing Complete!\r\n");
}

/************************************************************************************
**   Read a byte of data from EEPROM at specific EEPROM address
************************************************************************************/
unsigned char EEPROM_read_byte(int bank, unsigned char ADDR_high, unsigned char ADDR_low)
{
    unsigned char data = 0;

    if (bank == 0) {
        // write slave address
        I2C_start_write(EEPROM_ADDR_WRITE0);

        // write EEPROM address
        I2C_write(ADDR_high);
        I2C_write(ADDR_low);

        // repeated start condition starting read sequence
        I2C_start_write(EEPROM_ADDR_READ0);
        data = I2C_read(0);         // I2C_read generates the stop command
    }

    else {
        // write slave address
        I2C_start_write(EEPROM_ADDR_WRITE1);

        // write EEPROM address
        I2C_write(ADDR_high);
        I2C_write(ADDR_low);

        // repeated start condition starting read sequence
        I2C_start_write(EEPROM_ADDR_READ1);
        data = I2C_read(0);         // I2C_read generates the stop command
    }

    return data;
}

/************************************************************************************
**   Write a block of data to EEPROM at specific EEPROM address
************************************************************************************/
void EEPROM_write_block(int bank, unsigned char ADDR_high, unsigned char ADDR_low, unsigned int size)
{
    unsigned char i = 0;
    unsigned int count = 0;

    // write slave address
    if (bank == 1)
        I2C_start_write(EEPROM_ADDR_WRITE1);
    else
        I2C_start_write(EEPROM_ADDR_WRITE0);

    // write starting address in EEPROM
    I2C_write(ADDR_high);
    I2C_write(ADDR_low);

    while(count < size){
        printf("\r\nWriting 0x%x to address location 0x%2x%2x in bank %d", i, ADDR_high, ADDR_low, bank);
        I2C_write(i);

        if (ADDR_low == 0x7F) {
            I2C_stop_command();
            printf("\r\nEEPROM page full, restarting write on next page");
            ADDR_low++;                         // only increment low byte when we're not resetting it
            if (bank == 1)
                I2C_start_write(EEPROM_ADDR_WRITE1);        // start a new write
            else
                I2C_start_write(EEPROM_ADDR_WRITE0);

            // write starting address in EEPROM
            I2C_write(ADDR_high);
            I2C_write(ADDR_low);
        }

        else if (ADDR_low == 0xFF){
            if (ADDR_high == 0xFF){
                I2C_stop_command();
                ADDR_low = 0x00;                // reset address low byte and high byte
                ADDR_high = 0x00;

                if (bank == 0){
                    bank = 1;                   // need to write to new bank
                    printf("\r\nEEPROM bank full, restarting write on other bank");
                    // write slave address
                    I2C_start_write(EEPROM_ADDR_WRITE1);
                }
                else{
                    bank = 0;                   // need to write to new bank
                    printf("\r\nEEPROM bank full, restarting write on other bank");
                    // write slave address
                    I2C_start_write(EEPROM_ADDR_WRITE0);
                }
                // write starting address in EEPROM
                I2C_write(ADDR_high);
                I2C_write(ADDR_low);
            }
            else{
                I2C_stop_command();
                ADDR_low = 0x00;                // reset address low byte and increment high byte
                ADDR_high++;

                printf("\r\nEEPROM page full, restarting write on next page");
                if (bank == 1)
                    I2C_start_write(EEPROM_ADDR_WRITE1);        // start a new write
                else
                    I2C_start_write(EEPROM_ADDR_WRITE0);

                // write starting address in EEPROM
                I2C_write(ADDR_high);
                I2C_write(ADDR_low);
            }
        }
        else{
            ADDR_low++;                         // only increment low byte when we're not resetting it
        }

        count++;
        i++;
    }

    // generate stop command
    I2C_stop_command();
    printf("\r\nWriting Complete!");
}

/************************************************************************************
**   Read a block of data from EEPROM at specific EEPROM address
************************************************************************************/
void EEPROM_read_block(int bank, unsigned char ADDR_high, unsigned char ADDR_low, unsigned int size)
{
    unsigned int count = 0;
    unsigned char data = 0;

    if (bank == 0) {
        // write slave address
        I2C_start_write(EEPROM_ADDR_WRITE0);

        // write EEPROM address
        I2C_write(ADDR_high);
        I2C_write(ADDR_low);
        // repeated start condition starting read sequence
        I2C_start_write(EEPROM_ADDR_READ0);
    }
    else {
        // write slave address
        I2C_start_write(EEPROM_ADDR_WRITE1);

        I2C_write(ADDR_high);
        I2C_write(ADDR_low);

        // repeated start condition starting read sequence
        I2C_start_write(EEPROM_ADDR_READ1);
    }

    while (count < (size-1)){   // stop at the second last read, do the last read manually in order to send stop command
        if(ADDR_low == 0xFF){
            if(ADDR_high == 0xFF){
                // do final read with stop command
                data = I2C_read(0);         // I2C_read(0) generates a stop command
                printf("\r\nRead 0x%x from address location 0x%2x%2x in bank %d", data, ADDR_high, ADDR_low, bank);
                count++;
                printf("\r\nReading bank %d complete! Now reading other bank", bank);

                ADDR_low = 0x00;                // reset address low byte and high byte
                ADDR_high = 0x00;

                // restart read process in new bank
                if (bank == 0) {
                    bank = 1;
                    // write slave address
                    I2C_start_write(EEPROM_ADDR_WRITE1);

                    // write EEPROM address
                    I2C_write(ADDR_high);
                    I2C_write(ADDR_low);
                    // repeated start condition starting read sequence
                    I2C_start_write(EEPROM_ADDR_READ1);
                }
                else {
                    bank = 0;
                    // write slave address
                    I2C_start_write(EEPROM_ADDR_WRITE0);

                    I2C_write(ADDR_high);
                    I2C_write(ADDR_low);

                    // repeated start condition starting read sequence
                    I2C_start_write(EEPROM_ADDR_READ0);
                }
            }
            else{
                data = I2C_read(1);         // I2C_read(1) doesn't generate stop command
                printf("\r\nRead 0x%x from address location 0x%2x%2x in bank %d", data, ADDR_high, ADDR_low, bank);
                count++;

                ADDR_low = 0;
                ADDR_high++;
            }
        }
        else{
            data = I2C_read(1);         // I2C_read(1) doesn't generate stop command
            printf("\r\nRead 0x%x from address location 0x%2x%2x in bank %d", data, ADDR_high, ADDR_low, bank);
            count++;
            ADDR_low++;
        }
    }

    // do final read with stop command
    data = I2C_read(0);         // I2C_read(0) generates a stop command
    printf("\r\nRead 0x%x from address location 0x%2x%2x in bank %d", data, ADDR_high, ADDR_low, bank);

    printf("\r\nReading Complete!");
}

/************************************************************************************
**   Make the LED change brightness using DAC
************************************************************************************/
void DAC_LED(void) {
	unsigned char i = 0;
    int j = 0;

    int inc = 1;

	// write ADC slave adress 1 0 0 1 0 0 0 0(write)
    I2C_start_write(ADC_ADDR_WRITE);

	// write control bytes 0 1 0 0 0 0 0 0 = 0x40 for enabling DAC
    I2C_write(0x40);


	//endless loop
	//continously increased the voltage value that goes into LED.
	//for every 20 loop, the intensity of light goes up by1.
	//once it reaches 255, it starts from 0 again.
	while (1) {

        if ((j % 0x200) == 0){
            I2C_write(i);

            if (i == 255)
                inc = 0;
            else if (i == 0)
                inc = 1;

            if (inc == 1)
    		   i++;
            else{
    	       i--;
            }
        }
        j++;
	}
}

/************************************************************************************
**   Read all four ADC channels and print them out
************************************************************************************/
void ADC_Read(void) {
    unsigned char data = 0;

	// write ADC slave adress 1 0 0 1 0 0 0 0(write)
    I2C_start_write(ADC_ADDR_WRITE);

	// write control bytes for selecting ADC channel, starting with channel 0 with auto increment (0b0000_0100 = 0x04)
    I2C_write(0x04);

    I2C_stop_command();

    printf("\r\nReading 3 sensors");

    // write slave address
    I2C_start_write(ADC_ADDR_READ);

    data = I2C_read(1);
    printf("\r\nDigital value of external input = 0x%x", data);

    data = I2C_read(1);
    printf("\r\nDigital value of thermistor = 0x%x", data);

    data = I2C_read(1);
    printf("\r\nDigital value of photo-resistor = 0x%x", data);

    data = I2C_read(0);                                         // this sends the NACK and stop command
    printf("\r\nDigital value of potentiometer = 0x%x", data);

}
/******************************************************************************************************************************
* Start of user program
******************************************************************************************************************************/

void main()
{
    unsigned int i=0;
    char text[150] ;

	int PassFailFlag = 1 ;

    i = x = y = z = PortA_Count =0;
    Timer1Count = Timer2Count = Timer3Count = Timer4Count = 0;

    InstallExceptionHandler(PIA_ISR, 25) ;          // install interrupt handler for PIAs 1 and 2 on level 1 IRQ
    InstallExceptionHandler(ACIA_ISR, 26) ;		    // install interrupt handler for ACIA on level 2 IRQ
    InstallExceptionHandler(Timer_ISR, 27) ;		// install interrupt handler for Timers 1-4 on level 3 IRQ
    InstallExceptionHandler(Key2PressISR, 28) ;	    // install interrupt handler for Key Press 2 on DE1 board for level 4 IRQ
    InstallExceptionHandler(Key1PressISR, 29) ;	    // install interrupt handler for Key Press 1 on DE1 board for level 5 IRQ

    Timer1Data = 0x10;		// program time delay into timers 1-4
    Timer2Data = 0x20;
    Timer3Data = 0x15;
    Timer4Data = 0x25;

    Timer1Control = 3;		// write 3 to control register to Bit0 = 1 (enable interrupt from timers) 1 - 4 and allow them to count Bit 1 = 1
    Timer2Control = 3;
    Timer3Control = 3;
    Timer4Control = 3;

    Init_RS232() ;          // initialise the RS232 port for use with hyper terminal

/*************************************************************************************************
**  Test of scanf function
*************************************************************************************************/

    scanflush() ;                       // flush any text that may have been typed ahead
    /*
    printf("\r\nEnter Integer: ") ;
    scanf("%d", &i) ;
    printf("You entered %d", i) ;
    */

    printf("\r\nHello CPEN 412 Student\r\nYour LEDs should be Flashing\r\n") ;

    printf("\r\nStarting user program\r\nWritten by: Jerry Liu (16243140), Seungmin Lee (39572145)") ;

    // initialize I2C
    I2C_init();

    printf("\r\n----------------------------------------------------------------");
    printf("\r\n I2C Commands Summary");
    printf("\r\n----------------------------------------------------------------");
    printf("\r\n  A            - Write Single Byte of Data to EEPROM") ;
    printf("\r\n  B            - Read Single Byte of Data from EEPROM") ;
    printf("\r\n  C            - Write Block of Data (up to 128kB) to EEPROM") ;
    printf("\r\n  D            - Read Block of Data (up to 128kB) from EEPROM") ;
    printf("\r\n  E            - Change LED brightness with DAC") ;
    printf("\r\n  F            - Read ADC channels") ;
    printf("\r\n----------------------------------------------------------------\r\n");

    while(1){
        int Bank = 5;      // initialize it to something that's not 0 or 1
        unsigned char Addr_high, Addr_low, Data;
        unsigned int size;
        char c;

        FlushKeyboard() ;               // dump unread characters from keyboard
        printf("\r\n#") ;
        c = toupper(_getch());
        printf("%c",c);

        // Write Single Byte of Data to EEPROM
        if( c == (char)('A')) {
            printf("\r\nEnter EEPROM bank number (0 or 1): ");
            scanf("%d", &Bank) ;

            if ((Bank != 0) && (Bank != 1)) {
                printf("\r\nInvalid bank number!");
                continue;
            }
            else {
                printf("\r\nEnter EEPROM address high byte (00 to FF): ");
                Addr_high = Get2HexDigits(0) ;
                printf("\r\nEnter EEPROM address low byte (00 to FF): ");
                Addr_low = Get2HexDigits(0) ;
                printf("\r\nEnter byte of data to write (00 to FF): ");
                Data = Get2HexDigits(0) ;

                printf("\r\nWriting data 0x%x to EEPROM address 0x%2x%2x in Bank %d", Data, Addr_high, Addr_low, Bank);
                // write to EEPROM
                EEPROM_write_byte(Bank, Addr_high, Addr_low, Data);
            }

        }

        // Read Single Byte of Data from EEPROM
        else if( c == (char)('B')) {
            printf("\r\nEnter EEPROM bank number (0 or 1): ");
            scanf("%d", &Bank) ;

            if ((Bank != 0) && (Bank != 1)) {
                printf("\r\nInvalid bank number!");
                continue;
            }
            else {
                printf("\r\nEnter EEPROM address high byte (00 to FF): ");
                Addr_high = Get2HexDigits(0) ;
                printf("\r\nEnter EEPROM address low byte (00 to FF): ");
                Addr_low = Get2HexDigits(0) ;

                printf("\r\nReading data from EEPROM address 0x%2x%2x in bank %d", Addr_high, Addr_low, Bank);
                // reads from EEPROM
                Data = EEPROM_read_byte(Bank, Addr_high, Addr_low);
                printf("\r\nData = 0x%2x", Data);
            }
        }

        // Write Block of Data (up to 128kB) to EEPROM
        else if( c == (char)('C')) {
            printf("\r\nEnter starting EEPROM bank number (0 or 1): ");
            scanf("%d", &Bank) ;

            if ((Bank != 0) && (Bank != 1)) {
                printf("\r\nInvalid bank number!");
                continue;
            }
            else {
                printf("\r\nEnter starting EEPROM address high byte (00 to FF): ");
                Addr_high = Get2HexDigits(0) ;
                printf("\r\nEnter starting EEPROM address low byte (00 to FF): ");
                Addr_low = Get2HexDigits(0) ;
                printf("\r\nEnter number of bytes to write (0x000000 to 0x020000): ");
                size = Get6HexDigits(0) ;

                while(size > 0x20000){
                    printf("\r\nNumber of bytes must be less than 0x020000!");
                    printf("\r\nEnter number of bytes to write (0x000000 to 0x020000): ");
                    size = Get6HexDigits(0) ;
                }

                printf("\r\nWriting 0x%x bytes of data to starting EEPROM address 0x%2x%2x in Bank %d", size, Addr_high, Addr_low, Bank);
                // write to EEPROM
                EEPROM_write_block(Bank, Addr_high, Addr_low, size);

            }
        }

        // Read Block of Data (up to 128kB) from EEPROM
        else if( c == (char)('D')) {
            printf("\r\nEnter starting EEPROM bank number (0 or 1): ");
            scanf("%d", &Bank) ;

            if ((Bank != 0) && (Bank != 1)) {
                printf("\r\nInvalid bank number!");
                continue;
            }
            else {
                printf("\r\nEnter starting EEPROM address high byte (00 to FF): ");
                Addr_high = Get2HexDigits(0) ;
                printf("\r\nEnter starting EEPROM address low byte (00 to FF): ");
                Addr_low = Get2HexDigits(0) ;
                printf("\r\nEnter number of bytes to read (0x000000 to 0x020000): ");
                size = Get6HexDigits(0) ;

                while(size > 0x20000){
                    printf("\r\nNumber of bytes must be less than 0x020000!");
                    printf("\r\nEnter number of bytes to read (0x000000 to 0x020000): ");
                    size = Get6HexDigits(0) ;
                }

                printf("\r\nReading 0x%x bytes of data from starting EEPROM address 0x%2x%2x in Bank %d", size, Addr_high, Addr_low, Bank);
                // read from EEPROM
                EEPROM_read_block(Bank, Addr_high, Addr_low, size);

            }
        }
        else if( c == (char)('E')) {
            DAC_LED();
        }
        else if( c == (char)('F')) {
    		ADC_Read();
        }
        else {
            printf("\r\nInvalid Command!");

            printf("\r\n----------------------------------------------------------------");
            printf("\r\n I2C Commands Summary");
            printf("\r\n----------------------------------------------------------------");
            printf("\r\n  A            - Write Single Byte of Data to EEPROM") ;
            printf("\r\n  B            - Read Single Byte of Data from EEPROM") ;
            printf("\r\n  C            - Write Block of Data (up to 128kB) to EEPROM") ;
            printf("\r\n  D            - Read Block of Data (up to 128kB) from EEPROM") ;
            printf("\r\n  E            - Change LED brightness with DAC") ;
            printf("\r\n  F            - Read ADC channels") ;
            printf("\r\n----------------------------------------------------------------\r\n");
        }

    };

   // programs should NOT exit as there is nothing to Exit TO !!!!!!
   // There is no OS - just press the reset button to end program and call debug
}