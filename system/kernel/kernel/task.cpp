/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#include <kernel/task.h>

TaskAllocation create_new_stack(size_t len) {
	void *stack;
	if (get_mem(&stack, len) != mem_success) {
		kCriticalNoAlloc() << "Unable to allocate new stack!";
		std::abort();
	}
	return TaskAllocation{.addr = stack, .len = len};
}

#ifdef __i386__
/* Creates a stack frame matching the first half of swap_process_registers at
 * esp_top, and then returns. The frame is set-up to return to the start of
 * exit_scheduler_stub, with start_executing placed above it as an argument. The
 * existing stack is used and reset normally. */
ASM reg init_frame(reg esp_top, void (*exit_scheduler_stub)(init_task),
                   init_task start_executing);

/* Gets the current eflags register value. Used to set-up sane defaults until we
 * have explicit choices for each flag. */
ASM reg get_eflags();

Task create_new_task(init_task start_executing) {
	auto task = Task{};
	/* Allocate the stack, and create the topmost stack frame. */
	auto stack = create_new_stack();
	task.allocations.push_back(stack);
	/* set-up the stack so exit_scheduler_stub calls start_executing when it
	 * first gets scheduled */
	task.registers.general.ebp =
		init_frame(reinterpret_cast<uintptr_t>(stack.addr) + stack.len,
	               exit_scheduler_stub, start_executing);
	/* Use our current eflags (TODO: should there be explicit defaults) */
	task.registers.eflags = get_eflags();
	task.state = runnable;
	return task;
}
#else  // __i686__
Task create_new_task(init_task start_executing) {
	auto task = Task{};
	auto stack = create_new_stack();
	task.allocations.push_back(stack);
	task.registers.general.r13 =
		reinterpret_cast<uintptr_t>(stack.addr) + stack.len;
	/* "return" to exit_scheduler_stub */
	task.registers.general.r14 =
		reinterpret_cast<uintptr_t>(exit_scheduler_stub);
	/* and pass it start_executing as an argument */
	task.registers.general.r0 = reinterpret_cast<uintptr_t>(start_executing);
	task.state = runnable;
	return task;
}
#endif // __i686__ (else)
