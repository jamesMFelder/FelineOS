/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cinttypes>
#include <cstdlib>
#include <kernel/asm_compat.h>
#include <kernel/halt.h>
#include <kernel/log.h>

ASM [[noreturn]] void handle_abort(void *executing_address,
                                   uint32_t fault_status, void *fault_address);

enum fault_causes {
	tlb_miss = 0b0000,
	alignment = 0b0001,
	debug_event = 0b0010,
	access_bit_section = 0b0011,
	instruction_cache = 0b0100,
	translation_fault_section = 0b0101,
	access_bit_page = 0b0110,
	translation_fault_page = 0b0111,
	precise_external_abort = 0b1000,
	domain_fault_section = 0b1001,
	reserved = 0b1010,
	domain_fault_page = 0b1011,
	translation_first_level = 0b1100,
	permission_error_section = 0b1101,
	translation_second_level = 0b1110,
	permission_error_page = 0b1111,
};

/* Handle an ARM data abort */
void handle_abort(void *executing_address, uint32_t fault_status,
                  void *fault_address) {
	/* The reason is encoded in DFSR[10,3:0] (seriously) */
	constexpr uint32_t FAULT_CAUSE_MASK = 0b1111;
	auto cause = fault_status & FAULT_CAUSE_MASK;

	constexpr uint32_t DFSR_EXTRA_BIT_POSITION = 10;
	if (fault_status & (1 << DFSR_EXTRA_BIT_POSITION)) {
		constexpr uint32_t IMPRECISE_DATA_ABORT = 0b0110;
		if (cause == IMPRECISE_DATA_ABORT) {
			kcriticalf("Imprecise data abort, halting!");
		} else {
			kcriticalf("Reserved abort status %#" PRIb32
			           " with DFSR[10]=1 occured, halting!",
			           cause);
		}
		halt();
	}
	/* Go through the causes
	 * (ordered by priority according to the technical reference manual
	 * (page 351 or 6-34), not counting ones with DFSR[10]=1) */
	switch (cause) {

	case fault_causes::alignment:
		/* Alignment Issue */
		kerrorf("Instruction %p attempted to access unaligned address %p",
		        executing_address, fault_address);
		/* TODO: can we fix it? */
		halt();

	case fault_causes::tlb_miss:
		/* TLB miss */
		kcritical(
			"I'm unsure if this is supposed to be a TLB miss abort or not "
			"happening. Please fix if you encounter this. Halting now.");
		halt();

	case fault_causes::instruction_cache:
		/* Instruction Cache Maintainance Fault (TODO: flush instruction cache?)
		 */
		kcriticalf("Instruction cache corrupted! TODO: fix it! (note: "
		           "exception occured while executing %p)",
		           executing_address);
		halt();
		// break;

	case translation_first_level:
		/* External Abort on Translation on first level */
	case translation_second_level:
		/* External Abort on Translation on second level */
		kcriticalf("External abort attempting to access %p (reason = %#" PRIb32
		           "). Halting!",
		           fault_address, cause);
		halt();
	case fault_causes::translation_fault_section:
		/* Translation Fault on section */
	case fault_causes::translation_fault_page:
		/* Translation Fault on page */
		kcriticalf(
			"Translation fault attempting to access %p (reason = %#" PRIb32
			"). Halting!",
			fault_address, cause);
		std::abort();
	case fault_causes::access_bit_section:
		/* Access Bit Fault, Force AP Only (permission denied?) on section
		 * (TODO: what?) */
	case fault_causes::access_bit_page:
		/* Access Bit Fault, Force AP Only (permission denied?) on page (TODO:
		 * what?) */
		kcriticalf(
			"Permission denied attempting to access %p (reason = %#" PRIb32
			"). Halting!",
			fault_address, cause);
		halt();
	case fault_causes::domain_fault_section:
		/* Domain Fault on section */
	case fault_causes::domain_fault_page:
		/* Domain Fault on page */
		kcriticalf("Domains not yet implimented, so abort status %#" PRIb32
		           " should not be possible. Halting!",
		           cause);
		halt();
	case permission_error_section:
		/* Permission Error on section */
	case permission_error_page:
		/* Permission Error on page */
		kcriticalf("Permission error: abort status %#" PRIb32
		           " with DFSR[10]=0. Halting!",
		           cause);
		halt();

	case fault_causes::precise_external_abort:
		/* Precise External Abort */
		kerrorf("Instruction %p aborted trying to access %p", executing_address,
		        fault_address);
		std::abort();
		// return;

	case fault_causes::debug_event:
		/* Instruction Debug Event */
		kerrorf("Debug Event: this should probably switch to the debugger, but "
		        "just halting for now");
		halt();

	case reserved:
		kcriticalf("Reserved abort status %#" PRIb32
		           " with DFSR[10]=0 occured, halting!",
		           cause);
		halt();
	}
	__builtin_unreachable();
}
