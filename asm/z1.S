
#define STACK_SIZE 128

; start
.text
	ld sp,stack+STACK_SIZE
	call main
	halt

	;; sp+2,3 = mem to clear
	;; sp+4,5 = byte to put there
	;; sp+6,7 = size to clear

memset:
.public memset
	pop ix
	pop hl
	push hl
	pop de
	pop af

	pop bc
	dec bc
	inc de
	ld (hl),a
	ldir
	jp (ix)

	;; sp+2,3 = destination
	;; sp+4,5 = source
	;; sp+6,7 = size

memcpy:
.public memcpy
	pop ix
	pop hl
	pop de
	pop bc
	ldir
	jp (ix)


	.bss
stack:
	.space STACK_SIZE

	.data
string:
.public string
	.ascii "crap"
	.byte 0,1,2,3
	.asciiz "junk"
	.byte 1,2,3,4
	.word 5,6,7
