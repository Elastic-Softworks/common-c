;;    ===================================
;;    C - F O R M ( ASM )
;;    ASM FORMATTING GUIDE
;;    ELASTIC SOFTWORKS 2025
;;    ===================================


;; 				     --- C-FORM ETHOS ---
;;
;; C-FORM for assembly treats the source as a low-level textbook.
;; the style is designed to make hardware interaction, boot processes,
;; and other foundational concepts as clear as possible.

org 0x7c00
bits 16

section .text
	global _start

_start:
	
	;; code logic is separated into blocks by blank lines.
	;; labels always start at column 0.

	xor ax, ax    ; SET AX TO ZERO
	mov ds, ax    ; INIT DATA SEG REGISTER
	mov es, ax    ; INIT EXTRA SEG REGISTER

	;; instructions and directives are indented with one tab.
	;; they are always lowercase.
	
	mov si, msg_loading
	call print_string

load_kernel:
	
.retry_loop:

	mov ah, 0x02  ; BIOS READ SECTORS
	mov al, 8     ; NUMBERS OF SECTORS TO READ
	mov ch, 0     ; CYLINDER 0
	mov cl, 2     ; SECTOR 2
	
	;; single-line comments use a semicolon ';' and explain the
	;; immediate purpose of an instruction or magic number.
	
	int 0x13
	jnc .load_success

	jmp disk_error

.load_success:
	
	;; constants defined with 'equ' are always UPPERCASE.
	
	jmp CODE_SEG:init_pm

;; ==================================
;;           --- FUNCS ---
;; ==================================

;; use decorative comment blocks with double semicolons ';;'
;; to delineate major sections of the file like functions or data.

print_string:
	
	lodsb              ; MAKE SURE
	or al, al          ; THAT EACH INSTRUCTION
	jz .done           ; HAS A BRIEF COMMENT
	mov ah, 0x0E       ; THAT IS CORRECTLY ALIGNED
	int 0x10           ; ALL-CAPS FOR BRIEF COMMS
	jmp print_string   ; BOTTOM TEXT
	
.done:
	
	ret

disk_error:

	mov si, msg_error
	call print_string
	jmp $

;; ==================================
;;            --- DATA ---
;; ==================================

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

msg_loading db 'LOADING...', 13, 10, 0
msg_error   db 'DISK ERROR', 0

times 510 - ($ - $$) db 0
dw 0xAA55

;; ==================================
;;           --- EOF ---
;; ==================================

