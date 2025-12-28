#include "gic.h"
#include "timer.h"
#include "uart.h"
#include <stdint.h>

// GIC (Generic Interrupt Controller) Addresses
#define GICD_BASE 0x08000000
#define GICC_BASE 0x08010000

#define GICD_CTLR (*((volatile uint32_t *)(GICD_BASE + 0x000)))
#define GICD_ISENABLER ((volatile uint32_t *)(GICD_BASE + 0x100))
#define GICD_IPRIORITYR ((volatile uint32_t *)(GICD_BASE + 0x400))
#define GICD_ITARGETSR ((volatile uint32_t *)(GICD_BASE + 0x800))
#define GICC_CTLR (*((volatile uint32_t *)(GICC_BASE + 0x000)))
#define GICC_PMR (*((volatile uint32_t *)(GICC_BASE + 0x004)))
#define GICC_IAR (*((volatile uint32_t *)(GICC_BASE + 0x00C)))
#define GICC_EOIR (*((volatile uint32_t *)(GICC_BASE + 0x010)))

// Definition needed for UART config in gic_init
volatile unsigned int *const UART0_IMSC_GIC = (unsigned int *)0x09000038;

void gic_init(void) {
  GICD_CTLR = 0;

  volatile uint8_t *priority = (volatile uint8_t *)(GICD_BASE + 0x400);
  volatile uint8_t *targets = (volatile uint8_t *)(GICD_BASE + 0x800);
  volatile uint8_t *enables = (volatile uint8_t *)(GICD_BASE + 0x100);

  // Config UART (ID 33)
  priority[33] = 0x80; // Priority
  targets[33] = 0x01; // Target Core 0
  enables[33 / 8] |= (1 << (33 % 8)); // Enable

  // Config Timer (ID 30)
  priority[30] = 0xA0;
  targets[30] = 0x01;
  enables[30 / 8] |= (1 << (30 % 8));

  // Enable Distributor and CPU Interface
  GICD_CTLR = 1;
  GICC_PMR = 0xFF; // Allow all priorities
  GICC_CTLR = 1;

  // Enable UART Hardware Interrupts
  *UART0_IMSC_GIC = (1 << 4);
}

void gic_eoi(uint32_t id) { GICC_EOIR = id; }

// The Master Interrupt Handler
void handle_irq(void) {
  uint32_t id = GICC_IAR; // Read Interrupt ID

  if (id == 33) { // UART RX Interrupt
    uart_handle_irq();
    gic_eoi(id);         
  } else if (id == 30) { // Timer Interrupt
    timer_handler();
  } else {
    // Unknown Interrupt
    gic_eoi(id);
  }
}

// Sync Handler (Crashes, Bad Instructions)
void handle_sync(void) {
  uart_puts("\n[FATAL] Synchronous Exception!\n");
  uart_puts("Halting System.\n");
  while (1)
    ;
}
