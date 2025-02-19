
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

INT_VECTOR_SYS_CALL equ 0x90
_NR_printx	    equ 0
_NR_sendrec	    equ 1
_NR_manage_log   equ     2   ; 新增日志管理系统调用号
_NR_canary_check equ 3
; 导出符号
global	printx
global	sendrec
global	manage_log
global canary_check

bits 32
[section .text]

; ====================================================================================
;                  sendrec(int function, int src_dest, MESSAGE* msg);
; ====================================================================================
; Never call sendrec() directly, call send_recv() instead.
sendrec:
	push	ebx		; .
	push	ecx		;  > 12 bytes
	push	edx		; /

	mov	eax, _NR_sendrec
	mov	ebx, [esp + 12 +  4]	; function
	mov	ecx, [esp + 12 +  8]	; src_dest
	mov	edx, [esp + 12 + 12]	; msg
	int	INT_VECTOR_SYS_CALL

	pop	edx
	pop	ecx
	pop	ebx

	ret

; ====================================================================================
;                          void printx(char* s);
; ====================================================================================
printx:
	push	edx		; 4 bytes

	mov	eax, _NR_printx
	mov	edx, [esp + 4 + 4]	; s
	int	INT_VECTOR_SYS_CALL

	pop	edx

	ret

;====================================================================================
;                          int manage_log(int operation, int param);
;====================================================================================
manage_log:
		push	ebx
		push	ecx

		mov	eax, _NR_manage_log
		mov	ebx, [esp + 4 + 8]    ; operation
		mov	ecx, [esp + 4 + 12]    ; param
		int	INT_VECTOR_SYS_CALL

		pop	ecx
		pop	ebx
		ret
; ====================================================================================
;                          void canary_check();
; ====================================================================================
canary_check:
	mov eax, _NR_canary_check
	int INT_VECTOR_SYS_CALL
	ret

