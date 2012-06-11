cseg	segment
	assume cs:cseg

main:
	mov	dx, offset msg1 + 100h
	mov	ah, 09h
	int	21h
	mov	dx, offset msg2 + 100h
	int	21h
	mov	ax, 4c01h
	int	21h

msg1	db	'This progr$'
PEoff	dd	0
msg2	db	'am cannot be run in DOS mode.', 0dh, 0ah, '$'

cseg	ends
	end	main
