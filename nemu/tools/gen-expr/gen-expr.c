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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

void fpe_handler(int signum) {
	exit(1);
}

static uint32_t offset = 0;
static char op[4] = {'+', '-', '*', '/'};
uint32_t choose(uint32_t n){
		return rand() % n;
}

static uint32_t num_len(uint32_t num) {
		uint32_t len = 0;
		if (num == 0)len++;
		while (num != 0) {
			len++;
			num /= 10;
		}
		return len;
}

static void gen_num() {
	if (buf[offset] == '\0') {
		} else {
		offset++;
		}
	uint32_t num = abs(choose(1000000));
		sprintf(buf + offset, "%u", num);
		offset += num_len(num);
		buf[offset] = '\0';		
}


static void gen(char sym) {
		if (buf[offset] == '\0') {
		} else {
		offset++;
		}
		
	if (sym == '(' || sym == ')' || sym == '+' ||
				sym == '-' || sym == '/' || sym == '*'||
				sym == ' ') {
			buf[offset++] = sym;
		 	buf[offset] = '\0';	
		}
}


static void gen_rand_op () {
	gen(op[rand() % 4]);
}
/*
static void plus_1() {
		if (buf[offset] == '\0') {
		} else {
		offset++;
		}
	buf[offset++] = '(';
	buf[offset++] = '1';
	buf[offset++] = '+';
	buf[offset] = '\0';
}

static void multiply_2() {
	if (buf[offset] == '\0') {
		} else {
		offset++;
		}
	buf[offset++] = '*';
	buf[offset++] = '2';
	buf[offset++] = ')';
	buf[offset] = '\0';	
}
*/

static void gen_space() {
	switch(choose(2)){
		case 0:
			uint32_t times = choose(2);
			for (int i = 0; i < times; ++i){
				gen(' ');
			}
	}
}

static void gen_rand_expr() {
  if (offset >= 60000)return;
	switch (choose(3)) {
		case 0:gen_space();
					 gen_num();
										 break;
		case 1:gen_space();
					 gen('(');gen_rand_expr();
					 gen_space();
					 gen(')');gen_space();
										break;
		default:gen_space();
						gen_rand_expr();
						gen_space();
						gen_rand_op();
						gen_space();
						gen_rand_expr();
						gen_space();
						break;			 	
	}
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
	signal(SIGFPE, fpe_handler);
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf[0] = '\0';
		gen_rand_expr();
		if (offset >= 60000) {
			offset = 0;
			i--;
			continue;
		}
		offset = 0;
    sprintf(code_buf, code_format, buf);
		
    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -Werror=div-by-zero /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) {
			i--;
			continue;
		}
    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);
    int result;
    ret = fscanf(fp, "%d", &result);
    int status = pclose(fp);
		if (WIFEXITED(status) && WEXITSTATUS(status) == SIGFPE) {
			i--;
			continue;
		}
    printf("%u %s\n", result, buf);
  }
	//assert(count != loop);
  return 0;
}
