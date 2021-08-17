global setGdt

gdtr DW 0 ; For limit storage
     DD 0 ; For base storage

setGdt:
   CLI ;Disable interrupts
   MOV   EAX, [esp + 4]
   MOV   [gdtr + 2], EAX
   MOV   AX, [ESP + 8]
   MOV   [gdtr], AX
   LGDT  [gdtr]

   ; Reload CS register containing code selector:
   JMP   0x08:.reload_CS ; 0x08 points at the new privliged code selector


.reload_CS:
   ; Reload data segment registers:
   MOV   AX, 0x10 ; 0x10 points at the new privliged data selector
   MOV   DS, AX
   MOV   ES, AX
   MOV   FS, AX
   MOV   GS, AX
   MOV   SS, AX
   RET
