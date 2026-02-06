// Intel 8250 serial port (UART).

#include "types.h"
#include "defs.h"
#include "param.h"
#include "x86.h"

#define COM1    0x3f8

static int uart;    // is there a uart?

void
uartinit(void)
{
  char *p;

  // Turn off the FIFO (both the transmit and receive register can hold only one byte. so bytes cant be stored and have to be processed as and when they are received)
  outb(COM1+2, 0);

  // 9600 baud, 8 data bits, 1 stop bit, parity off.
  outb(COM1+3, 0x80);    // Unlock divisor
  outb(COM1+0, 115200/9600); //115200Hz is the internal base clock in xv6. so to get a baud rate of 9600, we need to divide 115200 by 12 (=115200/9600). 12 is the lower 8 bits of 12
  outb(COM1+1, 0); // 0 are the higher 8 bits of 12
  outb(COM1+3, 0x03);    // Lock divisor, 8 data bits. Writing 0x03 clears the bit 7 which locks the divisor and sets the lower 2 bits. 
                         // 00->word length = 5bits
                         // 01-> 6bits
                         // 10-> 7bits
                         // 11-> 8bits -> hence the word length is set to 8 bits and the uart serial port now sends and receives 8 data bits per character
  outb(COM1+4, 0);
  outb(COM1+1, 0x01);    // Enable receive interrupts.

  // If status is 0xFF, no serial port.
  if(inb(COM1+5) == 0xFF)
    return;
  uart = 1;

  // Announce that we're here.
  for(p="xv6...\n"; *p; p++)
    uartputc(*p);
}

void
uartputc(int c)
{
  int i;

  if(!uart)
    return;
  for(i = 0; i < 128 && !(inb(COM1+5) & 0x20); i++); // when the bit 5 of COM1+5 register is set, it means that the transmit buffer is empty and hence, uart is                                                 // ready to accept a new byte. 
  outb(COM1+0, c);
}