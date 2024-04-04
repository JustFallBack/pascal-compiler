			# This code was produced by the CERI Compiler
	.text		# The following lines contain the program
	.globl main	# The main function must be visible from outside
main:			# The main function body :
	movq %rsp, %rbp	# Save the position of the stack's top
	push $8
	push $5
	pop %rbx
	pop %rax
	addq	%rbx, %rax
	push %rax
	push $9
	push $2
	pop %rbx
	pop %rax
	addq	%rbx, %rax
	push %rax
	pop %rax
	pop %rbx
	cmpq %rax, %rbx
	ja True		 # Jump if above
	jmp False
False:	Push $0	# Faux
	jmp EndExp
True:	Push $-1	# Vrai
EndExp:
	movq %rbp, %rsp		# Restore the position of the stack's top
	ret			# Return from main function
