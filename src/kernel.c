#include "../include/gic.h"
#include "../include/mm.h"
#include "../include/sched.h"
#include "../include/timer.h"
#include "../include/uart.h"
#include <stddef.h>
#include <stdint.h>

void shell_loop(void);

void kernel_main() {
  uart_puts("\n\n--- Simple OS Kernel Booting ---\n");

  uart_puts("[INIT] Initializing MMU... ");
  mmu_init();
  uart_puts("Done.\n");

  uart_puts("[INIT] Initializing GIC... ");
  gic_init();
  uart_puts("Done.\n");

  uart_puts("[INIT] Initializing Timer... ");
  timer_init();
  uart_puts("Done.\n");

  uart_puts("[INIT] Creating Background Tasks... ");
  current_task = &task_pool[0];
  current_task->pid = 0;
  current_task->state = TASK_RUNNING;
  num_tasks = 1;

  task_create(task_a);
  task_create(task_b);
  uart_puts("Done (Tasks Created).\n");

  uart_puts("[INIT] Enabling Global Interrupts...\n");
  asm volatile("msr daifclr, #2"); // Unmask IRQ

  shell_loop();
}
