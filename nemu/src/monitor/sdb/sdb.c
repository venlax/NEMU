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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"

static int is_batch_mode = false;
void init_regex();
word_t paddr_read(paddr_t addr, int len); 
/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
	}

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Let the program step through N instructions and then \npause execution, If N is not given, the default value is 1.", cmd_si },
	{ "info", "r: Print register status\n w: Print watch information.",cmd_info },
	{ "x", "Evaluate the expression EXPR and use the result as the starting memory Address,\n output N consecutive 4 bytes in hexadecimal form.",cmd_x },
	{"p","Give a expression, calculate the value of the expression.",cmd_p},
	{"w","Give a expression, when the value of the expression changes,\n program execution is suspended.",cmd_w},
	{"d","delete a watch with serial number N.",cmd_d},
};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char* args) {
	int steps = args == NULL ? 1 : atoi(strtok(NULL, " "));
	cpu_exec(steps);
	return 0;	
}


static int cmd_info(char* args) {
	if (args == NULL) {
		printf("Command needs the SUBCMD.\n");
		return 0;
	}
	char* arg = strtok(NULL, " ");
	if (strcmp(args, "r") == 0) {
		/**print register status**/
		isa_reg_display();
	} else if (strcmp(args, "w") == 0) {
		/**print watchpoint message**/
		print_wp_message();
	}else {
		printf("Unknown Subcommand %s\n", arg);	
	}
	return 0;
}

static int cmd_x(char* args) {
	if (args == NULL) {
		printf("No needed arguments\n");
		return 0;
	}	
	char *num = strtok(NULL, " ");
	int N = atol(num);
	char *expr_ = num + strlen(num) + 1;
	if (expr_ == NULL) {
		printf("No needed expression\n");
		return 0;
	}
  bool* success = malloc(sizeof(bool));
	word_t addr = expr(expr_, success);
	if (success) {
		for (int i = 0; i < N; ++i) {
		word_t result = paddr_read(addr + 4 * i, 4);		
		printf("0x%08x\n", result);
		}
	}	
	else {
		printf("Invalid Expression\n");
	}
	free(success);
	return 0;
}

static int cmd_p(char *args) {
	if (args == NULL) {
		printf("No needed expression");
		return 0;
		}
	bool* success = malloc(sizeof(bool));
	word_t val = expr(args, success);
	if (success) {
		printf("The value of the %s is %u\n", args, val);
	}	
	else {
		printf("Invalid Expression\n");
	}
	free(success);
	return 0;
}

static int cmd_w(char *args) {
	if (args == NULL) {return 0;}
	add_wp(args);
	return 0;
}

static int cmd_d(char *args) {
	if (args == NULL) {
		printf("Please provide the number\n");
		return 0;
	}
	int num = atoi(strtok(NULL, " "));

	if (delete_watchpoint(num)) {
		printf("delete the watchpoint succeed\n");
	}else {
		printf("delete the watchpoint fail\n");
	}
	return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
