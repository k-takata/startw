cseg	segment
	assume cs:cseg

main:
	mov	dx, offset msg1 + 100h
	mov	ah, 09h
	int	21h
	mov	ah, 4ch
	int	21h

msg1	db	'Win32 program.', 0dh, 0ah, '$'
PEoff	dd	0

cseg	ends
	end	main
