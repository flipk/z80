
#define SIZE 32

	.text
main:
.public main
	ld bc,SIZE
	push bc
	ld a,0x45
	push af
	ld bc,buf1
	push bc
	call memset

	ld bc,SIZE
	push bc
	ld bc,buf2
	push bc
	ld bc,buf1
	push bc
	call memcpy

	ret

	.bss
buf1:
	.space SIZE
buf2:
	.space SIZE

        .data
string2:
.public string2
        .ascii "jingle"
        .byte 0
        .asciiz "jangle"
        .word 5,6,7

