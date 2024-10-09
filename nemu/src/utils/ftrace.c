#include <common.h>
#ifdef CONFIG_FTRACE 
// #include <string.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <assert.h>
#include <elf.h>
char *strtab_data = NULL;

struct {
	Elf32_Sym *sym_arr;
	size_t num;
} sym_table;

struct func_call_message{
	char str[32];
	vaddr_t addr;
	vaddr_t return_addr;
};	 
struct {
	struct func_call_message stack[512];
	int cur_ptr;
}func_call_stack;

static void init_func_call_stack() {
	func_call_stack.cur_ptr = -1;
}

void init_ftrace (const char *elf_file) {
	
	init_func_call_stack();

	Elf32_Ehdr header;
	FILE* fp_elf = fopen(elf_file, "rb");
	if (fp_elf) {
		
		// first read the ELF head
		size_t size = fread(&header, sizeof(header), 1 , fp_elf);
		assert(size > 0 && memcmp(header.e_ident, ELFMAG, SELFMAG) == 0);
		// locate the section header
		fseek(fp_elf,header.e_shoff, SEEK_SET);

		Elf32_Shdr shdr;
		// read the section header
		for (int i = 0; i < header.e_shnum; ++i) {
			size = fread(&shdr, sizeof(Elf32_Shdr), 1, fp_elf);
			long offset = ftell(fp_elf);
			if (shdr.sh_type == SHT_STRTAB && i != header.e_shstrndx) { // avoid the .shstrtab
				//read the .strtab
				strtab_data = (char *)malloc(shdr.sh_size);
				fseek(fp_elf, shdr.sh_offset, SEEK_SET);
				size = fread(strtab_data, shdr.sh_size, 1, fp_elf);
			} else if (shdr.sh_type == SHT_SYMTAB) {
				//read the .symtab
				sym_table.sym_arr = (Elf32_Sym *)malloc(shdr.sh_size);	
				fseek(fp_elf, shdr.sh_offset, SEEK_SET);
				Elf32_Sym temp;
				for (int i = 0; i < shdr.sh_size / shdr.sh_entsize; ++i) {
					size = fread(&temp, shdr.sh_entsize, 1, fp_elf);
					if(ELF32_ST_TYPE(temp.st_info) == STT_FUNC) { // must use this macro unless will be wrong
						*(sym_table.sym_arr + sym_table.num++) = temp;
					}
				}
					
			}
			fseek(fp_elf, offset, SEEK_SET);
		}
		// for (int i = 0; i < sym_table.num; ++i) {
		// printf("0x%x %s %d\n",(sym_table.sym_arr + i)->st_value,(sym_table.sym_arr + i)->st_name + strtab_data, (sym_table.sym_arr + i)->st_size);
		// }
		// printf("%lu",sym_table.num);
		fclose(fp_elf);
	}
}


void ftrace(vaddr_t pc,vaddr_t func_addr ,uint8_t reg) {
	
	
	if (reg == 1) {
		// this is the return condition
		printf("0x%x: ",pc);
		for (int i = 0; i < func_call_stack.cur_ptr; ++i) putchar(' ');
		int count = 0;
		do {
		printf("return from func[%s]\t",func_call_stack.stack[func_call_stack.cur_ptr].str);
		count++;
		} while (func_call_stack.stack[func_call_stack.cur_ptr--].return_addr != func_addr);
		if (count != 1) printf("## multiple returns once in the tail recursion");
		printf("\n");
	} else {
		for (int i = 0; i < sym_table.num; ++i) {
			if ((sym_table.sym_arr + i)->st_value  == func_addr) {
				strcpy(func_call_stack.stack[++func_call_stack.cur_ptr].str, (sym_table.sym_arr + i)->st_name + strtab_data);
				func_call_stack.stack[func_call_stack.cur_ptr].addr = func_addr;
				func_call_stack.stack[func_call_stack.cur_ptr].return_addr = pc + 4;
				printf("0x%x: ",pc);
				for (int j = 0; j < func_call_stack.cur_ptr; ++j) putchar(' ');
				printf("call func[%s@0x%x]\n",func_call_stack.stack[func_call_stack.cur_ptr].str,func_addr);
				break;
			}
		}
	}
}
#endif

// int main(int argc, char **argv) {
// 	init_ftrace(argv[1]);
// 	return 0;
// }
