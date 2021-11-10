; SPDX-License-Identifier: MIT
; Copyright (c) 2021 James McNaughton Felder

extern syscall

;Make a system call
;Note that any function can do this, so we don't do security, just convenience
syscall:
	push ebp ;Create a stack frame (for debugging)
	mov ebp, esp

	mov eax, [ebp+4] ;Move the first argument to eax (to select the syscall)

	cmp eax, 0 ;Compare against the max syscall number (0 because we don't support any)
	jle valid
	mov eax, -1 ;Return -1
	jmp done ;Jump to cleanup
	valid:

	int 0x1F ;Call the kernel
	;Return value is in eax, where it should be

	done: ;The syscall is over (used for errors in arguments)

	mov esp, ebp ;Remove the stack frame
	pop ebp

	ret ;Return to the calling function
