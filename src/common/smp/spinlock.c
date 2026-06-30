#include "spinlock.h"

void spinlock_init(spinlock_t *lock)
{
	*lock = SPINLOCK_INIT;
	__asm__ volatile("dmb ish" ::: "memory");
}

/*
 * AArch64 spinlock: LDAXR / STXR acquire, STLR release.
 * WFE while contended; SEV on unlock.  Requires cluster coherency
 * (boot.S / cores.S enable S3_1_c15_c2_1 bit 6).
 */
void spinlock_lock(spinlock_t *lock)
{
	uint32_t tmp;
	uint32_t one = 1u;

	for (;;) {
		__asm__ volatile(
		    "	sevl\n"
		    "1:	wfe\n"
		    "	ldaxr	%w0, %2\n"
		    "	cbnz	%w0, 1b\n"
		    "2:	stxr	%w0, %w1, %2\n"
		    "	cbnz	%w0, 2b"
		    : "=&r"(tmp)
		    : "r"(one), "Q"(*lock)
		    : "memory");
		return;
	}
}

int spinlock_trylock(spinlock_t *lock)
{
	uint32_t tmp;
	uint32_t one = 1u;
	int busy;

	__asm__ volatile(
	    "	ldaxr	%w0, %2\n"
	    "	cbnz	%w0, 1f\n"
	    "	stxr	%w0, %w1, %2\n"
	    "	cbnz	%w0, 1f\n"
	    "	mov	%w3, #0\n"
	    "	b	2f\n"
	    "1:	mov	%w3, #1\n"
	    "2:"
	    : "=&r"(tmp), "=r"(busy)
	    : "Q"(*lock), "r"(one)
	    : "memory");

	return busy ? -1 : 0;
}

void spinlock_unlock(spinlock_t *lock)
{
	__asm__ volatile(
	    "	stlr	wzr, %0\n"
	    "	dmb ish\n"
	    "	sev"
	    : "=Q"(*lock)
	    :
	    : "memory");
}
