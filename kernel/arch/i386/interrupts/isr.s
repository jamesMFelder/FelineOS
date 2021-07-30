extern exception_handler ;The C function for handling interrupts

%macro isr_err_stub 1

isr_stub_%+%1:
    cli ;Disable interrupts
    pusha ;Save registers
  
    push dword %1 ;Add the exeption number to the stack
    call exception_handler ;Call the generic exception handler (written in C)
    pop eax ;Take the exception number off the stack
    
    hlt ;Halt the computer until we know how to safely continue

    popa ;Restore registers
    add esp, 8 ;Restore the stack to how it was before the interrupt
    iret  ;Return from the interrupt
%endmacro
; if writing for 64-bit, use iretq instead

%macro isr_no_err_stub 1
isr_stub_%+%1:
    cli ;Disable interrupts
    pusha ;Save registers

    push dword 0 ;Push 0 as an exception number because we don't have one
    push dword %1 ;Add the exeption number to the stack
    call exception_handler ;Call the generic exception handler (written in C)
    pop eax ;Take the exception number off the stack
    pop eax ;Take the dummy exception off the stack
    
    hlt ;Halt the computer until we know how to safely continue

    popa ;Restore registers
    add esp, 8 ;Restore the stack to how it was before the interrupt
    iret ;Return from the interrupt
%endmacro

;List of interrupt service routines
isr_no_err_stub 0
isr_no_err_stub 1
isr_no_err_stub 2
isr_no_err_stub 3
isr_no_err_stub 4
isr_no_err_stub 5
isr_no_err_stub 6
isr_no_err_stub 7
isr_err_stub    8
isr_no_err_stub 9
isr_err_stub    10
isr_err_stub    11
isr_err_stub    12
isr_err_stub    13
isr_err_stub    14
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
isr_no_err_stub 31

;Used for the lidt instruction
global isr_stub_table
isr_stub_table:
%assign i 0 
%rep    32 
    dd isr_stub_%+i ; use DQ instead if targeting 64-bit
%assign i i+1 
%endrep
