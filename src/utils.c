#include "utils.h"
#include "uart.h"
#include <stdint.h>

int atoi(char *str) {
  int res = 0;
  for (int i = 0; str[i] != '\0'; ++i) {
    if (str[i] >= '0' && str[i] <= '9')
      res = res * 10 + str[i] - '0';
  }
  return res;
}

void putint(int n) {
  if (n == 0) {
    uart_puts("0");
    return;
  }
  if (n < 0) {
    uart_putc('-');
    n = -n;
  }

  char buffer[20];
  int i = 0;
  while (n > 0) {
    buffer[i++] = (n % 10) + '0';
    n /= 10;
  }
  while (i > 0)
    uart_putc(buffer[--i]);
}

int strcmp(const char *s1, const char *s2) {
  while (*s1 && (*s1 == *s2)) {
    s1++;
    s2++;
  }
  return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

void puthex(uint64_t val) {
  const char hexchars[] = "0123456789ABCDEF";
  uart_puts("0x");
  for (int i = 60; i >= 0; i -= 4) {
    uart_putc(hexchars[(val >> i) & 0xF]);
  }
}

void gets(char *buf, int max) {
  int i = 0;
  while (i < max - 1) {
    char c = uart_getc(); 

    if (c == 127 || c == 8) {
      if (i > 0) {
        i--;
        uart_puts("\b \b"); 
      }
      continue;
    }

    if (c == '\r' || c == '\n') {
      uart_putc('\r');
      uart_putc('\n');
      break;
    }

    buf[i++] = c;
    uart_putc(c); 
  }
  buf[i] = '\0'; 
}

void power_off(void) {
  // Function ID 0x84000008 = SYSTEM_OFF
  uart_puts("Shutting down...\n");
  asm volatile("mov x0, #0x84000000\n"
               "add x0, x0, #0x8\n"
               "hvc #0" // Hypervisor Call (Works for QEMU virt)
  );
  while (1)
    ; 
}
