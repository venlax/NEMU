#include <common.h>
#include "syscall.h"
#include <sys/time.h>
#include <proc.h>
void naive_uload(PCB *pcb, const char *filename);
#ifdef CONFIG_STRACE
void strace(char *name, uintptr_t NO, uintptr_t return_num) {
	printf("System call No %d : %s, return %d\n",NO,name, return_num);
}

void sfs_strace (char *name, uintptr_t NO, char *filename,uintptr_t return_num) {
	printf("System call No %d : %s, file: %s , return %d\n",NO,name,filename,return_num);
}
#endif

#ifdef CONFIG_STRACE
#define strace_(name) strace(name, a[0], c->GPRx); 
char* get_filename(int fd);

#define sfs_strace_(name, filename) sfs_strace(name, a[0], filename,c->GPRx);
#else
#define strace_(name) 
#define sfs_strace_(name,filename)
#endif
extern size_t fs_read(int fd, void *buf, size_t len);

extern int fs_open(const char *pathname, int flags, int mode);

extern size_t fs_write(int fd, const void *buf, size_t len);

extern size_t fs_lseek(int fd, size_t offset, int whence);

extern int fs_close(int fd);
void exit(uintptr_t exit_no) {
	naive_uload(NULL, "/bin/nterm");
	//halt(0);
}

int brk() {
	return 0;
}

size_t read(int fd, void *buf, size_t len) {
	return fs_read(fd, buf, len);
	
}

size_t write(int fd, const void *buf, size_t len) {
  return fs_write(fd ,buf, len);
}

int open(char *filename, int flags, int mode) {
	return fs_open(filename, flags, mode);
}

size_t lseek(int fd, size_t offset ,int whence) {
	return fs_lseek(fd, offset, whence);
}

int close(int fd) {
	return fs_close(fd);
}

int _gettimeofday(struct timeval *tv, struct timezone *tz) {
	uint64_t sec = io_read(AM_TIMER_UPTIME).us;
	tv->tv_sec = sec / 1000000;
	tv->tv_usec = sec % 1000000;
	return 0;
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
	//printf("a[3]: %d\n",a[3])
  switch (a[0]) {
		case SYS_exit:strace_("exit") exit(c->GPRx);break;
		case SYS_yield: yield();c->GPRx = 0;strace_("yield") break;
		case SYS_read: c->GPRx = read(a[1], (void *)a[2], a[3]); sfs_strace_("read", get_filename(a[1])) break;
		case SYS_write: c->GPRx = write(a[1], (const void *)a[2], a[3]); sfs_strace_("write", get_filename(a[1])) break;
		case SYS_open: c->GPRx = open((char *)a[1], a[2], a[3]);sfs_strace_("open",(char *)a[1]) break;
		case SYS_lseek: c->GPRx = lseek(a[1], a[2], a[3]); sfs_strace_("lseek",get_filename(a[1])) break;
		case SYS_brk: c->GPRx = brk();strace_("brk") break; 
		case SYS_close: c->GPRx = close(a[1]);sfs_strace_("close",get_filename(a[1])) break;
		case SYS_gettimeofday: c->GPRx = _gettimeofday((struct timeval *)a[1], (struct timezone *)a[2]); strace_("gettimeofday") break;
		case SYS_execve:c->GPRx = 0;naive_uload(NULL, (char *)a[1]); strace_(execve) break;
		default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
