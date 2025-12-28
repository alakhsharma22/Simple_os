#include "uart.h"
#include <stdint.h>

// PL011 UART Registers
volatile unsigned int *const UART0_DR = (unsigned int *)0x09000000;
volatile unsigned int *const UART0_FR = (unsigned int *)0x09000018;
volatile unsigned int *const UART0_IMSC = (unsigned int *)0x09000038;
volatile unsigned int *const UART0_ICR = (unsigned int *)0x09000044;

#define UART_FR_TXFF (1 << 5) // Transmit FIFO Full
#define UART_FR_RXFE (1 << 4) // Receive FIFO Empty

#define UART_BUF_SIZE 128
volatile char uart_buf[UART_BUF_SIZE];
volatile int uart_buf_r = 0;
volatile int uart_buf_w = 0;

void uart_putc(char c) {
  // Wait until Transmit FIFO is not full
  while (*UART0_FR & UART_FR_TXFF)
    ;
  *UART0_DR = c;
}

void uart_puts(const char *s) {
  while (*s) {
    uart_putc(*s++);
  }
}

// The "Smart" Get Char - Uses WFI to sleep while waiting
char uart_getc(void) {
  // While buffer is empty
  while (uart_buf_r == uart_buf_w) {
    // Wait For Interrupt (Saves Power, lets background tasks run via Timer IRQ)
    asm volatile("wfi");
  }

  char c = uart_buf[uart_buf_r];
  uart_buf_r = (uart_buf_r + 1) % UART_BUF_SIZE;
  return c;
}

void uart_handle_irq(void) {
  // Drain the hardware FIFO into our Ring Buffer
  while (!(*UART0_FR & UART_FR_RXFE)) {
    char c = (char)(*UART0_DR);

    int next_w = (uart_buf_w + 1) % UART_BUF_SIZE;
    if (next_w != uart_buf_r) { // Check for overflow
      uart_buf[uart_buf_w] = c;
      uart_buf_w = next_w;
    }
  }
  // Clear the UART hardware interrupt flag
  *UART0_ICR = (1 << 4);
}
