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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
	uint32_t cur_val;
	char expr_[32];
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
void print_wp_message() {
		if (head == NULL) {
			printf("No watchpoints\n");
			return;
		}
		printf("Num      what            val  \n");
		WP* temp = head;
		int count = 1;
		while (temp != NULL) {
			printf("%-10d%-16s%-5u\n",count++,temp->expr_,temp->cur_val);
		}
}

WP* new_wp() {
	WP* result = free_;
	assert(result != NULL);
	free_ = free_->next;
	return result;
}



void free_wp(WP *wp) {
	wp->next = free_;
	free_ = wp;
}

void scan_wp() {
	WP* iter = head;
	bool *success = malloc(sizeof(bool));
	while (iter != NULL) {
		uint32_t val = expr(iter->expr_,success);
		assert(success);
		if (val != iter->cur_val) {
			nemu_state.state = NEMU_STOP;
			printf("catch the watchpoint %s, old_val : %u , new_val : %u \n",
					iter->expr_,iter->cur_val, val);
			iter->cur_val = val;
		}
	}
	free(success);
}

void add_wp(char *args) {
  WP *new_wp_ = new_wp();
	strcpy(new_wp_->expr_,args);
}

bool delete_watchpoint(int N){
	WP* temp = head;
	WP* PR = NULL;
	while (temp != NULL && N != 1) {
			PR = temp;
			temp = temp->next;
			N--;
	}
	if (temp == NULL) return false;
	if (PR == NULL) {
		head = temp->next;
	} else {
		PR->next = temp->next;
	}
	free_wp(temp);
	return true;
}
