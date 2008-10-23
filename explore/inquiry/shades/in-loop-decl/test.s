	.file	"test.c"
	.text
.globl q
	.type	q, @function
q:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	$0, -4(%ebp)
	jmp	.L2
.L3:
	movl	8(%ebp), %eax
	movl	%eax, -8(%ebp)
	incl	-4(%ebp)
.L2:
	cmpl	$9, -4(%ebp)
	jle	.L3
	movl	$0, %eax
	leave
	ret
	.size	q, .-q
.globl qq
	.type	qq, @function
qq:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	$0, -8(%ebp)
	jmp	.L7
.L8:
	movl	8(%ebp), %eax
	movl	%eax, -4(%ebp)
	incl	-8(%ebp)
.L7:
	cmpl	$9, -8(%ebp)
	jle	.L8
	movl	$0, %eax
	leave
	ret
	.size	qq, .-qq
	.ident	"GCC: (GNU) 4.1.1"
	.section	.note.GNU-stack,"",@progbits
