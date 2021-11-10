; SPDX-License-Identifier: MIT
; Copyright (c) 2021 James McNaughton Felder

;Make strings easier to create.
;    Just type `string name, "value"`.
;        Note the comma.
;    Automatically null-terminates the strings.
%macro string 2
%1 db %2, 0
%endmacro

