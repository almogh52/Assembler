stop

W: .entry W
HE: .extern WDF

W: .data 5

SHF: .data 5, 7, 151, 84
STR: .string "HELLO THERE"

mov r0, WDF
add #5, r0
sub #4, r0
prn r2
sub #1, WDF
; Some comment here

STRC: .struct 51, "Something here"
