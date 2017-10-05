; Base Code for Library HTTPLIB
; Generated from HTTPLIB.LID version 1.0 on 05 Oct 2017 15:18

	IMPORT	||Image$$CODE$$Base||
	IMPORT	||Image$$CODE$$Length||
	IMPORT	||Image$$DATA$$Length||
	IMPORT	||Image$$BSS$$ZI$$Length||
	IMPORT	_shlgo_abs
	EXPORT	_version
	EXPORT	_verix_header
	EXPORT	||SHL$$Index||
||SHL$$Index||	EQU	4*27
	EXPORT	||SHL$$IndexBase||
||SHL$$IndexBase|| EQU	0

	PRESERVE8

	AREA	||.0_vrx_hdr||,CODE,READONLY
_verix_header
	DCB	0xA5	; magic
	DCB	6	; Flags
_version
	DCB	1,0	; version
	DCD	||Image$$CODE$$Length||+||Image$$DATA$$Length||+0
	DCD	||Image$$CODE$$Base||
	DCD	||Image$$DATA$$Length||+||Image$$BSS$$ZI$$Length||
	DCD	27	; ID number
	DCD	0
	DCD	0	; heap size
	DCD	_shlgo_abs-_verix_header ; startup code offset

; Dispatch Table
	IMPORT	_HttpInitRequest	;1
	CODE16
	BX	PC
	NOP
	CODE32
	LDR	IP,[PC,#0]
	BX	IP
	DCD	_HttpInitRequest
	IMPORT	_HttpSetProperty	;2
	CODE16
	BX	PC
	NOP
	CODE32
	LDR	IP,[PC,#0]
	BX	IP
	DCD	_HttpSetProperty
	IMPORT	_HttpSetRequestBody	;3
	CODE16
	BX	PC
	NOP
	CODE32
	LDR	IP,[PC,#0]
	BX	IP
	DCD	_HttpSetRequestBody
	IMPORT	_HttpConnect  	;4
	CODE16
	BX	PC
	NOP
	CODE32
	LDR	IP,[PC,#0]
	BX	IP
	DCD	_HttpConnect
	IMPORT	_HttpDisconnect	;5
	CODE16
	BX	PC
	NOP
	CODE32
	LDR	IP,[PC,#0]
	BX	IP
	DCD	_HttpDisconnect
	IMPORT	_HttpSend     	;6
	CODE16
	BX	PC
	NOP
	CODE32
	LDR	IP,[PC,#0]
	BX	IP
	DCD	_HttpSend
	IMPORT	_HttpRecv     	;7
	CODE16
	BX	PC
	NOP
	CODE32
	LDR	IP,[PC,#0]
	BX	IP
	DCD	_HttpRecv
	IMPORT	_HttpSetRequestBodyRaw	;8
	CODE16
	BX	PC
	NOP
	CODE32
	LDR	IP,[PC,#0]
	BX	IP
	DCD	_HttpSetRequestBodyRaw
	IMPORT	_HttpCompleteRequest	;9
	CODE16
	BX	PC
	NOP
	CODE32
	LDR	IP,[PC,#0]
	BX	IP
	DCD	_HttpCompleteRequest
	IMPORT	_HttpGetProperty	;10
	CODE16
	BX	PC
	NOP
	CODE32
	LDR	IP,[PC,#0]
	BX	IP
	DCD	_HttpGetProperty

	ENTRY	; dummy

	AREA	||.dummy_pdt||,DATA,READWRITE
	DCD	0	; PDT

	END
