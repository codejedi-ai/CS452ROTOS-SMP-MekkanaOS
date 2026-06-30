#include "smp.h"
#include "asm.h"

volatile uint32_t smp_boot_ready = 0;

static inline void smp_dmb(void)
{
	__asm__ volatile("dmb ish" ::: "memory");
}

static inline void smp_dsb(void)
{
	__asm__ volatile("dsb sy" ::: "memory");
}

void smp_set_core_id(uint32_t core_id)
{
	__asm__ volatile("msr tpidr_el1, %0" :: "r"((uint64_t)core_id));
}

uint32_t smp_get_core_id(void)
{
	uint64_t core_id;
	__asm__ volatile("mrs %0, tpidr_el1" : "=r"(core_id));
	return (uint32_t)core_id;
}

void smp_init_cpu(void)
{
	uint64_t mpidr;
	__asm__ volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
	smp_set_core_id((uint32_t)(mpidr & 0xffu));
}

void smp_wait_boot_ready(void)
{
	while (!smp_boot_ready)
		__asm__ volatile("wfe");
	smp_dmb();
}

void smp_signal_boot_ready(void)
{
	smp_boot_ready = 1;
	smp_dsb();
	__asm__ volatile("sev");
}

void smp_boot_secondary_cores(void)
{
	extern void secondary_entry(void);
	uint64_t entry = (uint64_t)(uintptr_t)secondary_entry;

	*(volatile uint64_t *)0xE0 = entry;
	*(volatile uint64_t *)0xE8 = entry;
	*(volatile uint64_t *)0xF0 = entry;
	smp_dsb();
	__asm__ volatile("sev" ::: "memory");
}
