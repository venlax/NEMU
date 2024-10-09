/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

#define CSRRW_OPE() do {\
	word_t temp = 0;\
	word_t imm_num = imm & 0xfff;\
	switch(imm_num) {\
		case 0x305 : temp = cpu.csr.mtvec;cpu.csr.mtvec = src1;break;\
		case 0x300 : temp = cpu.csr.mstatus;cpu.csr.mstatus = src1;break;\
		case 0x341 : temp = cpu.csr.mepc;cpu.csr.mepc = src1;break;\
		case 0x342 : temp = cpu.csr.mcause;cpu.csr.mcause=src1;break;						 }\
	R(rd) = temp;\
} while(0);

#define CSRRS_OPE() do {\
	word_t temp = 0;\
	word_t imm_num = imm & 0xfff;\
	switch(imm_num) {\
		case 0x305 : temp = cpu.csr.mtvec;cpu.csr.mtvec |= src1;break;\
		case 0x300 : temp = cpu.csr.mstatus;cpu.csr.mstatus |= src1;break;\
		case 0x341 : temp = cpu.csr.mepc;cpu.csr.mepc |= src1;break;\
		case 0x342 : temp = cpu.csr.mcause;cpu.csr.mcause |= src1;break;						 }\
	R(rd) = temp;\
} while(0);


#ifdef CONFIG_FTRACE
void ftrace(vaddr_t pc, vaddr_t func_addr ,uint8_t reg);
#endif

#ifdef CONFIG_FTRACE
#define FTRACE(pc,addr, reg) ftrace(pc, addr,reg);
#else
#define FTRACE(pc,addr, reg)  // nop instruc
#endif

#ifdef CONFIG_ETRACE
#define ETRACE() printf("address : 0x%x exception: 0x%x, goto 0x%x \n",s->pc,cpu.csr.mcause,cpu.csr.mtvec);
#define ETRACE_RET() printf("address : 0x%x return from exception 0x%x handle , set pc to 0x%x\n",s->pc,cpu.csr.mcause,s->dnpc);
#else
#define ETRACE()
#define ETRACE_RET()
#endif

enum {
  TYPE_I, TYPE_U, TYPE_S,
  TYPE_N, TYPE_J, TYPE_B,
	TYPE_R,// none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immJ() do { *imm = ((SEXT(BITS(i, 31, 31), 1) << 19) | (BITS(i,19,12) << 11 ) | (BITS(i,20,20) << 10) | BITS(i,30,21)) << 1;} while(0)
#define immB() do { *imm = ((SEXT(BITS(i,31,31), 1) << 11 ) | (BITS(i,7,7) << 10 ) | (BITS(i, 30 , 25) << 4) | (BITS(i, 11 , 8)) ) << 1;} while(0);
static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
  	case TYPE_J: immJ(); break;
		case TYPE_B: src1R(); src2R(); immB(); break;
		case TYPE_R: src1R(); src2R(); break;
	}
}

static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;
	
  assert(cpu.pc != 0);
