```
Create(int priority, ((void*)func()));
```
`priority`: the priority of the task / process
`func()`: the function in which the program is mean to run

SNV "N": ARM

==What happens when SVC is executed?==
- record exception code and N in ESR EL1 <- system register
- record next PC in ELR_EL1
	- If we do not save the PC we do not know where we are when we did the context switch
- record PSTATE in SPSR_EL1
- switch processor exception label to EL1
- switch to use EL1 stockpointer
- sets PC to address determined VBR_EL1
- 
The hardware remembers for us. PC is the program counter which records where in the program are we. 

VBR_EL1 points a to an exception vecotor
