
d273liu-a1.elf:	file format elf64-littleaarch64

Disassembly of section .text.boot:

0000000000080000 <_start>:
   80000: d5384241     	mrs	x1, CurrentEL
   80004: 927d0021     	and	x1, x1, #0x8
   80008: b4000101     	cbz	x1, 0x80028 <el1_entry>
   8000c: 58000362     	ldr	x2, 0x80078 <$d>
   80010: d51c1102     	msr	HCR_EL2, x2
   80014: 58000363     	ldr	x3, 0x80080 <$d+0x8>
   80018: d51c4003     	msr	SPSR_EL2, x3
   8001c: 10000064     	adr	x4, #12
   80020: d51c4024     	msr	ELR_EL2, x4
   80024: d69f03e0     	eret

0000000000080028 <el1_entry>:
   80028: 58000302     	ldr	x2, 0x80088 <$d+0x10>
   8002c: d5181002     	msr	SCTLR_EL1, x2
   80030: d5034fdf     	msr	DAIFSet, #15
   80034: d50041bf     	msr	SPSel, #1
   80038: 580002c0     	ldr	x0, 0x80090 <$d+0x18>
   8003c: 9100001f     	mov	sp, x0
   80040: 94000016     	bl	0x80098 <kmain>
   80044: d2800482     	mov	x2, #36
   80048: d2800380     	mov	x0, #28
   8004c: f2bfc202     	movk	x2, #65040, lsl #16
   80050: f2bfc200     	movk	x0, #65040, lsl #16
   80054: 52800023     	mov	w3, #1
   80058: 52800401     	mov	w1, #32
   8005c: 72ab4003     	movk	w3, #23040, lsl #16
   80060: b9000043     	str	w3, [x2]
   80064: 72ab4001     	movk	w1, #23040, lsl #16
   80068: b9000001     	str	w1, [x0]

000000000008006c <exit>:
   8006c: d503207f     	wfi
   80070: 17ffffff     	b	0x8006c <exit>
   80074: 00000000     	udf	#0

0000000000080078 <$d>:
   80078: 00 00 00 80  	.word	0x80000000
   8007c: 00 00 00 00  	.word	0x00000000
   80080: c5 01 00 00  	.word	0x000001c5
   80084: 00 00 00 00  	.word	0x00000000
   80088: 00 0a d5 30  	.word	0x30d50a00
   8008c: 00 00 00 00  	.word	0x00000000
   80090: 90 0f 09 00  	.word	0x00090f90
   80094: 00 00 00 00  	.word	0x00000000

Disassembly of section .text:

