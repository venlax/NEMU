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

#include <isa.h>
#include "local-include/reg.h"
#define NUM_REG 32

extern CPU_state cpu;
const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

//static uint32_t regs_val[NUM_REG] = {
	//0,0,0,0,0,0,0,0,
	//0,0,0,0,0,0,0,0,
	//0,0,0,0,0,0,0,0,
	//0,0,0,0,0,0,0,0
//};

word_t isa_reg_str2val(const char *s, bool *success);
void isa_reg_display() {
	for (int i = 0; i < NUM_REG; ++i){
			printf("reg: %-5s =  %5u  0x%x\n", regs[i], cpu.gpr[i],cpu.gpr[i]);
	}
}
static int str2reg_index(const char *s) {
	//printf("%d", strcmp(s + 1, regs[0]));
	for (int i = 0; i < NUM_REG; ++i) {
			if (strcmp(s + 1, regs[i]) == 0 || strcmp(s,regs[i]) == 0) {
					return i;
			} // +1 is to ignore the pre$
	}
	return -1;
}
word_t isa_reg_str2val(const char *s, bool *success) {
  int i = str2reg_index(s);
	*success = false;
	if (i != -1) {
			*success = true;
			return cpu.gpr[i];
	}
	char *str = NULL;
	*str = 5;
	return 0;
}

word_t get_reg_val(const char *name) {
	bool *success = malloc(sizeof(bool));
	word_t result = isa_reg_str2val(name, success);
	if (success) {
		free(success);
		return result;
	}else {
		free(success);
		return 0;
	}
}
