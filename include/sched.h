#ifndef SCHED_H
#define SCHED_H

#include <stdint.h>

struct cpu_context {
  uint64_t x19;
  uint64_t x20;
  uint64_t x21;
  uint64_t x22;
  uint64_t x23;
  uint64_t x24;
  uint64_t x25;
  uint64_t x26;
  uint64_t x27;
  uint64_t x28;
  uint64_t fp; // x29
  uint64_t lr; // x30
  uint64_t sp;
};

enum task_state { TASK_RUNNING, TASK_WAITING };

struct task {
  long pid;
  enum task_state state;
  struct cpu_context context;
};

#define MAX_TASKS 4
#define STACK_SIZE 4096

extern struct task task_pool[MAX_TASKS];
extern struct task *current_task;
extern int num_tasks;

// Background Task Counters
extern volatile uint64_t counter_a;
extern volatile uint64_t counter_b;

void task_create(void (*func)(void));
void schedule(void);
void task_a(void);
void task_b(void);
void cpu_switch_to(struct task *prev, struct task *next); // Assembly

#endif
