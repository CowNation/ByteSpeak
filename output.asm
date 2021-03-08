section .text
	global _start ; tells linker the entry point
_start:
	mov	ecx, LINE_1 ; message to write
	mov	edx, 14 ; message length
	call print

	mov	ecx, LINE_2 ; message to write
	mov	edx, 14 ; message length
	call print

	mov	ecx, LINE_3 ; message to write
	mov	edx, 15 ; message length
	call print

	mov	ecx, LINE_4 ; message to write
	mov	edx, 27 ; message length
	call print

	mov	eax, 1 ; system call number (sys_exit)
	int	0x80 ; call kernel
print:
	mov	ebx, 1 ; file descriptor (stdout)
	mov	eax, 4 ; system call number (sys_write)
	int	0x80 ; call kernel
	ret

section .data
	LINE_1 db 72, 101, 108, 108, 111, 32, 87, 111, 114, 108, 100, 33, 13, 10
	LINE_2 db 74, 101, 108, 108, 111, 32, 87, 111, 114, 108, 100, 33, 13, 10
	LINE_3 db 77, 101, 108, 108, 111, 119, 32, 87, 111, 114, 108, 100, 33, 13, 10
	LINE_4 db 72, 101, 108, 108, 111, 32, 77, 101, 108, 108, 111, 119, 32, 74, 101, 108, 108, 111, 32, 87, 111, 114, 108, 100, 33, 13, 10