#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  INSTPAT_START();
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm);
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));

  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  // my add inst:
	INSTPAT("???????????????????? ????? 01101 11", lui, U, R(rd) = imm);	
	// lui
	
	INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add,R, R(rd) = src1 + src2);
	// add
	
	INSTPAT("???????????? ????? 000 ????? 00100 11", addi , I , R(rd) = src1  + imm);
	// addi |  li | mov | zext.b

	INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub,R, R(rd) = src1 - src2);
	// sub

	INSTPAT("0000001 ????? ????? 000 ????? 01100 11",mul,R,R(rd) = (src1 * src2));
	// mul

	INSTPAT("0000001 ????? ????? 001 ????? 01100 11",mulh,R,R(rd) = (SEXT(src1,32) * SEXT(src2,32)) >> 32);
	// mulh
	
	INSTPAT("0000001 ????? ????? 011 ????? 01100 11",mulhu,R,R(rd) = ((uint64_t)src1 * (uint64_t)src2) >> 32);
	// mulhu
	
	INSTPAT("0000001 ????? ????? 100 ????? 01100 11",div,R,src1 = src2 == 0  ? 0x80000000 : src1;src2 = src2 == 0 ? -1 : src2;R(rd) =(word_t) ((int32_t)src1 / (int32_t)src2));
	// div
	
	
	INSTPAT("0000001 ????? ????? 101 ????? 01100 11",divu,R,assert(src2 != 0);R(rd) = src1 / src2);
	// divu

	INSTPAT("0000001 ????? ????? 110 ????? 01100 11",rem,R,assert(src2 != 0);R(rd) = (word_t)((int32_t)src1 % (int32_t)src2));
	// rem

	INSTPAT("0000001 ????? ????? 111 ????? 01100 11",remu,R,assert(src2 != 0);R(rd)= src1 % src2);
	// remu

	INSTPAT("0100000 ????? ????? 101 ????? 01100 11",sra,R,R(rd) = (word_t) ((int32_t)src1 >> (src2 & 0x1f)));
	// sra

	INSTPAT("0100000 ????? ????? 101 ????? 00100 11", srai,I, R(rd) = (word_t)(((int32_t)src1) >> (imm & 0x1f)));
	// srai | sraiw
	
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl,R,R(rd) =src1 >> (src2 & 0x1f));
	// srl

	INSTPAT("000000 ????? ????? 001 ????? 00100 11", slli,I, R(rd) = src1 << (imm & 0x1f));
	// slli | slliw

	INSTPAT("000000 ????? ????? 101 ????? 00100 11", srli,I, R(rd) = src1 >> (imm & 0x1f));
	// srli | srliw
	
	INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and, R, R(rd) = src1 & src2);
	// and

	INSTPAT("???????????? ????? 111 ????? 00100 11", andi,I, R(rd) = src1 & imm);
	// andi
	INSTPAT("???????????? ????? 110 ????? 00100 11", ori,I, R(rd) = src1 | imm);
	// ori


	INSTPAT("0000000 ????? ????? 100 ????? 01100 11",xor,R, R(rd) = src1 ^ src2);
	// xor
	
	INSTPAT("???????????? ????? 100 ????? 00100 11",xori,I, R(rd) = src1 ^ imm);
	// xori
	

	INSTPAT("0000000 ????? ????? 110 ????? 01100 11",or,R, R(rd) = src1 | src2);
	// or
	
	INSTPAT("???????????????????? ????? 11011 11",jal, J,
		R(rd) = s->pc + 4; s->dnpc = s->pc + imm; FTRACE(s->pc,s->dnpc, 0xff);assert(s->dnpc != 0););
	// jal | j 0xff is used to distinguish jal && jalr

  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw, S, Mw(src1 + imm, 4, src2));
	// sw
	
	INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh, S, Mw(src1 + imm, 2, src2));
	// sh

	INSTPAT("???????????? ????? 000 ????? 11001 11", jalr, I, R(rd) = s->pc + 4;s->dnpc = src1 + imm; FTRACE(s->pc, s->dnpc, BITS(s->isa.inst.val,19, 15)));
	// jr | jalr | ret

	INSTPAT("???????????? ????? 010 ????? 00000 11", lw, I, R(rd) = Mr(src1 + imm, 4));
	// lw

	INSTPAT("???????????? ????? 001 ????? 00000 11", lh, I, R(rd) = SEXT(Mr(src1 + imm, 2),16));
	// lh
	
	INSTPAT("???????????? ????? 101 ????? 00000 11", lhu, I , R(rd) = Mr(src1 + imm , 2) & 0x0000ffff);
	// lhu

	INSTPAT("???????????? ????? 100 ????? 00000 11",lbu,I, R(rd) = Mr(src1 + imm, 1) & 0x000000ff);
	// lbu
	
	INSTPAT("???????????? ????? 000 ????? 00000 11", lb,I, R(rd) = SEXT(Mr(src1 + imm, 1),8));
	// lb

	INSTPAT("0000000 ????? ????? 001 ????? 01100 11",sll,R, R(rd) = src1 << (src2 & 0x1f));
	// sll

	INSTPAT("??????? ????? ????? 100 ????? 11000 11",blt,B, s->dnpc = (int32_t) src1 < (int32_t) src2 ? s->pc + imm : s->dnpc);
	// blt

	INSTPAT("??????? ????? ????? 110 ????? 11000 11",blt,B, s->dnpc =  src1 <  src2 ? s->pc + imm : s->dnpc);
	// bltu

	INSTPAT("0000000 ????? ????? 011 ????? 01100 11",sltu, R, R(rd) = src1 < src2 ? 1 : 0);
	// sltu | snez
	
	INSTPAT("0000000 ????? ????? 010 ????? 01100 11",slt, R, R(rd) = (int32_t) src1 < (int32_t)src2 ? 1 : 0);
	// slt | sltz | sgtz

	INSTPAT("???????????? ????? 010 ????? 0010011", slti, I, R(rd) = (int32_t)src1 < (int32_t)imm ? 1 : 0);
	// slti

	INSTPAT("???????????? ????? 011 ????? 0010011", sltiu, I, R(rd) = src1 < imm ? 1 : 0);
	// sltiu | seqz
	
	INSTPAT("??????? ????? ????? 001 ????? 11000 11",bne, B, s->dnpc = (src1 != src2) ? s->pc + imm : s->dnpc);
	// bne | bnez
	
	
	INSTPAT("??????? ????? ????? 000 ????? 11000 11",beq, B, s->dnpc = (src1 == src2) ? s->pc + imm : s->dnpc);
	// beq | beqz

	INSTPAT("??????? ????? ????? 101 ????? 11000 11",bge, B, s->dnpc = ((int32_t)src1 >= (int32_t)src2) ? s->pc + imm : s->dnpc);
	// bge | blez
	
	INSTPAT("??????? ????? ????? 111 ????? 11000 11",bgeu, B, s->dnpc = (src1 >= src2) ? s->pc + imm : s->dnpc);
	// bgeu

	INSTPAT("???????????? ????? 001 ????? 1110011",csrrw, I, CSRRW_OPE() );
	// csrrw

	INSTPAT("???????????? ????? 010 ????? 1110011",csrrs, I, CSRRS_OPE());
  // csrrs csrr
	
  INSTPAT("0011000 00010 00000 000 00000 1110011",mret, R, s->dnpc = cpu.csr.mepc + 4  ; ETRACE_RET());
	// mret

	INSTPAT("000000000000 00000 000 00000 1110011",ecall, I,s->dnpc = isa_raise_intr(cpu.gpr[17],s->pc); ETRACE());
	// ecall

	// add inst end
	INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  
	INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  //printf("%u\n%d\n",imm,imm);
	return 0;
}

int isa_exec_once(Decode *s) {
 	 
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
