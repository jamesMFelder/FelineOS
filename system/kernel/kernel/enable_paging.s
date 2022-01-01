; SPDX-License-Identifier: MIT
; Copyright (c) 2021 James McNaughton Felder

global enable_paging

enable_paging:
	push ebp
	mov ebp, esp

	;Load the page directory into cr3
	mov eax, [ebp+8]
	mov cr3, eax

	;Enable paging
	mov ebx, cr0
	or ebx, (1<<31)
	mov cr0, ebx

	; Now reload the segment registers (CS, DS, SS, etc.) with the appropriate segment selectors...
	mov ax, 0x10 ;Data selector
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; Reload CS with the code selector
	jmp 0x08:reloadCS ;Code selector

	reloadCS:
	mov eax, 0
	mov esp, ebp
	pop ebp
	ret
