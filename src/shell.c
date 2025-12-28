#include "sched.h"
#include "uart.h"
#include "utils.h"

struct file {
  char name[32];
  char *content;
  int size;
  int is_executable;
};

struct file ramdisk[] = {
    {"readme.txt", "Welcome to Simple OS! This is a RAM filesystem.", 46, 0},
    {"credits.txt", "Created by a brave OS Developer and AI researcher", 32, 0},
    {"calc.app", "[Binary Executable]", 0, 1} 
};
#define NUM_FILES (sizeof(ramdisk) / sizeof(struct file))

void app_calculator() {
  char input[64];
  char num1_str[16], num2_str[16];
  char op;

  uart_puts("\n--- Simple Calc v1.0 ---\n");
  uart_puts("Format: [num] [op] [num] (e.g., 5 + 5)\n");
  uart_puts("Type 'exit' to quit.\n");

  while (1) {
    uart_puts("CALC> ");
    gets(input, 64);

    if (strcmp(input, "exit") == 0)
      return;

    int i = 0, j = 0;

    while (input[i] >= '0' && input[i] <= '9') {
      num1_str[j++] = input[i++];
    }
    num1_str[j] = '\0';

    while (input[i] == ' ')
      i++;

    op = input[i++];

    while (input[i] == ' ')
      i++;

    j = 0;
    while (input[i] >= '0' && input[i] <= '9') {
      num2_str[j++] = input[i++];
    }
    num2_str[j] = '\0';

    int n1 = atoi(num1_str);
    int n2 = atoi(num2_str);
    int res = 0;

    if (op == '+')
      res = n1 + n2;
    else if (op == '-')
      res = n1 - n2;
    else if (op == '*')
      res = n1 * n2;
    else if (op == '/') {
      if (n2 == 0) {
        uart_puts("Error: Div by 0\n");
        continue;
      }
      res = n1 / n2;
    } else {
      uart_puts("Syntax Error.\n");
      continue;
    }

    uart_puts("Result: ");
    putint(res);
    uart_puts("\n");
  }
}

void shell_loop(void) {
  char cmd[64];
  uart_puts("\nInteractive Shell Ready.\n");

  while (1) {
    uart_puts("OS> ");
    gets(cmd, 64);

    if (strcmp(cmd, "help") == 0) {
      uart_puts("Commands:\n");
      uart_puts("  ls      - List files in RamFS\n");
      uart_puts("  cat [f] - Print file content\n");
      uart_puts("  calc    - Launch Calculator App\n");
      uart_puts("  stats   - System stats\n");
      uart_puts("  clear   - Clear screen\n");
      uart_puts("  exit    - Shutdown OS\n");
    } else if (strcmp(cmd, "stats") == 0) {
      uart_puts("Task A: ");
      puthex(counter_a);
      uart_puts("\n");
      uart_puts("Task B: ");
      puthex(counter_b);
      uart_puts("\n");
    } else if (strcmp(cmd, "clear") == 0) {
      uart_puts("\033[2J\033[H");
    } else if (strcmp(cmd, "exit") == 0) {
      power_off();
    }
    else if (strcmp(cmd, "ls") == 0) {
      uart_puts("Files:\n");
      for (int i = 0; i < NUM_FILES; i++) {
        uart_puts("  ");
        uart_puts(ramdisk[i].name);
        if (ramdisk[i].is_executable)
          uart_puts("*");
        uart_puts("\n");
      }
    } else if (cmd[0] == 'c' && cmd[1] == 'a' && cmd[2] == 't' &&
               cmd[3] == ' ') {
      char *fname = cmd + 4;
      int found = 0;
      for (int i = 0; i < NUM_FILES; i++) {
        if (strcmp(ramdisk[i].name, fname) == 0) {
          uart_puts(ramdisk[i].content);
          uart_puts("\n");
          found = 1;
          break;
        }
      }
      if (!found)
        uart_puts("File not found.\n");
    }
    else if (strcmp(cmd, "calc") == 0) {
      app_calculator();
    } else if (cmd[0] == '\0') {
    } else {
      uart_puts("Unknown command: ");
      uart_puts(cmd);
      uart_puts("\n");
    }
  }
}
