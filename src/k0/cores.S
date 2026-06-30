/*
 * Per-core entry for SMP multikernel.  QEMU raspi4b releases cores 1–3 from
 * spin-table mailboxes 0xE0 / 0xE8 / 0xF0 into secondary_entry.
 */

#define SCTLR_RESERVED (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11)
#define USER_MASK_ACCESS (1 << 9)
#define SCTLR_WFE_WFI_ENABLED (1 << 18) | (1 << 16)
#define SCTLR_VALUE_MMU_DISABLED (SCTLR_RESERVED | USER_MASK_ACCESS | SCTLR_WFE_WFI_ENABLED)
#define HCR_RW (1 << 31)
#define SPSR_MASK_ALL (7 << 6)
#define SPSR_EL1      (5 << 0)
#define SPSR_VALUE (SPSR_MASK_ALL | SPSR_EL1)

.section ".text"
.align 4

.global secondary_park
secondary_park:
	msr DAIFSet, #0xf
1:	wfi
	b	1b

.global secondary_entry
secondary_entry:
	msr DAIFSet, #0xf

	mrs x0, mpidr_el1
	and x0, x0, #0xff

	mrs x1, CurrentEL
	and x1, x1, #8
	cbz x1, sec_el1_entry

	ldr x2, =HCR_RW
	msr hcr_el2, x2
	ldr x3, =SPSR_VALUE
	msr spsr_el2, x3
	adr x4, sec_el1_entry
	msr elr_el2, x4
	eret

sec_el1_entry:
	mrs x5, S3_1_c15_c2_1
	orr x5, x5, #(1 << 6)
	msr S3_1_c15_c2_1, x5
	isb

	ldr x2, =SCTLR_VALUE_MMU_DISABLED
	msr sctlr_el1, x2

	mrs x0, mpidr_el1
	and x0, x0, #0xff

	ldr x1, =__cpu_stack_base
	mov x2, #0x4000
	mul x3, x0, x2
	add sp, x1, x3

	ldr x4, =vector_table_el1
	msr VBAR_EL1, x4
	isb

	msr DAIFClr, #0x4

	bl	smp_secondary_kmain

2:	wfi
	b	2b
