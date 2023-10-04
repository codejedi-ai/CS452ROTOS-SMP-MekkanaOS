```
void kmain(){
	initialize();
	for(;;){
		cur_task=schedule();
		request = activate(cur_task);
		handel(request);
	}
}
```

`activate(cur_task)` is the context switch that switches away from the kernel context. Wake up in this loop in the exact process.
# Kernel View 2

- Kernel is
	- Boot code
		- similar to `initialize()
		- create and activates first task
	- Exception handler
		- Similar to `handel()` + `schedule` + `activate()`
# Difference between View 1 and View 2
Always have context weather or not I initialized it from scratch or use the pre-switch context. The kernel in View #2 cannot 


For the first kernel assignment I can definitely structure my kernel code as such. The first thing that would start running is the kernel. `initialize()` initializes data structure the kernel uses Data structures that keep track of what are the tasks that the kernel has created so that it can decide how to switch up on them when it gets around to schedule inside the current task structures if you have several tasks information about each of the test that's r association the kernel is going to create a structure to represent March so they'll be this structure that it's that it creates. They'll be kind of one structured one task that the kernel knows about OK but hasn't launched yet after we finished set it up so we know how we're going to launch it but we haven't , so that's initialization OK where are you at? OK I'm gonna choose a task to run. Choose a test switched schedule function as a scheduler and it's going to decide which task should run next so in the kernel that your building tasks will have priorities OK and sometimes some of the task will be blocked because they're waiting for some event and other tasks won't be bl and ready to go in normally what the schedule is going to do is it's going to choose a task that's not blocked and probably wants to choose the highest priority task and it's going to decide that's the task that I want to run next OK whatever decision the schedule it's gonna return his choice so after I do initialize I really only have one task in the system so the scheduler will take that task as the next task to run, and once the schedule has chosen a task, magic, activation function function with gonna start that cast running to the kernel all activate and inside of activate their death, the kernel will invoke its cold that causes a contact switch from the kernel to that task activate enable essentially fall asleep inside of activate. OK when we switch the contacts from the kernel to the user test, happens up here, control, cars, activate and will contact switch and install T ones contacts in the processor and this test will start running OK and while this task is running, the kernel is essentially stopped and saved and contacts inside of acti, this thing could run forever that's all that would happen is LeBron forever but eventually while this task is running an exceptional walker, so a timer interrupt will happen, or perhaps this task will make a call exception, happens control switch to the kernel, and the kernel will save this task, contacts and restore its own context, context, the context of whatever the context was rec so when this thing here kernels gonna wake up inside of activate OK and inside of activate, it's going to look in some registers I'll show you this later and I will find out. Why did I wake up wasn't an exception or a system call OK and it will return from activate with a Okuda request that says Wyatt woke up. Why do we come back here and then the kernel will handle it OK so maybe what reason that I woke back up and came into the kernel of insanity create another text control go back to the turn off the carnival wake up in activate come out of activate and say the request is pl I will run the handle or see what was the request it was to create another task so I will create a structure for another task. I'll initialize all the information in there OK I'm on the handler start doing that we go back to the top schedule ones again and says which of these past should run next maybe the colone one and the colonel will reactivate per task in the context switch what happens on the phone back to tasks or maybe the scheduler will decide it is brand new task teach you should run in that case activating to contact switch will be running OK so this just goes on forever colonel chooses a task activates. It sleeps essentially inside of activate until an exception occurs with us back up and activate learns by it was guns what the exception was handles the exception chooses the next task to Ryan that's the life of a current believe about how the contact switching happens, but that's kind of a