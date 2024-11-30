/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#ifndef KERNEL_TASK_H
#define KERNEL_TASK_H

#include <cstdint>
#include <feline/kallocator.h>
#include <feline/kvector.h>
#include <feline/logger.h>
#include <kernel/asm_compat.h>
#include <kernel/mem.h>

/* The type of a function that starts a new task. NOTE: [[noreturn]]
 * is not supported on typedefs, and is not equivalent to the attribute (why?),
 * so you have to declare the lambda or function with __attribute__((noreturn)).
 * Calling end_cur_task() is the supported way to finish, but any [[noreturn]]
 * function will also work. */
typedef __attribute__((noreturn)) void (*init_task)();
/* This function safely leaves the scheduler before calling the start_executing
 * function. Things should be setup so that when switch_task returns to it,
 * start_executing is in the right place to be seen as an argument. */
[[noreturn]] void exit_scheduler_stub(init_task start_executing);

enum task_state {
	runnable,
	finished,
};

/* TODO: change this when supporting 64-bit systems*/
typedef uint32_t reg;

#if defined(__i386__)

struct x86GeneralRegisters {
		reg eax;
		reg ebx;
		reg ecx;
		reg edx;
		reg esi;
		reg edi;
		reg ebp;
		reg esp;
};

struct x86Registers {
		x86GeneralRegisters general;
		reg eflags;
};

typedef x86Registers Registers;

#elif defined(__arm__)

struct ARMGeneralRegisters {
		reg r0;
		reg r1;
		reg r2;
		reg r3;
		reg r4;
		reg r5;
		reg r6;
		reg r7;
		reg r8;
		reg r9;
		reg r10;
		reg r11;
		reg r12;
		reg r13;
		reg r14;
};
struct ARMRegisters {
		ARMGeneralRegisters general;
};
typedef ARMRegisters Registers;
#else
#error "Unknown architecture! Can't figure how to layout process structures!"
#endif

/* TODO: support different address spaces */
struct TaskAllocation {
		void *addr;
		size_t len;
};

struct Task {
		Registers registers;
		task_state state;
		size_t num_times_scheduled;
		KVector<TaskAllocation, KGeneralAllocator<TaskAllocation>> allocations;
		// TODO: add threads
};

Task create_new_task(init_task start_executing);

/* TODO: allocate guard pages, and place the stack where it can be expanded */
TaskAllocation create_new_stack(size_t len = 16_KiB);

/* Load the next process's and return to it. Will return from this function when
 * switching back to this process. This is a very low-level swap which only
 * swaps the registers. Do not call outside of end_cur_task() or sched() */
ASM void swap_task_registers(Registers *current, Registers *next);

/* Creates a new Task that is ready to be switched to (but does not actually
 * switch to it). */
Task create_new_task(init_task start_executing);

#endif // KERNEL_TASK_H
