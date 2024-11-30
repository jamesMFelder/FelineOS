/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#include <algorithm>
#include <cassert>
#include <feline/logger.h>
#include <feline/settings.h>
#include <feline/shortcuts.h>
#include <feline/spinlock.h>
#include <kernel/halt.h>
#include <kernel/mem.h>
#include <kernel/scheduler.h>
#include <kernel/task.h>

/* TODO: add some acquire only if available so we don't hang on recursive
 * acquires. */
Spinlock editing_task_list;

/* TODO: make cpu-local */
static Task current_task;

KVector<Task, KGeneralAllocator<Task>> all_tasks;

/* Returns the next process to run, or nullptr if no other processes are
 * runnable. TODO: do this faster and support priorities. */
decltype(all_tasks)::iterator find_next_task() {
	Task *next_task = nullptr;
	for (auto &p : all_tasks) {
		if (p.state == runnable) {
			if (!next_task ||
			    next_task->num_times_scheduled > p.num_times_scheduled) {
				next_task = &p;
			}
		}
	}
	return next_task;
}

void init_scheduler() {
	current_task = create_new_task([]() __attribute__((noreturn)) {
		kCritical() << "Initial task re-scheduled without having called sched!";
		halt();
	});
}

void sched() {
	/* If the scheduler was running when this interrupted it, don't do anything
	 * and just return so we can keep doing the task switch we were already
	 * doing. TODO: spin if another processor has the lock, instead of it being
	 * recursively taken. */
	if (!editing_task_list.try_acquire_lock()) {
		return;
	}
	auto next_task = find_next_task();
	if (!next_task) {
		/* If there is no other task to run, keep running this one. */
		editing_task_list.release_lock();
		return;
	}
	std::swap(current_task, *next_task);
	current_task.num_times_scheduled += 1;
	swap_task_registers(&next_task->registers, &current_task.registers);
	/* NOTE: we only returned here after being re-scheduled. This release the
	 * lock taken by another task. The lock taken at the beginning of this
	 * function was released by the next task that got scheduled. */
	editing_task_list.release_lock();
}

void add_new_task(init_task start_func) {
	editing_task_list.acquire_lock();
	all_tasks.push_back(create_new_task(start_func));
	editing_task_list.release_lock();
}

[[noreturn]] void end_cur_task() {
	/* NOTE: this lock is released by whatever task we switch to, so it's
	 * correct for it to look unlocked in this function. */
	editing_task_list.acquire_lock();
	auto next_task = find_next_task();
	if (!next_task) {
		/* TODO: support power-saving or something instead of running as an idle
		 * task. */
		kCritical() << "No more tasks to run, and current task ended. "
					   "Waiting for next interrupt.";
		editing_task_list.release_lock();
		halt();
	}
	current_task.state = finished;
	std::swap(current_task, *next_task);
	current_task.num_times_scheduled += 1;
	swap_task_registers(&next_task->registers, &current_task.registers);
	/* Since our state is finished, we can never return from switch_process, but
	 * the compiler doesn't know that */
	__builtin_unreachable();
}

void cleanup_finished_tasks() {
	/* Cleanup isn't (always) super important, so don't bother waiting if
	 * someone else has the lock. */
	if (!editing_task_list.try_acquire_lock()) {
		return;
	}
	for (auto task = begin(all_tasks); task != end(all_tasks);) {
		if (task->state == finished) {
			for (auto &allocation : task->allocations) {
				free_mem(allocation.addr, allocation.len);
			}
			all_tasks.erase(task);
		} else {
			++task;
		}
	}
	editing_task_list.release_lock();
}

/* This is "called" by being returned to from swap_process_registers. */
void exit_scheduler_stub(init_task start_executing) {
	editing_task_list.release_lock();
	start_executing();
}

/* This happens every time a timer interrupt occurs. TODO: should it just be run
 * on every interrupt? */
void scheduler_handle_tick() {
	/* TODO: only do when computer is idle or low on resources? */
	cleanup_finished_tasks();

	/* If it is a new second, print the time */
	static size_t second_since_boot = 0;
	if (Settings::Time::ns_since_boot.get() / 1'000'000'000 >
	    second_since_boot) {
		add_new_task([]() __attribute__((noreturn)) {
			kLog() << "It has been "
				   << dec(Settings::Time::ns_since_boot.get() / 1'000'000'000)
				   << " seconds since boot.";
			end_cur_task();
		});
		second_since_boot = Settings::Time::ns_since_boot.get() / 1'000'000'000;
	}

	/* Run a different process (possibly) */
	sched();
}
