#include <proc.h>
#include <elf.h>
#include <string.h>
#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

#if defined(__ISA_AM_NATIVE__)
#define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_X86__)
#define EXPECT_TYPE EM_X86_64
#elif defined(__ISA_MIPS32__)
#define EXPECT_TYPE EM_MIPS
#elif defined(__riscv)
#define EXPECT_TYPE EM_RISCV
#elif
#error unsupported ISA __ISA__
#endif
size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t fs_read(int fd, void *buf, size_t len);

extern int fs_open(const char *pathname, int flags, int mode);

extern size_t fs_write(int fd, const void *buf, size_t len);

extern size_t fs_lseek(int fd, size_t offset, int whence);

extern int fs_close(int fd);
static uintptr_t loader(PCB *pcb, const char *filename) {
  //TODO();
	if (strchr(filename,'\n') != NULL) *strchr(filename, '\n') = '\0';
	int fd = fs_open(filename, 0, 0);
	Elf_Ehdr elf_header;
  fs_lseek(fd, 0, 0);
	fs_read(fd, &elf_header, sizeof(Elf_Ehdr));
	printf("%s %d",filename,strlen(filename));
	assert(*(uint32_t *)elf_header.e_ident == 0x464c457f);
  assert(elf_header.e_machine == EXPECT_TYPE);
  Elf_Phdr program_header;
  for (int i = 0; i < elf_header.e_phnum; ++i) {
     // ramdisk_read(&program_header, elf_header.e_phoff + i * sizeof(Elf_Phdr), sizeof(Elf_Phdr));
			
  fs_lseek(fd, elf_header.e_phoff + i * sizeof(Elf_Phdr), 0);
	fs_read(fd, &program_header, sizeof(Elf_Phdr));
      if (program_header.p_type == PT_LOAD) {
				//ramdisk_read((void *)program_header.p_vaddr, program_header.p_offset, program_header.p_memsz);

  			fs_lseek(fd, program_header.p_offset, 0);
				fs_read(fd, (void *)program_header.p_vaddr, program_header.p_memsz);
        memset((void *)program_header.p_vaddr + program_header.p_filesz, 0, program_header.p_memsz - program_header.p_filesz);
      }
  }
  return elf_header.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
	Log("Jump to entry = %p", entry);
	((void(*)())entry) ();
}

