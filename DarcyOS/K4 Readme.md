
Kernel description
IO server:
![[Pasted image 20231030175847.png]]
I have a UART interrupt Notifier that would send commands to the UART server whenever there is an interrupt.


Added interrupts to the handler. Added the RXIC, TXIC and CTX interrupts to the handler in addition to a queue that would catch missed interrupts. 

The UART server also have similar mechanisms to catch missed interrupts in a queue. Since I am not fond of using delays I would opt to use a queue. 


RXIC server:
this server imitates the FIFO queue that I do not have the luxury to enjoy. 