0000000000080098 <kmain>:
   80098: a9ba7bfd     	stp	x29, x30, [sp, #-96]!
   8009c: 910003fd     	mov	x29, sp
   800a0: 90000000     	adrp	x0, 0x80000 <kmain+0x8>
   800a4: 9138c001     	add	x1, x0, #3632
   800a8: 910043e0     	add	x0, sp, #16
   800ac: aa0103e3     	mov	x3, x1
   800b0: d28007c1     	mov	x1, #62
   800b4: aa0103e2     	mov	x2, x1
   800b8: aa0303e1     	mov	x1, x3
   800bc: 94000334     	bl	0x80d8c <memcpy>
   800c0: 9400009e     	bl	0x80338 <uart_init>
   800c4: 52984001     	mov	w1, #49664
   800c8: 72a00021     	movk	w1, #1, lsl #16
   800cc: d2800020     	mov	x0, #1
   800d0: 940000c3     	bl	0x803dc <uart_config_and_enable>
   800d4: 94000288     	bl	0x80af4 <get_timerLO>
   800d8: b90057e0     	str	w0, [sp, #84]
   800dc: d503201f     	nop
   800e0: 94000285     	bl	0x80af4 <get_timerLO>
   800e4: 2a0003e1     	mov	w1, w0
   800e8: b94057e0     	ldr	w0, [sp, #84]
   800ec: 4b000021     	sub	w1, w1, w0
   800f0: 528270e0     	mov	w0, #4999
   800f4: 6b00003f     	cmp	w1, w0
   800f8: 54ffff49     	b.ls	0x800e0 <kmain+0x48>
   800fc: 910043e0     	add	x0, sp, #16
   80100: aa0003e1     	mov	x1, x0
   80104: d2800020     	mov	x0, #1
   80108: 9400019c     	bl	0x80778 <uart_puts>
   8010c: 52800020     	mov	w0, #1
   80110: b9005fe0     	str	w0, [sp, #92]
   80114: b9405fe0     	ldr	w0, [sp, #92]
   80118: 11000401     	add	w1, w0, #1
   8011c: b9005fe1     	str	w1, [sp, #92]
   80120: 2a0003e2     	mov	w2, w0
   80124: 90000000     	adrp	x0, 0x80000 <kmain+0x8c>
   80128: 91380001     	add	x1, x0, #3584
   8012c: d2800020     	mov	x0, #1
   80130: 94000252     	bl	0x80a78 <uart_printf>
   80134: 52800400     	mov	w0, #32
   80138: 39016fe0     	strb	w0, [sp, #91]
   8013c: 14000013     	b	0x80188 <kmain+0xf0>
   80140: d2800020     	mov	x0, #1
   80144: 9400013d     	bl	0x80638 <uart_getc>
   80148: 39016fe0     	strb	w0, [sp, #91]
   8014c: 39416fe0     	ldrb	w0, [sp, #91]
   80150: 7100341f     	cmp	w0, #13
   80154: 54000141     	b.ne	0x8017c <kmain+0xe4>
   80158: b9405fe0     	ldr	w0, [sp, #92]
   8015c: 11000401     	add	w1, w0, #1
   80160: b9005fe1     	str	w1, [sp, #92]
   80164: 2a0003e2     	mov	w2, w0
   80168: 90000000     	adrp	x0, 0x80000 <kmain+0xd0>
   8016c: 91384001     	add	x1, x0, #3600
   80170: d2800020     	mov	x0, #1
   80174: 94000241     	bl	0x80a78 <uart_printf>
   80178: 14000004     	b	0x80188 <kmain+0xf0>
   8017c: 39416fe1     	ldrb	w1, [sp, #91]
   80180: d2800020     	mov	x0, #1
   80184: 94000148     	bl	0x806a4 <uart_putc>
   80188: 39416fe0     	ldrb	w0, [sp, #91]
   8018c: 7101c41f     	cmp	w0, #113
   80190: 54fffd81     	b.ne	0x80140 <kmain+0xa8>
   80194: 90000000     	adrp	x0, 0x80000 <kmain+0xfc>
   80198: 9138a001     	add	x1, x0, #3624
   8019c: d2800020     	mov	x0, #1
   801a0: 94000176     	bl	0x80778 <uart_puts>
   801a4: 52800000     	mov	w0, #0
   801a8: a8c67bfd     	ldp	x29, x30, [sp], #96
   801ac: d65f03c0     	ret

00000000000801b0 <setup_gpio>:
   801b0: d10083ff     	sub	sp, sp, #32
   801b4: b9000fe0     	str	w0, [sp, #12]
   801b8: b9000be1     	str	w1, [sp, #8]
   801bc: b90007e2     	str	w2, [sp, #4]
   801c0: b9400fe1     	ldr	w1, [sp, #12]
   801c4: 529999a0     	mov	w0, #52429
   801c8: 72b99980     	movk	w0, #52428, lsl #16
   801cc: 9ba07c20     	umull	x0, w1, w0
   801d0: d360fc00     	lsr	x0, x0, #32
   801d4: 53037c00     	lsr	w0, w0, #3
   801d8: b9001fe0     	str	w0, [sp, #28]
   801dc: b9400fe2     	ldr	w2, [sp, #12]
   801e0: 529999a0     	mov	w0, #52429
   801e4: 72b99980     	movk	w0, #52428, lsl #16
   801e8: 9ba07c40     	umull	x0, w2, w0
   801ec: d360fc00     	lsr	x0, x0, #32
   801f0: 53037c01     	lsr	w1, w0, #3
   801f4: 2a0103e0     	mov	w0, w1
   801f8: 531e7400     	lsl	w0, w0, #2
   801fc: 0b010000     	add	w0, w0, w1
   80200: 531f7800     	lsl	w0, w0, #1
   80204: 4b000041     	sub	w1, w2, w0
   80208: 2a0103e0     	mov	w0, w1
   8020c: 531f7800     	lsl	w0, w0, #1
   80210: 0b010000     	add	w0, w0, w1
   80214: b9001be0     	str	w0, [sp, #24]
   80218: d2bfc401     	mov	x1, #4263510016
   8021c: 90000000     	adrp	x0, 0x80000 <setup_gpio+0x6c>
   80220: 913a0000     	add	x0, x0, #3712
   80224: b9401fe2     	ldr	w2, [sp, #28]
   80228: b8627800     	ldr	w0, [x0, x2, lsl  #2]
   8022c: 2a0003e0     	mov	w0, w0
   80230: 8b000020     	add	x0, x1, x0
   80234: b9400000     	ldr	w0, [x0]
   80238: b90017e0     	str	w0, [sp, #20]
   8023c: b9401be0     	ldr	w0, [sp, #24]
   80240: 528000e1     	mov	w1, #7
   80244: 1ac02020     	lsl	w0, w1, w0
   80248: 2a2003e0     	mvn	w0, w0
   8024c: b94017e1     	ldr	w1, [sp, #20]
   80250: 0a000020     	and	w0, w1, w0
   80254: b90017e0     	str	w0, [sp, #20]
   80258: b9401be0     	ldr	w0, [sp, #24]
   8025c: b9400be1     	ldr	w1, [sp, #8]
   80260: 1ac02020     	lsl	w0, w1, w0
   80264: b94017e1     	ldr	w1, [sp, #20]
   80268: 2a000020     	orr	w0, w1, w0
   8026c: b90017e0     	str	w0, [sp, #20]
   80270: d2bfc401     	mov	x1, #4263510016
   80274: 90000000     	adrp	x0, 0x80000 <setup_gpio+0xc4>
   80278: 913a0000     	add	x0, x0, #3712
   8027c: b9401fe2     	ldr	w2, [sp, #28]
   80280: b8627800     	ldr	w0, [x0, x2, lsl  #2]
   80284: 2a0003e0     	mov	w0, w0
   80288: 8b000020     	add	x0, x1, x0
   8028c: b94017e1     	ldr	w1, [sp, #20]
   80290: b9000001     	str	w1, [x0]
   80294: b9400fe0     	ldr	w0, [sp, #12]
   80298: 53047c00     	lsr	w0, w0, #4
   8029c: b9001fe0     	str	w0, [sp, #28]
   802a0: b9400fe0     	ldr	w0, [sp, #12]
   802a4: 12000c00     	and	w0, w0, #0xf
   802a8: 531f7800     	lsl	w0, w0, #1
   802ac: b9001be0     	str	w0, [sp, #24]
   802b0: d2bfc401     	mov	x1, #4263510016
   802b4: 90000000     	adrp	x0, 0x80000 <setup_gpio+0x104>
   802b8: 913a6000     	add	x0, x0, #3736
   802bc: b9401fe2     	ldr	w2, [sp, #28]
   802c0: b8627800     	ldr	w0, [x0, x2, lsl  #2]
   802c4: 2a0003e0     	mov	w0, w0
   802c8: 8b000020     	add	x0, x1, x0
   802cc: b9400000     	ldr	w0, [x0]
   802d0: b90017e0     	str	w0, [sp, #20]
   802d4: b9401be0     	ldr	w0, [sp, #24]
   802d8: 52800061     	mov	w1, #3
   802dc: 1ac02020     	lsl	w0, w1, w0
   802e0: 2a2003e0     	mvn	w0, w0
   802e4: b94017e1     	ldr	w1, [sp, #20]
   802e8: 0a000020     	and	w0, w1, w0
   802ec: b90017e0     	str	w0, [sp, #20]
   802f0: b9401be0     	ldr	w0, [sp, #24]
   802f4: b94007e1     	ldr	w1, [sp, #4]
   802f8: 1ac02020     	lsl	w0, w1, w0
   802fc: b94017e1     	ldr	w1, [sp, #20]
   80300: 2a000020     	orr	w0, w1, w0
   80304: b90017e0     	str	w0, [sp, #20]
   80308: d2bfc401     	mov	x1, #4263510016
   8030c: 90000000     	adrp	x0, 0x80000 <setup_gpio+0x15c>
   80310: 913a6000     	add	x0, x0, #3736
   80314: b9401fe2     	ldr	w2, [sp, #28]
   80318: b8627800     	ldr	w0, [x0, x2, lsl  #2]
   8031c: 2a0003e0     	mov	w0, w0
   80320: 8b000020     	add	x0, x1, x0
   80324: b94017e1     	ldr	w1, [sp, #20]
   80328: b9000001     	str	w1, [x0]
   8032c: d503201f     	nop
   80330: 910083ff     	add	sp, sp, #32
   80334: d65f03c0     	ret

0000000000080338 <uart_init>:
   80338: a9bf7bfd     	stp	x29, x30, [sp, #-16]!
   8033c: 910003fd     	mov	x29, sp
   80340: 52800060     	mov	w0, #3
   80344: 52800001     	mov	w1, #0
   80348: 2a0103e2     	mov	w2, w1
   8034c: 2a0003e1     	mov	w1, w0
   80350: 52800080     	mov	w0, #4
   80354: 97ffff97     	bl	0x801b0 <setup_gpio>
   80358: 52800060     	mov	w0, #3
   8035c: 52800001     	mov	w1, #0
   80360: 2a0103e2     	mov	w2, w1
   80364: 2a0003e1     	mov	w1, w0
   80368: 528000a0     	mov	w0, #5
   8036c: 97ffff91     	bl	0x801b0 <setup_gpio>
   80370: 52800060     	mov	w0, #3
   80374: 52800001     	mov	w1, #0
   80378: 2a0103e2     	mov	w2, w1
   8037c: 2a0003e1     	mov	w1, w0
   80380: 528000c0     	mov	w0, #6
   80384: 97ffff8b     	bl	0x801b0 <setup_gpio>
   80388: 52800060     	mov	w0, #3
   8038c: 52800001     	mov	w1, #0
   80390: 2a0103e2     	mov	w2, w1
   80394: 2a0003e1     	mov	w1, w0
   80398: 528000e0     	mov	w0, #7
   8039c: 97ffff85     	bl	0x801b0 <setup_gpio>
   803a0: 52800080     	mov	w0, #4
   803a4: 52800001     	mov	w1, #0
   803a8: 2a0103e2     	mov	w2, w1
   803ac: 2a0003e1     	mov	w1, w0
   803b0: 528001c0     	mov	w0, #14
   803b4: 97ffff7f     	bl	0x801b0 <setup_gpio>
   803b8: 52800080     	mov	w0, #4
   803bc: 52800001     	mov	w1, #0
   803c0: 2a0103e2     	mov	w2, w1
   803c4: 2a0003e1     	mov	w1, w0
   803c8: 528001e0     	mov	w0, #15
   803cc: 97ffff79     	bl	0x801b0 <setup_gpio>
   803d0: d503201f     	nop
   803d4: a8c17bfd     	ldp	x29, x30, [sp], #16
   803d8: d65f03c0     	ret

00000000000803dc <uart_config_and_enable>:
   803dc: d10083ff     	sub	sp, sp, #32
   803e0: f90007e0     	str	x0, [sp, #8]
   803e4: b90007e1     	str	w1, [sp, #4]
   803e8: 528d8000     	mov	w0, #27648
   803ec: 72a05b80     	movk	w0, #732, lsl #16
   803f0: 2a0003e0     	mov	w0, w0
   803f4: d37ef401     	lsl	x1, x0, #2
   803f8: b94007e0     	ldr	w0, [sp, #4]
   803fc: 9ac00820     	udiv	x0, x1, x0
   80400: b9001fe0     	str	w0, [sp, #28]
   80404: 90000000     	adrp	x0, 0x80000 <uart_config_and_enable+0x28>
   80408: 913ba000     	add	x0, x0, #3816
   8040c: f94007e1     	ldr	x1, [sp, #8]
   80410: f8617801     	ldr	x1, [x0, x1, lsl  #3]
   80414: 52800600     	mov	w0, #48
   80418: 2a0003e0     	mov	w0, w0
   8041c: 8b000020     	add	x0, x1, x0
   80420: b9400000     	ldr	w0, [x0]
   80424: b9001be0     	str	w0, [sp, #24]
   80428: 52800020     	mov	w0, #1
   8042c: 2a2003e2     	mvn	w2, w0
   80430: 90000000     	adrp	x0, 0x80000 <uart_config_and_enable+0x54>
   80434: 913ba000     	add	x0, x0, #3816
   80438: f94007e1     	ldr	x1, [sp, #8]
   8043c: f8617801     	ldr	x1, [x0, x1, lsl  #3]
   80440: 52800600     	mov	w0, #48
   80444: 2a0003e0     	mov	w0, w0
   80448: 8b000020     	add	x0, x1, x0
   8044c: b9401be1     	ldr	w1, [sp, #24]
   80450: 0a010041     	and	w1, w2, w1
   80454: b9000001     	str	w1, [x0]
   80458: f94007e0     	ldr	x0, [sp, #8]
   8045c: f100081f     	cmp	x0, #2
   80460: 54000221     	b.ne	0x804a4 <uart_config_and_enable+0xc8>
   80464: 52800801     	mov	w1, #64
   80468: 52800400     	mov	w0, #32
   8046c: 2a000021     	orr	w1, w1, w0
   80470: 52800200     	mov	w0, #16
   80474: 2a000022     	orr	w2, w1, w0
   80478: 52800101     	mov	w1, #8
   8047c: 90000000     	adrp	x0, 0x80000 <uart_config_and_enable+0xa0>
   80480: 913ba000     	add	x0, x0, #3816
   80484: f94007e3     	ldr	x3, [sp, #8]
   80488: f8637803     	ldr	x3, [x0, x3, lsl  #3]
   8048c: 52800580     	mov	w0, #44
   80490: 2a0003e0     	mov	w0, w0
   80494: 8b000060     	add	x0, x3, x0
   80498: 2a010041     	orr	w1, w2, w1
   8049c: b9000001     	str	w1, [x0]
   804a0: 14000011     	b	0x804e4 <uart_config_and_enable+0x108>
   804a4: f94007e0     	ldr	x0, [sp, #8]
   804a8: f100041f     	cmp	x0, #1
   804ac: 540001c1     	b.ne	0x804e4 <uart_config_and_enable+0x108>
   804b0: 52800801     	mov	w1, #64
   804b4: 52800400     	mov	w0, #32
   804b8: 2a000022     	orr	w2, w1, w0
   804bc: 52800201     	mov	w1, #16
   804c0: 90000000     	adrp	x0, 0x80000 <uart_config_and_enable+0xe4>
   804c4: 913ba000     	add	x0, x0, #3816
   804c8: f94007e3     	ldr	x3, [sp, #8]
   804cc: f8637803     	ldr	x3, [x0, x3, lsl  #3]
   804d0: 52800580     	mov	w0, #44
   804d4: 2a0003e0     	mov	w0, w0
   804d8: 8b000060     	add	x0, x3, x0
   804dc: 2a010041     	orr	w1, w2, w1
   804e0: b9000001     	str	w1, [x0]
   804e4: 90000000     	adrp	x0, 0x80000 <uart_config_and_enable+0x108>
   804e8: 913ba000     	add	x0, x0, #3816
   804ec: f94007e1     	ldr	x1, [sp, #8]
   804f0: f8617801     	ldr	x1, [x0, x1, lsl  #3]
   804f4: 52800480     	mov	w0, #36
   804f8: 2a0003e0     	mov	w0, w0
   804fc: 8b000020     	add	x0, x1, x0
   80500: b9401fe1     	ldr	w1, [sp, #28]
   80504: 53067c21     	lsr	w1, w1, #6
   80508: b9000001     	str	w1, [x0]
   8050c: 90000000     	adrp	x0, 0x80000 <uart_config_and_enable+0x130>
   80510: 913ba000     	add	x0, x0, #3816
   80514: f94007e1     	ldr	x1, [sp, #8]
   80518: f8617801     	ldr	x1, [x0, x1, lsl  #3]
   8051c: 52800500     	mov	w0, #40
   80520: 2a0003e0     	mov	w0, w0
   80524: 8b000020     	add	x0, x1, x0
   80528: b9401fe1     	ldr	w1, [sp, #28]
   8052c: 12001421     	and	w1, w1, #0x3f
   80530: b9000001     	str	w1, [x0]
   80534: 52800021     	mov	w1, #1
   80538: b9401be0     	ldr	w0, [sp, #24]
   8053c: 2a000021     	orr	w1, w1, w0
   80540: 52802000     	mov	w0, #256
   80544: 2a000022     	orr	w2, w1, w0
   80548: 52804001     	mov	w1, #512
   8054c: 90000000     	adrp	x0, 0x80000 <uart_config_and_enable+0x170>
   80550: 913ba000     	add	x0, x0, #3816
   80554: f94007e3     	ldr	x3, [sp, #8]
   80558: f8637803     	ldr	x3, [x0, x3, lsl  #3]
   8055c: 52800600     	mov	w0, #48
   80560: 2a0003e0     	mov	w0, w0
   80564: 8b000060     	add	x0, x3, x0
   80568: 2a010041     	orr	w1, w2, w1
   8056c: b9000001     	str	w1, [x0]
   80570: d503201f     	nop
   80574: 910083ff     	add	sp, sp, #32
   80578: d65f03c0     	ret

000000000008057c <uart_getc_queue>:
   8057c: d10043ff     	sub	sp, sp, #16
   80580: f90007e0     	str	x0, [sp, #8]
   80584: 90000000     	adrp	x0, 0x80000 <uart_getc_queue+0x8>
   80588: 913ba000     	add	x0, x0, #3816
   8058c: f94007e1     	ldr	x1, [sp, #8]
   80590: f8617801     	ldr	x1, [x0, x1, lsl  #3]
   80594: 52800300     	mov	w0, #24
   80598: 2a0003e0     	mov	w0, w0
   8059c: 8b000020     	add	x0, x1, x0
   805a0: b9400001     	ldr	w1, [x0]
   805a4: 52800200     	mov	w0, #16
   805a8: 0a000020     	and	w0, w1, w0
   805ac: 7100001f     	cmp	w0, #0
   805b0: 54000060     	b.eq	0x805bc <uart_getc_queue+0x40>
   805b4: 52800000     	mov	w0, #0
   805b8: 14000002     	b	0x805c0 <uart_getc_queue+0x44>
   805bc: 52800020     	mov	w0, #1
   805c0: 910043ff     	add	sp, sp, #16
   805c4: d65f03c0     	ret

00000000000805c8 <uart_getc_modified>:
   805c8: d10083ff     	sub	sp, sp, #32
   805cc: f90007e0     	str	x0, [sp, #8]
   805d0: 90000000     	adrp	x0, 0x80000 <uart_getc_modified+0x8>
   805d4: 913ba000     	add	x0, x0, #3816
   805d8: f94007e1     	ldr	x1, [sp, #8]
   805dc: f8617801     	ldr	x1, [x0, x1, lsl  #3]
   805e0: 52800300     	mov	w0, #24
   805e4: 2a0003e0     	mov	w0, w0
   805e8: 8b000020     	add	x0, x1, x0
   805ec: b9400001     	ldr	w1, [x0]
   805f0: 52800200     	mov	w0, #16
   805f4: 0a000020     	and	w0, w1, w0
   805f8: 7100001f     	cmp	w0, #0
   805fc: 54000060     	b.eq	0x80608 <uart_getc_modified+0x40>
   80600: 52800000     	mov	w0, #0
   80604: 1400000b     	b	0x80630 <uart_getc_modified+0x68>
   80608: 90000000     	adrp	x0, 0x80000 <uart_getc_modified+0x40>
   8060c: 913ba000     	add	x0, x0, #3816
   80610: f94007e1     	ldr	x1, [sp, #8]
   80614: f8617801     	ldr	x1, [x0, x1, lsl  #3]
   80618: 52800000     	mov	w0, #0
   8061c: 2a0003e0     	mov	w0, w0
   80620: 8b000020     	add	x0, x1, x0
   80624: b9400000     	ldr	w0, [x0]
   80628: 39007fe0     	strb	w0, [sp, #31]
   8062c: 39407fe0     	ldrb	w0, [sp, #31]
   80630: 910083ff     	add	sp, sp, #32
   80634: d65f03c0     	ret

0000000000080638 <uart_getc>:
   80638: d10083ff     	sub	sp, sp, #32
   8063c: f90007e0     	str	x0, [sp, #8]
   80640: d503201f     	nop
   80644: 90000000     	adrp	x0, 0x80000 <uart_getc+0xc>
   80648: 913ba000     	add	x0, x0, #3816
   8064c: f94007e1     	ldr	x1, [sp, #8]
   80650: f8617801     	ldr	x1, [x0, x1, lsl  #3]
   80654: 52800300     	mov	w0, #24
   80658: 2a0003e0     	mov	w0, w0
   8065c: 8b000020     	add	x0, x1, x0
   80660: b9400001     	ldr	w1, [x0]
   80664: 52800200     	mov	w0, #16
   80668: 0a000020     	and	w0, w1, w0
   8066c: 7100001f     	cmp	w0, #0
   80670: 54fffea1     	b.ne	0x80644 <uart_getc+0xc>
   80674: 90000000     	adrp	x0, 0x80000 <uart_getc+0x3c>
   80678: 913ba000     	add	x0, x0, #3816
   8067c: f94007e1     	ldr	x1, [sp, #8]
   80680: f8617801     	ldr	x1, [x0, x1, lsl  #3]
   80684: 52800000     	mov	w0, #0
   80688: 2a0003e0     	mov	w0, w0
   8068c: 8b000020     	add	x0, x1, x0
   80690: b9400000     	ldr	w0, [x0]
   80694: 39007fe0     	strb	w0, [sp, #31]
   80698: 39407fe0     	ldrb	w0, [sp, #31]
   8069c: 910083ff     	add	sp, sp, #32
   806a0: d65f03c0     	ret

00000000000806a4 <uart_putc>:
   806a4: d10043ff     	sub	sp, sp, #16
   806a8: f90007e0     	str	x0, [sp, #8]
   806ac: 39001fe1     	strb	w1, [sp, #7]
   806b0: d503201f     	nop
   806b4: 90000000     	adrp	x0, 0x80000 <uart_putc+0x10>
   806b8: 913ba000     	add	x0, x0, #3816
   806bc: f94007e1     	ldr	x1, [sp, #8]
   806c0: f8617801     	ldr	x1, [x0, x1, lsl  #3]
   806c4: 52800300     	mov	w0, #24
   806c8: 2a0003e0     	mov	w0, w0
   806cc: 8b000020     	add	x0, x1, x0
   806d0: b9400001     	ldr	w1, [x0]
   806d4: 52800400     	mov	w0, #32
   806d8: 0a000020     	and	w0, w1, w0
   806dc: 7100001f     	cmp	w0, #0
   806e0: 54fffea1     	b.ne	0x806b4 <uart_putc+0x10>
   806e4: 90000000     	adrp	x0, 0x80000 <uart_putc+0x40>
   806e8: 913ba000     	add	x0, x0, #3816
   806ec: f94007e1     	ldr	x1, [sp, #8]
   806f0: f8617801     	ldr	x1, [x0, x1, lsl  #3]
   806f4: 52800000     	mov	w0, #0
   806f8: 2a0003e0     	mov	w0, w0
   806fc: 8b000020     	add	x0, x1, x0
   80700: 39401fe1     	ldrb	w1, [sp, #7]
   80704: b9000001     	str	w1, [x0]
   80708: d503201f     	nop
   8070c: 910043ff     	add	sp, sp, #16
   80710: d65f03c0     	ret

0000000000080714 <uart_putl>:
   80714: a9bc7bfd     	stp	x29, x30, [sp, #-64]!
   80718: 910003fd     	mov	x29, sp
   8071c: f90017e0     	str	x0, [sp, #40]
   80720: f90013e1     	str	x1, [sp, #32]
   80724: f9000fe2     	str	x2, [sp, #24]
   80728: b9003fff     	str	wzr, [sp, #60]
   8072c: 1400000b     	b	0x80758 <uart_putl+0x44>
   80730: b9403fe0     	ldr	w0, [sp, #60]
   80734: f94013e1     	ldr	x1, [sp, #32]
   80738: 8b000020     	add	x0, x1, x0
   8073c: 39400000     	ldrb	w0, [x0]
   80740: 2a0003e1     	mov	w1, w0
   80744: f94017e0     	ldr	x0, [sp, #40]
   80748: 97ffffd7     	bl	0x806a4 <uart_putc>
   8074c: b9403fe0     	ldr	w0, [sp, #60]
   80750: 11000400     	add	w0, w0, #1
   80754: b9003fe0     	str	w0, [sp, #60]
   80758: b9403fe0     	ldr	w0, [sp, #60]
   8075c: f9400fe1     	ldr	x1, [sp, #24]
   80760: eb00003f     	cmp	x1, x0
   80764: 54fffe68     	b.hi	0x80730 <uart_putl+0x1c>
   80768: d503201f     	nop
   8076c: d503201f     	nop
   80770: a8c47bfd     	ldp	x29, x30, [sp], #64
   80774: d65f03c0     	ret

0000000000080778 <uart_puts>:
   80778: a9be7bfd     	stp	x29, x30, [sp, #-32]!
   8077c: 910003fd     	mov	x29, sp
   80780: f9000fe0     	str	x0, [sp, #24]
   80784: f9000be1     	str	x1, [sp, #16]
   80788: 14000009     	b	0x807ac <uart_puts+0x34>
   8078c: f9400be0     	ldr	x0, [sp, #16]
   80790: 39400000     	ldrb	w0, [x0]
   80794: 2a0003e1     	mov	w1, w0
   80798: f9400fe0     	ldr	x0, [sp, #24]
   8079c: 97ffffc2     	bl	0x806a4 <uart_putc>
   807a0: f9400be0     	ldr	x0, [sp, #16]
   807a4: 91000400     	add	x0, x0, #1
   807a8: f9000be0     	str	x0, [sp, #16]
   807ac: f9400be0     	ldr	x0, [sp, #16]
   807b0: 39400000     	ldrb	w0, [x0]
   807b4: 7100001f     	cmp	w0, #0
   807b8: 54fffea1     	b.ne	0x8078c <uart_puts+0x14>
   807bc: d503201f     	nop
   807c0: d503201f     	nop
   807c4: a8c27bfd     	ldp	x29, x30, [sp], #32
   807c8: d65f03c0     	ret

00000000000807cc <uart_format_print>:
   807cc: a9bc7bfd     	stp	x29, x30, [sp, #-64]!
   807d0: 910003fd     	mov	x29, sp
   807d4: f9000bf3     	str	x19, [sp, #16]
   807d8: f90017e0     	str	x0, [sp, #40]
   807dc: f90013e1     	str	x1, [sp, #32]
   807e0: aa0203f3     	mov	x19, x2
   807e4: 14000098     	b	0x80a44 <uart_format_print+0x278>
   807e8: 3940ffe0     	ldrb	w0, [sp, #63]
   807ec: 7100941f     	cmp	w0, #37
   807f0: 540000a0     	b.eq	0x80804 <uart_format_print+0x38>
   807f4: 3940ffe1     	ldrb	w1, [sp, #63]
   807f8: f94017e0     	ldr	x0, [sp, #40]
   807fc: 97ffffaa     	bl	0x806a4 <uart_putc>
   80800: 14000091     	b	0x80a44 <uart_format_print+0x278>
   80804: f94013e0     	ldr	x0, [sp, #32]
   80808: 91000401     	add	x1, x0, #1
   8080c: f90013e1     	str	x1, [sp, #32]
   80810: 39400000     	ldrb	w0, [x0]
   80814: 3900ffe0     	strb	w0, [sp, #63]
   80818: 3940ffe0     	ldrb	w0, [sp, #63]
   8081c: 7101e01f     	cmp	w0, #120
   80820: 540009c0     	b.eq	0x80958 <uart_format_print+0x18c>
   80824: 7101e01f     	cmp	w0, #120
   80828: 540010ec     	b.gt	0x80a44 <uart_format_print+0x278>
   8082c: 7101d41f     	cmp	w0, #117
   80830: 54000200     	b.eq	0x80870 <uart_format_print+0xa4>
   80834: 7101d41f     	cmp	w0, #117
   80838: 5400106c     	b.gt	0x80a44 <uart_format_print+0x278>
   8083c: 7101cc1f     	cmp	w0, #115
   80840: 54000c80     	b.eq	0x809d0 <uart_format_print+0x204>
   80844: 7101cc1f     	cmp	w0, #115
   80848: 54000fec     	b.gt	0x80a44 <uart_format_print+0x278>
   8084c: 7101901f     	cmp	w0, #100
   80850: 540004c0     	b.eq	0x808e8 <uart_format_print+0x11c>
   80854: 7101901f     	cmp	w0, #100
   80858: 54000f6c     	b.gt	0x80a44 <uart_format_print+0x278>
   8085c: 7100001f     	cmp	w0, #0
   80860: 54001040     	b.eq	0x80a68 <uart_format_print+0x29c>
   80864: 7100941f     	cmp	w0, #37
   80868: 54000e60     	b.eq	0x80a34 <uart_format_print+0x268>
   8086c: 14000076     	b	0x80a44 <uart_format_print+0x278>
   80870: b9401a61     	ldr	w1, [x19, #24]
   80874: f9400260     	ldr	x0, [x19]
   80878: 7100003f     	cmp	w1, #0
   8087c: 540000ab     	b.lt	0x80890 <uart_format_print+0xc4>
   80880: 91002c01     	add	x1, x0, #11
   80884: 927df021     	and	x1, x1, #0xfffffffffffffff8
   80888: f9000261     	str	x1, [x19]
   8088c: 1400000d     	b	0x808c0 <uart_format_print+0xf4>
   80890: 11002022     	add	w2, w1, #8
   80894: b9001a62     	str	w2, [x19, #24]
   80898: b9401a62     	ldr	w2, [x19, #24]
   8089c: 7100005f     	cmp	w2, #0
   808a0: 540000ad     	b.le	0x808b4 <uart_format_print+0xe8>
   808a4: 91002c01     	add	x1, x0, #11
   808a8: 927df021     	and	x1, x1, #0xfffffffffffffff8
   808ac: f9000261     	str	x1, [x19]
   808b0: 14000004     	b	0x808c0 <uart_format_print+0xf4>
   808b4: f9400662     	ldr	x2, [x19, #8]
   808b8: 93407c20     	sxtw	x0, w1
   808bc: 8b000040     	add	x0, x2, x0
   808c0: b9400000     	ldr	w0, [x0]
   808c4: 9100c3e1     	add	x1, sp, #48
   808c8: aa0103e2     	mov	x2, x1
   808cc: 52800141     	mov	w1, #10
   808d0: 940000bf     	bl	0x80bcc <ui2a>
   808d4: 9100c3e0     	add	x0, sp, #48
   808d8: aa0003e1     	mov	x1, x0
   808dc: f94017e0     	ldr	x0, [sp, #40]
   808e0: 97ffffa6     	bl	0x80778 <uart_puts>
   808e4: 14000058     	b	0x80a44 <uart_format_print+0x278>
   808e8: b9401a61     	ldr	w1, [x19, #24]
   808ec: f9400260     	ldr	x0, [x19]
   808f0: 7100003f     	cmp	w1, #0
   808f4: 540000ab     	b.lt	0x80908 <uart_format_print+0x13c>
   808f8: 91002c01     	add	x1, x0, #11
   808fc: 927df021     	and	x1, x1, #0xfffffffffffffff8
   80900: f9000261     	str	x1, [x19]
   80904: 1400000d     	b	0x80938 <uart_format_print+0x16c>
   80908: 11002022     	add	w2, w1, #8
   8090c: b9001a62     	str	w2, [x19, #24]
   80910: b9401a62     	ldr	w2, [x19, #24]
   80914: 7100005f     	cmp	w2, #0
   80918: 540000ad     	b.le	0x8092c <uart_format_print+0x160>
   8091c: 91002c01     	add	x1, x0, #11
   80920: 927df021     	and	x1, x1, #0xfffffffffffffff8
   80924: f9000261     	str	x1, [x19]
   80928: 14000004     	b	0x80938 <uart_format_print+0x16c>
   8092c: f9400662     	ldr	x2, [x19, #8]
   80930: 93407c20     	sxtw	x0, w1
   80934: 8b000040     	add	x0, x2, x0
   80938: b9400000     	ldr	w0, [x0]
   8093c: 9100c3e1     	add	x1, sp, #48
   80940: 940000e7     	bl	0x80cdc <i2a>
   80944: 9100c3e0     	add	x0, sp, #48
   80948: aa0003e1     	mov	x1, x0
   8094c: f94017e0     	ldr	x0, [sp, #40]
   80950: 97ffff8a     	bl	0x80778 <uart_puts>
   80954: 1400003c     	b	0x80a44 <uart_format_print+0x278>
   80958: b9401a61     	ldr	w1, [x19, #24]
   8095c: f9400260     	ldr	x0, [x19]
   80960: 7100003f     	cmp	w1, #0
   80964: 540000ab     	b.lt	0x80978 <uart_format_print+0x1ac>
   80968: 91002c01     	add	x1, x0, #11
   8096c: 927df021     	and	x1, x1, #0xfffffffffffffff8
   80970: f9000261     	str	x1, [x19]
   80974: 1400000d     	b	0x809a8 <uart_format_print+0x1dc>
   80978: 11002022     	add	w2, w1, #8
   8097c: b9001a62     	str	w2, [x19, #24]
   80980: b9401a62     	ldr	w2, [x19, #24]
   80984: 7100005f     	cmp	w2, #0
   80988: 540000ad     	b.le	0x8099c <uart_format_print+0x1d0>
   8098c: 91002c01     	add	x1, x0, #11
   80990: 927df021     	and	x1, x1, #0xfffffffffffffff8
   80994: f9000261     	str	x1, [x19]
   80998: 14000004     	b	0x809a8 <uart_format_print+0x1dc>
   8099c: f9400662     	ldr	x2, [x19, #8]
   809a0: 93407c20     	sxtw	x0, w1
   809a4: 8b000040     	add	x0, x2, x0
   809a8: b9400000     	ldr	w0, [x0]
   809ac: 9100c3e1     	add	x1, sp, #48
   809b0: aa0103e2     	mov	x2, x1
   809b4: 52800201     	mov	w1, #16
   809b8: 94000085     	bl	0x80bcc <ui2a>
   809bc: 9100c3e0     	add	x0, sp, #48
   809c0: aa0003e1     	mov	x1, x0
   809c4: f94017e0     	ldr	x0, [sp, #40]
   809c8: 97ffff6c     	bl	0x80778 <uart_puts>
   809cc: 1400001e     	b	0x80a44 <uart_format_print+0x278>
   809d0: b9401a61     	ldr	w1, [x19, #24]
   809d4: f9400260     	ldr	x0, [x19]
   809d8: 7100003f     	cmp	w1, #0
   809dc: 540000ab     	b.lt	0x809f0 <uart_format_print+0x224>
   809e0: 91003c01     	add	x1, x0, #15
   809e4: 927df021     	and	x1, x1, #0xfffffffffffffff8
   809e8: f9000261     	str	x1, [x19]
   809ec: 1400000d     	b	0x80a20 <uart_format_print+0x254>
   809f0: 11002022     	add	w2, w1, #8
   809f4: b9001a62     	str	w2, [x19, #24]
   809f8: b9401a62     	ldr	w2, [x19, #24]
   809fc: 7100005f     	cmp	w2, #0
   80a00: 540000ad     	b.le	0x80a14 <uart_format_print+0x248>
   80a04: 91003c01     	add	x1, x0, #15
   80a08: 927df021     	and	x1, x1, #0xfffffffffffffff8
   80a0c: f9000261     	str	x1, [x19]
   80a10: 14000004     	b	0x80a20 <uart_format_print+0x254>
   80a14: f9400662     	ldr	x2, [x19, #8]
   80a18: 93407c20     	sxtw	x0, w1
   80a1c: 8b000040     	add	x0, x2, x0
   80a20: f9400000     	ldr	x0, [x0]
   80a24: aa0003e1     	mov	x1, x0
   80a28: f94017e0     	ldr	x0, [sp, #40]
   80a2c: 97ffff53     	bl	0x80778 <uart_puts>
   80a30: 14000005     	b	0x80a44 <uart_format_print+0x278>
   80a34: 3940ffe1     	ldrb	w1, [sp, #63]
   80a38: f94017e0     	ldr	x0, [sp, #40]
   80a3c: 97ffff1a     	bl	0x806a4 <uart_putc>
   80a40: d503201f     	nop
   80a44: f94013e0     	ldr	x0, [sp, #32]
   80a48: 91000401     	add	x1, x0, #1
   80a4c: f90013e1     	str	x1, [sp, #32]
   80a50: 39400000     	ldrb	w0, [x0]
   80a54: 3900ffe0     	strb	w0, [sp, #63]
   80a58: 3940ffe0     	ldrb	w0, [sp, #63]
   80a5c: 7100001f     	cmp	w0, #0
   80a60: 54ffec41     	b.ne	0x807e8 <uart_format_print+0x1c>
   80a64: 14000002     	b	0x80a6c <uart_format_print+0x2a0>
   80a68: d503201f     	nop
   80a6c: f9400bf3     	ldr	x19, [sp, #16]
   80a70: a8c47bfd     	ldp	x29, x30, [sp], #64
   80a74: d65f03c0     	ret

0000000000080a78 <uart_printf>:
   80a78: a9b77bfd     	stp	x29, x30, [sp, #-144]!
   80a7c: 910003fd     	mov	x29, sp
   80a80: f9001fe0     	str	x0, [sp, #56]
   80a84: f9001be1     	str	x1, [sp, #48]
   80a88: f90033e2     	str	x2, [sp, #96]
   80a8c: f90037e3     	str	x3, [sp, #104]
   80a90: f9003be4     	str	x4, [sp, #112]
   80a94: f9003fe5     	str	x5, [sp, #120]
   80a98: f90043e6     	str	x6, [sp, #128]
   80a9c: f90047e7     	str	x7, [sp, #136]
   80aa0: 910243e0     	add	x0, sp, #144
   80aa4: f90023e0     	str	x0, [sp, #64]
   80aa8: 910243e0     	add	x0, sp, #144
   80aac: f90027e0     	str	x0, [sp, #72]
   80ab0: 910183e0     	add	x0, sp, #96
   80ab4: f9002be0     	str	x0, [sp, #80]
   80ab8: 128005e0     	mov	w0, #-48
   80abc: b9005be0     	str	w0, [sp, #88]
   80ac0: b9005fff     	str	wzr, [sp, #92]
   80ac4: a94407e0     	ldp	x0, x1, [sp, #64]
   80ac8: a90107e0     	stp	x0, x1, [sp, #16]
   80acc: a94507e0     	ldp	x0, x1, [sp, #80]
   80ad0: a90207e0     	stp	x0, x1, [sp, #32]
   80ad4: 910043e0     	add	x0, sp, #16
   80ad8: aa0003e2     	mov	x2, x0
   80adc: f9401be1     	ldr	x1, [sp, #48]
   80ae0: f9401fe0     	ldr	x0, [sp, #56]
   80ae4: 97ffff3a     	bl	0x807cc <uart_format_print>
   80ae8: d503201f     	nop
   80aec: a8c97bfd     	ldp	x29, x30, [sp], #144
   80af0: d65f03c0     	ret

0000000000080af4 <get_timerLO>:
   80af4: d10043ff     	sub	sp, sp, #16
   80af8: d2860001     	mov	x1, #12288
   80afc: f2bfc001     	movk	x1, #65024, lsl #16
   80b00: 52800080     	mov	w0, #4
   80b04: 2a0003e0     	mov	w0, w0
   80b08: 8b000020     	add	x0, x1, x0
   80b0c: b9400000     	ldr	w0, [x0]
   80b10: b9000fe0     	str	w0, [sp, #12]
   80b14: b9400fe0     	ldr	w0, [sp, #12]
   80b18: 910043ff     	add	sp, sp, #16
   80b1c: d65f03c0     	ret

0000000000080b20 <get_timerHI>:
   80b20: d10043ff     	sub	sp, sp, #16
   80b24: d2860001     	mov	x1, #12288
   80b28: f2bfc001     	movk	x1, #65024, lsl #16
   80b2c: 52800100     	mov	w0, #8
   80b30: 2a0003e0     	mov	w0, w0
   80b34: 8b000020     	add	x0, x1, x0
   80b38: b9400000     	ldr	w0, [x0]
   80b3c: b9000fe0     	str	w0, [sp, #12]
   80b40: b9400fe0     	ldr	w0, [sp, #12]
   80b44: 910043ff     	add	sp, sp, #16
   80b48: d65f03c0     	ret

0000000000080b4c <a2d>:
   80b4c: d10043ff     	sub	sp, sp, #16
   80b50: 39003fe0     	strb	w0, [sp, #15]
   80b54: 39403fe0     	ldrb	w0, [sp, #15]
   80b58: 7100bc1f     	cmp	w0, #47
   80b5c: 540000e9     	b.ls	0x80b78 <a2d+0x2c>
   80b60: 39403fe0     	ldrb	w0, [sp, #15]
   80b64: 7100e41f     	cmp	w0, #57
   80b68: 54000088     	b.hi	0x80b78 <a2d+0x2c>
   80b6c: 39403fe0     	ldrb	w0, [sp, #15]
   80b70: 5100c000     	sub	w0, w0, #48
   80b74: 14000014     	b	0x80bc4 <a2d+0x78>
   80b78: 39403fe0     	ldrb	w0, [sp, #15]
   80b7c: 7101801f     	cmp	w0, #96
   80b80: 540000e9     	b.ls	0x80b9c <a2d+0x50>
   80b84: 39403fe0     	ldrb	w0, [sp, #15]
   80b88: 7101981f     	cmp	w0, #102
   80b8c: 54000088     	b.hi	0x80b9c <a2d+0x50>
   80b90: 39403fe0     	ldrb	w0, [sp, #15]
   80b94: 51015c00     	sub	w0, w0, #87
   80b98: 1400000b     	b	0x80bc4 <a2d+0x78>
   80b9c: 39403fe0     	ldrb	w0, [sp, #15]
   80ba0: 7101001f     	cmp	w0, #64
   80ba4: 540000e9     	b.ls	0x80bc0 <a2d+0x74>
   80ba8: 39403fe0     	ldrb	w0, [sp, #15]
   80bac: 7101181f     	cmp	w0, #70
   80bb0: 54000088     	b.hi	0x80bc0 <a2d+0x74>
   80bb4: 39403fe0     	ldrb	w0, [sp, #15]
   80bb8: 5100dc00     	sub	w0, w0, #55
   80bbc: 14000002     	b	0x80bc4 <a2d+0x78>
   80bc0: 12800000     	mov	w0, #-1
   80bc4: 910043ff     	add	sp, sp, #16
   80bc8: d65f03c0     	ret

0000000000080bcc <ui2a>:
   80bcc: d10083ff     	sub	sp, sp, #32
   80bd0: b9000fe0     	str	w0, [sp, #12]
   80bd4: b9000be1     	str	w1, [sp, #8]
   80bd8: f90003e2     	str	x2, [sp]
   80bdc: b9001fff     	str	wzr, [sp, #28]
   80be0: 52800020     	mov	w0, #1
   80be4: b9001be0     	str	w0, [sp, #24]
   80be8: 14000005     	b	0x80bfc <ui2a+0x30>
   80bec: b9401be1     	ldr	w1, [sp, #24]
   80bf0: b9400be0     	ldr	w0, [sp, #8]
   80bf4: 1b007c20     	mul	w0, w1, w0
   80bf8: b9001be0     	str	w0, [sp, #24]
   80bfc: b9400fe1     	ldr	w1, [sp, #12]
   80c00: b9401be0     	ldr	w0, [sp, #24]
   80c04: 1ac00820     	udiv	w0, w1, w0
   80c08: b9400be1     	ldr	w1, [sp, #8]
   80c0c: 6b00003f     	cmp	w1, w0
   80c10: 54fffee9     	b.ls	0x80bec <ui2a+0x20>
   80c14: 1400002a     	b	0x80cbc <ui2a+0xf0>
   80c18: b9400fe1     	ldr	w1, [sp, #12]
   80c1c: b9401be0     	ldr	w0, [sp, #24]
   80c20: 1ac00820     	udiv	w0, w1, w0
   80c24: b90017e0     	str	w0, [sp, #20]
   80c28: b9400fe0     	ldr	w0, [sp, #12]
   80c2c: b9401be1     	ldr	w1, [sp, #24]
   80c30: 1ac10802     	udiv	w2, w0, w1
   80c34: b9401be1     	ldr	w1, [sp, #24]
   80c38: 1b017c41     	mul	w1, w2, w1
   80c3c: 4b010000     	sub	w0, w0, w1
   80c40: b9000fe0     	str	w0, [sp, #12]
   80c44: b9401be1     	ldr	w1, [sp, #24]
   80c48: b9400be0     	ldr	w0, [sp, #8]
   80c4c: 1ac00820     	udiv	w0, w1, w0
   80c50: b9001be0     	str	w0, [sp, #24]
   80c54: b9401fe0     	ldr	w0, [sp, #28]
   80c58: 7100001f     	cmp	w0, #0
   80c5c: 540000e1     	b.ne	0x80c78 <ui2a+0xac>
   80c60: b94017e0     	ldr	w0, [sp, #20]
   80c64: 7100001f     	cmp	w0, #0
   80c68: 5400008c     	b.gt	0x80c78 <ui2a+0xac>
   80c6c: b9401be0     	ldr	w0, [sp, #24]
   80c70: 7100001f     	cmp	w0, #0
   80c74: 54000241     	b.ne	0x80cbc <ui2a+0xf0>
   80c78: b94017e0     	ldr	w0, [sp, #20]
   80c7c: 7100241f     	cmp	w0, #9
   80c80: 5400006c     	b.gt	0x80c8c <ui2a+0xc0>
   80c84: 52800601     	mov	w1, #48
   80c88: 14000002     	b	0x80c90 <ui2a+0xc4>
   80c8c: 52800ae1     	mov	w1, #87
   80c90: b94017e0     	ldr	w0, [sp, #20]
   80c94: 12001c02     	and	w2, w0, #0xff
   80c98: f94003e0     	ldr	x0, [sp]
   80c9c: 91000403     	add	x3, x0, #1
   80ca0: f90003e3     	str	x3, [sp]
   80ca4: 0b020021     	add	w1, w1, w2
   80ca8: 12001c21     	and	w1, w1, #0xff
   80cac: 39000001     	strb	w1, [x0]
   80cb0: b9401fe0     	ldr	w0, [sp, #28]
   80cb4: 11000400     	add	w0, w0, #1
   80cb8: b9001fe0     	str	w0, [sp, #28]
   80cbc: b9401be0     	ldr	w0, [sp, #24]
   80cc0: 7100001f     	cmp	w0, #0
   80cc4: 54fffaa1     	b.ne	0x80c18 <ui2a+0x4c>
   80cc8: f94003e0     	ldr	x0, [sp]
   80ccc: 3900001f     	strb	wzr, [x0]
   80cd0: d503201f     	nop
   80cd4: 910083ff     	add	sp, sp, #32
   80cd8: d65f03c0     	ret

0000000000080cdc <i2a>:
   80cdc: a9be7bfd     	stp	x29, x30, [sp, #-32]!
   80ce0: 910003fd     	mov	x29, sp
   80ce4: b9001fe0     	str	w0, [sp, #28]
   80ce8: f9000be1     	str	x1, [sp, #16]
   80cec: b9401fe0     	ldr	w0, [sp, #28]
   80cf0: 7100001f     	cmp	w0, #0
   80cf4: 5400012a     	b.ge	0x80d18 <i2a+0x3c>
   80cf8: b9401fe0     	ldr	w0, [sp, #28]
   80cfc: 4b0003e0     	neg	w0, w0
   80d00: b9001fe0     	str	w0, [sp, #28]
   80d04: f9400be0     	ldr	x0, [sp, #16]
   80d08: 91000401     	add	x1, x0, #1
   80d0c: f9000be1     	str	x1, [sp, #16]
   80d10: 528005a1     	mov	w1, #45
   80d14: 39000001     	strb	w1, [x0]
   80d18: b9401fe0     	ldr	w0, [sp, #28]
   80d1c: f9400be2     	ldr	x2, [sp, #16]
   80d20: 52800141     	mov	w1, #10
   80d24: 97ffffaa     	bl	0x80bcc <ui2a>
   80d28: d503201f     	nop
   80d2c: a8c27bfd     	ldp	x29, x30, [sp], #32
   80d30: d65f03c0     	ret

0000000000080d34 <memset>:
   80d34: d100c3ff     	sub	sp, sp, #48
   80d38: f9000fe0     	str	x0, [sp, #24]
   80d3c: b90017e1     	str	w1, [sp, #20]
   80d40: f90007e2     	str	x2, [sp, #8]
   80d44: f9400fe0     	ldr	x0, [sp, #24]
   80d48: f90017e0     	str	x0, [sp, #40]
   80d4c: 1400000a     	b	0x80d74 <memset+0x40>
   80d50: f94017e0     	ldr	x0, [sp, #40]
   80d54: 91000401     	add	x1, x0, #1
   80d58: f90017e1     	str	x1, [sp, #40]
   80d5c: b94017e1     	ldr	w1, [sp, #20]
   80d60: 12001c21     	and	w1, w1, #0xff
   80d64: 39000001     	strb	w1, [x0]
   80d68: f94007e0     	ldr	x0, [sp, #8]
   80d6c: d1000400     	sub	x0, x0, #1
   80d70: f90007e0     	str	x0, [sp, #8]
   80d74: f94007e0     	ldr	x0, [sp, #8]
   80d78: f100001f     	cmp	x0, #0
   80d7c: 54fffea1     	b.ne	0x80d50 <memset+0x1c>
   80d80: f9400fe0     	ldr	x0, [sp, #24]
   80d84: 9100c3ff     	add	sp, sp, #48
   80d88: d65f03c0     	ret

0000000000080d8c <memcpy>:
   80d8c: d10103ff     	sub	sp, sp, #64
   80d90: f9000fe0     	str	x0, [sp, #24]
   80d94: f9000be1     	str	x1, [sp, #16]
   80d98: f90007e2     	str	x2, [sp, #8]
   80d9c: f9400be0     	ldr	x0, [sp, #16]
   80da0: f9001fe0     	str	x0, [sp, #56]
   80da4: f9400fe0     	ldr	x0, [sp, #24]
   80da8: f9001be0     	str	x0, [sp, #48]
   80dac: f90017ff     	str	xzr, [sp, #40]
   80db0: 1400000c     	b	0x80de0 <memcpy+0x54>
   80db4: f9401fe1     	ldr	x1, [sp, #56]
   80db8: 91000420     	add	x0, x1, #1
   80dbc: f9001fe0     	str	x0, [sp, #56]
   80dc0: f9401be0     	ldr	x0, [sp, #48]
   80dc4: 91000402     	add	x2, x0, #1
   80dc8: f9001be2     	str	x2, [sp, #48]
   80dcc: 39400021     	ldrb	w1, [x1]
   80dd0: 39000001     	strb	w1, [x0]
   80dd4: f94017e0     	ldr	x0, [sp, #40]
   80dd8: 91000400     	add	x0, x0, #1
   80ddc: f90017e0     	str	x0, [sp, #40]
   80de0: f94017e1     	ldr	x1, [sp, #40]
   80de4: f94007e0     	ldr	x0, [sp, #8]
   80de8: eb00003f     	cmp	x1, x0
   80dec: 54fffe43     	b.lo	0x80db4 <memcpy+0x28>
   80df0: f9400fe0     	ldr	x0, [sp, #24]
   80df4: 910103ff     	add	sp, sp, #64
   80df8: d65f03c0     	ret
