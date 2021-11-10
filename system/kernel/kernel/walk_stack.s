; SPDX-License-Identifier: MIT
; Copyright (c) 2021 James McNaughton Felder

; Walks backwards through the call stack and builds a list of return addresses.
; Args:
;  * Array of 32-bit addresses.
;  * Maximum number of elements in array.
; Return value: The number of addresses stored in the array.
; Calling convention: cdecl
[global walk_stack]
walk_stack:
    ; Create stack frame & save caller's EDI and EBX.
    push ebp
    mov  ebp,       esp
	push edi
	push ebx

    ; Set up local registers.
    xor  eax,       eax         ; EAX = return value (number of stack frames found).
    mov  ebx,       [ebp]  ; EBX = old EBP.
    mov  edi,       [ebp + 8]  ; Destination array pointer in EDI.
    mov  ecx,       [ebp + 12]  ; Maximum array size in ECX.

.walk:
    ; Walk backwards through EBP linked list, storing return addresses in EDI array.
    test ebx,       ebx
    jz   .done
    mov  edx,       [ebx +  4]  ; EDX = previous stack frame's IP.
    mov  ebx,       [ebx +  0]  ; EBX = previous stack frame's BP.
	sub  edx, 1                 ; We got the IP of the next instruction
    mov  [edi],     edx         ; Copy IP to the array.
    add  edi,       4
    inc  eax
    loop .walk ;Repeat if ecx isn't zero

.done:
    ; Restore caller's EDI and EBX, leave stack frame & return EAX.
    pop ebx
    pop edi
    mov esp, ebp
	pop ebp
    ret
