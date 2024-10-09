/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF A
***************************************************************************************/

#include <common.h>
#include <time.h>
void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

word_t expr(char *e, bool *success);
int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif	
  /* Start engine. */
	engine_start();
//	FILE *fp = fopen("tools/gen-expr/input", "r");
//	assert(fp != NULL);
//	uint32_t num = 0;
//	char expr_[65536];
//	bool *success = malloc(1);
//	int count_error = 0;
//	int count = 0;
//	while (	fscanf(fp, "%u %65536[^\n]",&num, expr_) != EOF) {	
//	time_t start = time(NULL);
//	word_t re = expr(expr_, success);	
//	time_t end = time(NULL);
//	if (num != re){
//		printf("wrong!%s,\ngot%u,\nreal%u\n",expr_,re, num);
//		count_error++;
//		} else {
//		printf("expression%d: passed\n", ++count);
//		printf("%ld\n", end - start);
//		}
//	}
//	fclose(fp);	
//	printf("pass = %d\n", count - count_error);
//	printf("error = %d\n", count_error);
  return is_exit_status_bad();
}
