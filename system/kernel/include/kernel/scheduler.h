/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#ifndef _KERN_SCHEDULER_H
#define _KERN_SCHEDULER_H

#include <kernel/task.h>

/* Set-up the scheduler so it knows that we are the current task */
void init_scheduler();
/* Switch to a different task, and return when the current task gets
 * re-scheduled */
void sched();
/* Create a new task that will run func, and make it schedule-able. */
void add_new_task(init_task func);
/* Switch to a different task and do not let this one be scheduled again. */
[[noreturn]] void end_cur_task();
/* Release resources from terminated task. */
void cleanup_finished_tasks();
/* Stuff to do when a timer interrupt occurs */
void scheduler_handle_tick();

#endif // _KERN_SCHEDULER_H
