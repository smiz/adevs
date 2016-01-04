#include <mcs51/8051.h>
#include <stdio.h>
#include <ctype.h>
 
char getchar (void) {
 char c;
 while (!RI); /* wait to receive */
 c = SBUF; /* receive from serial */
 RI = 0;
 return c;
}
 
void putchar (char c) {
 while (!TI); /* wait end of last transmission */
 TI = 0;
 SBUF = c; /* transmit to serial */
}
 
void UART_Init() {
 SCON = 0x50; /* configure serial */
 TMOD = 0x20; /* configure timer */
 TH1 = 0xE6; /* baud rate 1200 */
 TL1 = 0xE6; /* baud rate 1200 */
 TR1 = 1; /* enable timer */
 TI = 1; /* enable transmitting */
 RI = 0; /* waiting to receive */
}
 
void main() {
 UART_Init();
 for(;;) {
  char c;
  c = getchar();
  c = toupper(c);
  putchar(c);
 }
}

