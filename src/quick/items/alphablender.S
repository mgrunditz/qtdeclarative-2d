        .data
        .balign 4
stride:
        .word 0
	.text
	/*.align 4*/
        
        .balign 4
.global alphablend

alphablend:
	PUSH {r0-r12}
        VPUSH {s0-s31}
        /*str	fp, [sp, #-4]!
	add	fp, sp, #0
	sub	sp, sp, #12
	str	r1, [fp, #-8]
	ldr	r8, [fp, #-8]
	*/
        ldr r7,[r1] 
        add r1,r1,#4
        ldr r3,[r1] 
        add r1,r1,#4
        ldr r4,[r1] 
        add r1,r1,#4
        ldr r5,[r1] 
	add r1,r1,#4
	ldr r6,[r1]
	add r1,r1,#4
	ldr r8,[r1]
	add r1,r1,#4
        ldr r10, addr_stride
        ldr r11,[r1]
        str r11,[r10]
	mov r12,#4
	mul r9,r4,r12
	mul r10,r6,r12
	mul r11,r9,r8
	add r10,r11,r10
	add r0,r10
b1: 
	mov r1,r4
        mov r10,r1
	mov r8,r5
	mov r4,r7 /* X */
	mov r5,r3 /* Y */
	/* 16 or 32 ?*/
	mov r6,#4
	mul r7,r4,r6
/* screen witdh bytes 
	mov r6,#1920
*/
        ldr r6,addr_stride
        ldr r6,[r6]
	mul r9,r5,r6
	add r7,r7,r9
	add r2,r2,r7
	b blitloop1
blitloop:
   	/*mov r6,#1920*/
        ldr r6,addr_stride
        ldr r6,[r6]
	add r2,r2,r6
	/* 16 or 32 ?*/
	mov r6,#4
        mul r7,r6,r10
        sub r2,r2,r7	
blitloop1:
	PUSH {r1,r8}
        mov r7,#0
        mov r3,#0
        mov r4,#0
        mov r5,#0
        mov r6,#0
        mov r9,#0
	ldrb r6,[r0],#1
	ldrb r3,[r2],#1
	ldrb r4,[r0],#1
	ldrb r5,[r2],#1
	ldrb r7,[r0],#1
	ldrb r8,[r2],#1
	ldrb r9,[r0],#1
	ldrb r1,[r2],#1
	cmp r9,#255
	beq noalpha
	cmp r9,#0
	beq contblit
	vmov s2,r9
	vmov s1,r1
	mov  r9,#255
	vmov.f s0,r9
	vdiv.f32 s3,s1,s0
	vdiv.f32 s4,s2,s0
	vmov.f s9,r3
	vmov.f s10,r5
	vmov.f s11,r8
	vmov.f s12,r6
	vmov.f s13,r4
	vmov.f32 s14,r7
	vmov s15,#1.0
	vsub.f32 s16,s15,s4
        vmul.f32 s17,s3,s16
	vadd.f32 s18,s17,s4
/*alpha = s18*/
        vmul.f32 s19,s9,s16
	vadd.f32 s20,s19,s12
 	
        vmul.f32 s21,s10,s16
	vadd.f32 s22,s21,s13

        vmul.f32 s21,s11,s16
	vadd.f32 s23,s21,s14

	sub r2,#4
breakpoint:
	vmov r8,s20
b2:
	strb r8,[r2]
	add r2,r2,#1
	vmov r8,s22
	strb r8,[r2]
	add r2,r2,#1
	vmov r8,s23
	strb r8,[r2]
	add r2,r2,#1
	mov r9,#255
	vmov s27,r9
	vmul.f32 s28,s27,s18
	vmov r8,s28
	strb r8,[r2]
	add r2,r2,#1
contblit:
	POP  {r1,r8}
	sub r1,r1,#1
	cmp r1,#0
	bne blitloop1
b3:
	mov r1,r10
	sub r8,r8,#1
	cmp r8,#0
	bne blitloop
        VPOP {s0-s31}
	POP {r0-r12}
	bx lr

	
noalpha:
	sub r2,#4
	strb r6,[r2],#1	
	strb r4,[r2],#1	
	strb r7,[r2],#1	
	strb r9,[r2],#1	
	b contblit


addr_stride:
        .word stride
