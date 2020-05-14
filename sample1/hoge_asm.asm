SECTION .data

msg:	db "Hello world", 0	; C string needs 0
fmt:    db "%s", 10, 0          ; The printf format, "\n",'0'

SECTION .text

GLOBAL geso		; the standard gcc entry point

geso:
    push    rbp;		; set up stack frame, must be aligned
	
    nop;
    nop;
    
    pop     rbp;

    mov     rax,5;

	ret;