SPSR_ELn: Saved Program Status Register
- Exists as:
	- SPSR_EL1
	- SPSR_EL2
	- SPSR_EL3
ELR_ELn: Exception link register holds the address of the instruction which caused the aborting data access. 
# The Exception Syndrome Register, $ESR_{ELn}$
contains information which allows the exception handler to determine the reason for the exception. It is updated only for synchronous exceptions and SError. It is not updated for IRQ or FIQ as these interrupt handlers typically obtain status information from registers in the Generic Interrupt Controller (GIC). (See The Generic Interrupt Controller on page 10-17.) 
The bit coding for the register is:
- Bits [31:26] of ESR_ELn indicate the exception class which allows the handler to distinguish between the various possible exception causes (such as unallocated instruction, exceptions from MCR/MRC to CP15, exception from FP operation, SVC, HVC or SMC executed, Data Aborts, and alignment exceptions).
- Bit [25] indicates the length of the trapped instruction (0 for a 16-bit instruction or 1 for a 32-bit instruction) and is also set for certain exception classes.
- Bits [24:0] form the Instruction Specific Syndrome (ISS) field containing information specific to that exception type. For example, when a system call instruction (SVC, HVC or SMC) is executed, the field contains the immediate value associated with the opcode such as 0x123456 for SVC 0x123456

## $FAR_{ELn}$: Fault Address Register
![[Pasted image 20230925154533.png]]
![[Pasted image 20230925154554.png]]


![[Pasted image 20230925160858.png]]

# $SPSR$ Saved Program Status Register
When taking an exception, the processor state is stored in the relevant *Saved Program Status Register* (SPSR), in a similar way to the CPSR in ARMv7. The SPSR holds the value of `PSTATE` before taking an exception and is used to restore the value of `PSTATE` when executing an exception return.
![[Pasted image 20230925194917.png]]
