#ifndef GIC_H
#define GIC_H

#include <stdint.h>

void gic_init(void);
void handle_irq(void);
void handle_sync(void);
void gic_eoi(uint32_t id);

#endif
