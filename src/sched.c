#include "sched.h"
#include "uart.h"
#include <stddef.h>

struct task task_pool[MAX_TASKS];
// Align stacks to 16 bytes to prevent AArch64 stack alignment faults
__attribute__((aligned(16))) uint8_t task_stacks[MAX_TASKS][STACK_SIZE];

struct task *current_task = NULL;
int num_tasks = 0;

volatile uint64_t counter_a = 0;
volatile uint64_t counter_b = 0;

extern void cpu_switch_to(struct task *prev, struct task *next);

void task_create(void (*func)(void)) {
  if (num_tasks >= MAX_TASKS) {
    uart_puts("[ERR] Max tasks reached.\n");
    return;
  }

  struct task *p = &task_pool[num_tasks];
  p->pid = num_tasks;
  p->state = TASK_RUNNING;

  // Calculate stack top (Stacks grow DOWN)
  uint64_t stack_top = (uint64_t)&task_stacks[num_tasks][STACK_SIZE];
  stack_top -= 16; 

  // Initialize Context
  p->context.sp = stack_top;
  p->context.lr = (uint64_t)func; // Return address = function entry
  p->context.fp = stack_top;      // Frame pointer

  num_tasks++;
}

// Round-Robin Scheduler
void schedule(void) {
  if (num_tasks <= 1)
    return; 

  struct task *prev = current_task;

  int next_pid = (current_task->pid + 1) % num_tasks;
  struct task *next = &task_pool[next_pid];

  if (prev != next) {
    current_task = next;
    cpu_switch_to(prev, next);
  }
}

void task_a(void) {
  asm volatile("msr daifclr, #2");
  while (1) {
    counter_a++;
    for (volatile int i = 0; i < 5000; i++)
      ;
  }
}

void task_b(void) {
  asm volatile("msr daifclr, #2");
  while (1) {
    counter_b++;
    for (volatile int i = 0; i < 5000; i++)
      ;
  }
}
