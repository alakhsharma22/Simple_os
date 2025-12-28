#include "timer.h"
#include "gic.h"
#include "sched.h"
#include <stdint.h>

void timer_init(void) {
  uint64_t cntfrq;
  asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq));
  asm volatile("msr cntp_tval_el0, %0" ::"r"(cntfrq / 100)); // 100Hz
  asm volatile("msr cntp_ctl_el0, %0" ::"r"(1)); // Enable
}

void timer_handler(void) {
  // 1. Reset the Timer
  uint64_t cntfrq;
  asm volatile("mrs %0, cntfrq_el0" : "=r"(cntfrq));
  uint64_t ticks = cntfrq / 100;
  asm volatile("msr cntp_tval_el0, %0" ::"r"(ticks));

  // 2. Acknowledge Interrupt in GIC *BEFORE* Switching
  gic_eoi(30);

  // 3. Preempt the current task
  schedule();
}
