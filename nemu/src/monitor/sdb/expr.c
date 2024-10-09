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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

word_t get_reg_val(const char *name);
enum {
  TK_NOTYPE = 256, TK_EQ,TK_NEQ,

  /* TODO: Add more token types */
	TK_NUM,
	TK_NE,//Negative
	TK_HNUM,
	TK_RG,
	TK_DEREFE,
	TK_LM,
	TK_RM,
	TK_CAND,
	TK_COR,
	TK_LESSEQ,
	TK_MOREEQ,

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
	{" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"\\-", '-'},					// minus/negative
	{"\\*", '*'},					// multiply 
	{"/", '/'},						// divide
	{"==", TK_EQ},        // equal
	{"!=", TK_NEQ},				// not equal 
	{"&&", TK_CAND},			// C and
	{"\\|\\|", TK_COR},		// C or 
	{"<=", TK_LESSEQ},		// less or equal
	{">=", TK_MOREEQ},		// more or equal
	{"<<", TK_LM},				// left move
  {">>", TK_RM},				// right move
	{"<", '<'},						// less
	{">", '>'},						// more  
	{"!", '!'},						// not										
	{"\\(", '('},					// left bracket
	{"\\)", ')'},					// right bracket
	{"\\${1,2}[0-9arspgt]{1,3}", TK_RG},			// register
	{"0x[0-9a-fA-F]+",TK_HNUM},	// hexadecimal numberr
	{"[0-9]+", TK_NUM},		// demical number
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

word_t paddr_read(paddr_t addr, int len);
/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[65536] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            //i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          //default: TODO();
					case TK_NUM :
					case TK_HNUM:
					case TK_RG:	
						tokens[nr_token].type = rules[i].token_type;
						if (substr_len < 32) {
							memcpy(tokens[nr_token++].str, substr_start, substr_len);
							tokens[nr_token - 1].str[substr_len] = '\0';
						} else {
							char *str = NULL;
							*str = 0;
							//TODO
							//assert(0);
							//tokens[nr_token].str[0] = '@'; // This is a sign for overflow
							//char **ptr = malloc(sizeof(char*));

							//*ptr = malloc(sizeof(char) * substr_len + 1);
							//memcpy(tokens[nr_token++].str + 24 , ptr, sizeof(char*)); //save ptr
							//strcpy(*ptr, substr_start);	
							//free(ptr);
						}
					case TK_NOTYPE:
						break;
					default:
							tokens[nr_token++].type = rules[i].token_type;

        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static void normalize() {
	for (int i = 0; i < nr_token; ++i) {
		if (tokens[i].type == '-') {
				if (i != 0 && (tokens[i - 1].type == TK_NUM || tokens[i - 1].type == ')' || tokens[i - 1].type == TK_HNUM || tokens[i - 1].type == TK_RG)) {
				// the '-' is the minus symbol, so keep it ;
				} else {
				// the following is to handle the negative occasion
					 tokens[i].type = TK_NE;
						
				}
				} else if (tokens[i].type == '*') {
					if (i == 0 || tokens[i - 1].type == '+' || tokens[i - 1].type == '-' ||
						tokens[i - 1].type == '*' || tokens[i - 1].type == '/' || tokens[i - 1].type == '(' ) {
							tokens[i].type = TK_DEREFE;
					}
				}
		}
	}		


static bool check_parentheses(int left, int right) {
	if ((tokens[left].type == '(') && tokens[right].type == ')') {
		++left;
		int count = 0;
		for (;left < right; left++) {
			if (tokens[left].type == '(')count++;
			else if(tokens[left].type == ')')count--;

			if (count < 0) return false;
		}
		return count == 0;
	} else {
		return false;
		
	}
	return false;
}


static uint32_t eval(int left, int right) {
	//assert(left >= 0 && right < nr_token);
	if (left > right) {
		assert(0);
	} else if (left == right) {
		switch (tokens[left].type) {
			case TK_NUM :
				return strtoul(tokens[left].str, NULL, 10);
			case TK_HNUM:
				return strtoul(tokens[left].str, NULL, 16);
			case TK_RG:	
				return get_reg_val(tokens[left].str);		
		}
	} else if (check_parentheses(left, right)) {
		return  eval(left + 1, right-1);
	} else {
		assert(tokens[right].type != '-' && tokens[right].type != '+' && tokens[right].type != '*' && tokens[right].type != '/');
			int low_pr = 1;//This is a trick to correctly get the sym; 
			int pos = left;
			int count = 0;
			for (int i = left;i < right; i++) {
					if (tokens[i].type == TK_NUM || tokens[i].type == TK_HNUM || tokens[i].type == TK_RG)continue;
				if (tokens[i].type == '(') {
					count++;
					continue;
				} else if (tokens[i].type == ')') {
					count--;
					continue;
				}
				if (count == 0) {
					if (low_pr == 0 && (tokens[i].type == TK_DEREFE || tokens[i].type =='!' || tokens[i].type == TK_NE)){
						pos = i;
					}	
					else if (low_pr <= 1 && (tokens[i].type == '*' || tokens[i].type == '/')) {
						pos = i;
						low_pr = 1;
					}	else if(low_pr <= 2 && (tokens[i].type ==	'+' || tokens[i].type == '-')) {
						pos = i;
						low_pr = 2;
					}
				 	else if (low_pr <= 3 && ((tokens[i].type == TK_LM) ||  tokens[i].type == TK_RM)) {
						pos = i;
						low_pr = 3;	
					}
					 	else if (low_pr <= 4 && ((tokens[i].type == '<') ||  tokens[i].type == '>' || tokens[i].type == TK_LESSEQ || tokens[i].type == TK_MOREEQ)) {
						pos = i;
						low_pr = 4;
					}
					 	else if (low_pr <= 5 && ((tokens[i].type == TK_EQ) || tokens[i].type == TK_NEQ)) {
						pos = i;
						low_pr = 5;
					}
					 	else if (low_pr <= 6 && ((tokens[i].type == '&'))) {
						pos = i;
						low_pr = 6;
					}
					 	else if (low_pr <= 7 && ((tokens[i].type == '|'))) {
						pos = i;
						low_pr = 7;
					}
					 	else if (low_pr <= 8 && ((tokens[i].type == TK_CAND))) {
						pos = i;
						low_pr = 8;
					} else if (low_pr <= 9 && (tokens[i].type == TK_COR)) {
						pos = i;
						low_pr = 9;
					}
				
				}
			}	
			switch(tokens[pos].type) {
				case '+':
					return eval(left, pos - 1) + eval(pos + 1, right);
				case '-':
					return eval(left, pos - 1) - eval(pos + 1, right);
				case '*':
					return eval(left, pos - 1) * eval(pos + 1, right);
				case '/':
					//assert(res != 0)
					return eval(left, pos - 1) / eval(pos + 1, right);
				case TK_EQ:
					return eval(left, pos - 1) == eval(pos + 1, right);
				case TK_CAND:
					return eval(left, pos - 1) && eval(pos + 1, right);
				case TK_COR:
					return eval(left, pos - 1) || eval(pos + 1 ,right);				
				case TK_LESSEQ:
					return eval(left, pos - 1) <= eval(pos + 1, right);
				case TK_MOREEQ:
					return eval(left, pos - 1) >= eval(pos + 1, right);
				case '<':
					return eval(left, pos - 1) < eval(pos + 1, right);
				case '>':
					return eval(left, pos - 1) > eval(pos + 1, right);	
				case '|':
					return eval(left, pos - 1) | eval(pos + 1 , right);
				case '&':
					return eval(left, pos - 1) & eval(pos + 1, right);	
				case TK_LM:
					return eval(left, pos - 1) << eval(pos + 1, right);
				case TK_RM:
					return eval(left, pos - 1) >> eval(pos + 1, right);
				case '!' :
						return !eval(pos + 1, right);
				case TK_NE:
						return -eval(pos + 1, right);
				case TK_DEREFE:
						return paddr_read(eval(pos + 1, right),4);

			}	
			
	
	}

			
	return 0;
}



word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
	normalize();
  /* TODO: Insert codes to evaluate the expression. */
	 

  return eval(0, nr_token - 1);
}
