main:
	lw      a4,-20(s0)
	li      a5,98304
	addi    a5,a5,1695
	bgt     a4,a5, main
	lw      a4,-20(s0)
	li      a5,1000
	rem     a5,a4,a5
	li      a4,-8192
	addi    a3,s0,-16
	add     a4,a3,a4
	slli    a5,a5,3
	add     a5,a4,a5
	lw      a6,188(a5)
	lw      a5,184(a5)
	sw      a5,-20(s0)
	lw      a5,-20(s0)
