#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);
#ifdef ok
size_t sb_write(const void *buf, size_t offset, size_t len);
size_t sbctl_write(const void *buf, size_t offset, size_t len);
size_t sbctl_read(void *buf, size_t offset, size_t len);
#endif
#define NumFile sizeof(file_table) / sizeof(Finfo)

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
	size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write, 0},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write, 0},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write, 0},
	[FD_FB] = {"/dev/fb",0,0,invalid_read,fb_write},
	{"/dev/events", 0, 0, events_read, invalid_write},
	{"/proc/dispinfo",0,0,dispinfo_read,invalid_write},
#ifdef ok
	{"/dev/sb",0,0,invalid_read,sb_write},
	{"/dev/sbctl",0,0,sbctl_read,sbctl_write},
#endif
#include "files.h"
};
#ifdef CONFIG_STRACE
char* get_filename(int fd) {
		return (file_table[fd].name);
}
#endif


void init_fs() {
  // TODO: initialize the size of /dev/fb
	AM_GPU_CONFIG_T wh = io_read(AM_GPU_CONFIG);
	//printf("%d %d",wh.width,wh.height);
	file_table[FD_FB].size = wh.width * wh.height * 4;
}


int fs_open(const char *pathname, int flags, int mode) {
	for (int i = 0; i < NumFile; ++i) {
		if (strcmp(file_table[i].name, pathname) == 0) {
			file_table[i].open_offset = 0;
			//printf("%d\n",i);
			return i;
		}
	}
	return -1;
}

size_t fs_read(int fd, void *buf, size_t len) {
	Finfo *file_handler = &file_table[fd]; // file handler
	
	//printf("%d\n",fd);
	if (file_handler == NULL) return -1;
	//if (file_handler->read == events_read)printf("ok");	
 	//if (strcmp(file_handler->name, "/dev/events") == 0) printf("ok");	
	
	
	size_t count = (file_handler->open_offset + len ) <= file_handler->size ? len : file_handler->size - file_handler->open_offset;
	if (file_handler->read == NULL) file_handler->read = ramdisk_read;
	if (file_handler->read == events_read || file_handler->read == dispinfo_read) count = len;
	count = file_handler->read(buf,file_handler->disk_offset + file_handler->open_offset, count);	
	
	file_handler->open_offset += count;
		
	return count;
}

size_t fs_write(int fd,const void *buf, size_t len) {
	//printf("%d\n",fd);

	size_t count = 0;
	Finfo* file_handler = &file_table[fd];
	if (file_handler == NULL) return -1; 
	if (file_handler->write == NULL) file_handler->write = ramdisk_write;
	count = file_handler->write(buf, file_handler->disk_offset + file_handler->open_offset, len);
	file_handler->open_offset += count;
	
	return count;
}


size_t fs_lseek(int fd, size_t offset, int whence) {
	Finfo *file_handler = &file_table[fd];
	//printf("%d\n",fd);
	if (file_handler == NULL) return -1; 
	switch(whence) {
		case SEEK_CUR:
			assert(file_handler->open_offset + offset <= file_handler->size);
		 	file_handler->open_offset += offset;
			return file_handler->open_offset;
		case SEEK_SET:
			//printf("%d %s\n",fd,file_handler->name);
			//assert(offset <= file_handler->size);
			file_handler->open_offset = offset;
			//printf("%d %d\n",offset, file_handler->size);
			return file_handler->open_offset;
		case SEEK_END:
			assert(offset <= file_handler->size);
			file_handler->open_offset = file_handler->size - offset;	
			return file_handler->open_offset;
		default :
			panic("unknown whence");
	}
	return 0;	
}

int fs_close(int fd) {
	//printf("%d\n",fd);
	return 0;
}

