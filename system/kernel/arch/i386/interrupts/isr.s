; SPDX-License-Identifier: MIT
; Copyright (c) 2021 James McNaughton Felder
extern kcriticalf ;Log an critical error printf(3) style
extern kerrorf ;Log an error printf(3) style
extern klogf ;Logging printf(3) style
extern abort

%macro isr_err_stub 1

isr_stub_%+%1:
	push ebp ;Create a stack frame for debugging
	mov ebp, esp

    push eax ;Save registers
	push ecx
	push edx
	cld ;Reset string operation direction flag before calling C code

	push dword [ebp+8] ;Push the error code for the logging function
    push dword %1 ;Push the error code for the logging function
	push error_msg_generic_interrupt
    call kerrorf ;Error because we don't know what to do

	push interrupt_corrupted_stack_trace ;Apologize for the broken stack trace
	call kerrorf

	push DWORD [ebp+4] ;Push the faulting instruction
	push error_msg_generic_eip
	call kerrorf

	hlt ;Halt because we don't know what to do

	add esp, 12 ;Get rid of local variables

    pop edx ;Restore registers we pushed
	pop ecx
	pop eax
    add esp, 8 ;Restore the stack to how it was before the interrupt
    iret  ;Return from the interrupt
%endmacro
; if writing for 64-bit, use iretq instead

%macro isr_no_err_stub 1
isr_stub_%+%1:
	push ebp ;Create a stack frame for debugging
	mov ebp, esp

    push eax ;Save registers
	push ecx
	push edx

    push dword %1 ;Add the exeption number to the stack
	push noerror_msg_generic_interrupt
    call kerrorf ;Error because we don't know what to do

	push DWORD [ebp+4] ;Push the faulting instruction
	push error_msg_generic_eip
	call kerrorf

	add esp, 8 ;Remove local variables

    hlt ;Halt because we don't know what to do

    pop edx ;Restore registers
	pop ecx
	pop eax

	mov esp, ebp ;Remove the stack frame
	pop ebp

    iret ;Return from the interrupt
%endmacro

;List of interrupt service routines
;#DE (Division by Zero)
isr_stub_0:
	push ebp ;Create a stack frame (for debugging)
	mov ebp, esp
	push eax ;Save registers
	push ecx
	push edx
	cld ;Reset string operation direction flag before calling C code
	push DWORD [ebp+4] ;put the fauling address lower on the stack (for logging)
	;Change to `push QWORD [ebp+8]` for 64-bit
	push error_msg_division_by_zero ;The format string
	call kerrorf
	call abort ;Hang
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
;#DF (Double fault)
;99% unrecoverable
isr_stub_8:
	push ebp ;Create a stack frame (for debugging)
	mov ebp, esp
	push eax ;Save registers
	push ecx
	push edx
	cld ;Reset string operation direction flag before calling C code
	push error_msg_double_fault
	call kcriticalf
	push interrupt_corrupted_stack_trace ;Apologize for the broken stack trace
	call kcriticalf
	call abort ;Hang
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
;#GPF(General Protection Fault)
isr_stub_13:
	push ebp ;Create a stack frame (for debugging)
	mov ebp, esp
	push eax ;Save registers
	push ecx
	push edx

	cld ;Reset string operation direction flag before calling C code

	push DWORD [ebp+8] ;put the fauling address lower on the stack (for logging)
	;Change to `push QWORD [ebp+8]` for 64-bit
	push error_msg_GPF ;The format string
	call kerrorf

	mov eax, [ebp+4] ;Check if we have a relevant segment selector index
	cmp eax, 0
	je gpf_end ;If we don't (it's 0), jump to the abort call

	push eax ;put the segment selector index on the stack (for logging)
	push error_msg_GPF_segment ;Format string if we have a segment selector index
	call kerrorf

	push interrupt_corrupted_stack_trace ;Apologize for the broken stack trace
	call kerrorf

	gpf_end: ;Jump here to skip printing a non-relevant segment selector index
	call abort ;Hang
;#PF (Page Fault)
isr_stub_14:
	push ebp ;Create a stack frame
	mov ebp, esp
	push eax ;Save registers
	push ecx
	push edx
	cld ;Reset string operation direction flag before calling C code

	mov eax, cr2
	push eax ;Push the invalid address

	;Change to `push QWORD [ebp+16]` for 64-bit
	push DWORD [ebp+8] ;Push the faulting address

	mov eax, [ebp+4] ;Check if this is due to a missing page, or invalid access to a page
	and eax, 1
	cmp eax, 0
	jne push_invalid_access ;The page table is present

	push log_msg_page_notpresent ;The page table isn't present
	jmp pg_msg_pushed ;Don't push the other message

	push_invalid_access: ;The page table is present
	push log_msg_page_access_violation

	pg_msg_pushed:
	call klogf ;Actually print it
	push interrupt_corrupted_stack_trace ;Apologize for the broken stack trace
	call kerrorf
	call abort ;We didn't actually fix anything

	mov esp, ebp
	pop ebp
	iret
isr_no_err_stub 15
isr_no_err_stub 16
isr_err_stub    17
isr_no_err_stub 18
isr_no_err_stub 19
isr_no_err_stub 20
isr_no_err_stub 21
isr_no_err_stub 22
isr_no_err_stub 23
isr_no_err_stub 24
isr_no_err_stub 25
isr_no_err_stub 26
isr_no_err_stub 27
isr_no_err_stub 28
isr_no_err_stub 29
isr_err_stub    30
;System call (for now)
isr_stub_31:
	push ebp ;Create a stack frame (for debugging)
	mov ebp, esp

	push ecx ;Save registers
	push edx
	cld ;Reset string operation direction flag before calling C code

	push eax ;Push the syscall number
	push temp_syscall_error_msg ;The format string
	call klogf ;Error because we don't support anything yet

	call abort ;Quit

	mov eax, 0 ;Return success

	add esp, 1 ;Remove local variables
	pop edx ;Restore registers
	pop ecx

	mov esp, ebp ;Remove the stack frame
	pop ebp

	iret ;Return to the next instruction

;Used for the lidt instruction
global isr_stub_table
isr_stub_table:
%assign i 0
%rep    32
    dd isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1
%endrep
section .rodata

%include "string.inc"

;Change %lX to %llX for 64-bit
	string interrupt_corrupted_stack_trace, "Replace the address that caused the fault in the stack trace below with the address shown here when you are debugging."
	string temp_syscall_error_msg, "Unknown system call %ld made."
	string error_msg_generic_interrupt, "Unknown interrupt 0x%lX fired with error 0x%lX."
	string noerror_msg_generic_interrupt, "Unknown interrupt 0x%lX fired"
	string error_msg_generic_eip, "The instruction was at %p."
	string error_msg_division_by_zero, "Division by zero at %p."
	string error_msg_double_fault, "A double fault occured. Halting."
	string error_msg_GPF, "General protection fault at %p."
	string error_msg_GPF_segment, "Segment selector index: 0x%lX."
	string log_msg_page_notpresent, "Instruction at %p attempted to access unmapped page at %p."
	string log_msg_page_access_violation, "Instruction at %p is not allowed to access to address %p."
