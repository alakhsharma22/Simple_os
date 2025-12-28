#include "mm.h"
#include <stdint.h>

// MMU Page Tables (Aligned to 4KB)
__attribute__((aligned(4096))) uint64_t L1_Page_Table[512];

void mmu_init(void) {
  // 1. Identity Map Device Memory (0x00000000 - 0x3FFFFFFF)
  // AttrIndex=1 (Device), AccessFlag=1, Block Entry
  L1_Page_Table[0] = (0x00000000) | (1 << 10) | 0x1 | (1 << 2);

  // 2. Identity Map Normal Memory (0x40000000 - 0x7FFFFFFF)
  // AttrIndex=0 (Normal), AccessFlag=1, Block Entry
  L1_Page_Table[1] = (0x40000000) | (1 << 10) | 0x1 | (0 << 2);

  // 3. Set MAIR Attributes (Attr0 = Normal, Attr1 = Device)
  asm volatile("msr mair_el1, %0" ::"r"((uint64_t)0x00FF));

  // 4. Set TCR (Translation Control)
  asm volatile("msr tcr_el1, %0" ::"r"((uint64_t)25 | (0 << 14)));

  // 5. Set TTBR0 (Page Table Base)
  asm volatile("msr ttbr0_el1, %0" ::"r"(L1_Page_Table));

  // 6. Enable MMU and Caches via SCTLR
  uint64_t sctlr;
  asm volatile("mrs %0, sctlr_el1" : "=r"(sctlr));
  sctlr |= 1;         // M bit (MMU)
  sctlr |= (1 << 2);  // C bit (D-Cache)
  sctlr |= (1 << 12); // I bit (I-Cache)

  asm volatile("msr sctlr_el1, %0" ::"r"(sctlr));
  asm volatile("isb"); // Instruction Barrier
}
