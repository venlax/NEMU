#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

int screen_width = 0, screen_height = 0;
size_t serial_write(const void *buf, size_t offset, size_t len) {
  for (int i = 0; i < len; ++i) putch(*((const char *)buf + i));
	return len;
}

size_t events_read(void *buf, size_t offset, size_t len) {
	AM_INPUT_KEYBRD_T kb = io_read(AM_INPUT_KEYBRD);
	if (kb.keycode == AM_KEY_NONE) return 0;
	int ret = 0;
	strcpy((char *)buf, kb.keydown ? "kd " : "ku ");
	ret += 3;
	strcat((char *)buf, keyname[kb.keycode]);
	ret += strlen(keyname[kb.keycode]) + 1;
	strcat((char*)buf, "\n");
	return ret;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
 	//printf("xxw:%d h:%d\n",w,h);
	size_t count = sprintf((char *)buf,"WIDTH: %d\nHEIGHT: %d\n",screen_width,screen_height);
	//memcpy(buf,&w,4);
	//memcpy(buf + 4, &h, 4);	
	return count;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
 	//io_write(AM_GPU_MEstMCPY, offset, (void *)buf, len);
	AM_GPU_FBDRAW_T info;
	info.pixels = (void *)buf;
	info.sync = true;
	info.w = (offset + len > screen_width * screen_height * 4 ?  screen_width* screen_height * 4 - offset : len) / 4;
	info.h = 1;
	info.x = (offset / 4) % screen_width ;
 	info.y = (offset / 4 ) / screen_width;	
	//printf("\ndraw x:%d y:%d w:%d h:%d\n",info.x,info.y,info.w,info.h);
	ioe_write(AM_GPU_FBDRAW, &info);
 	return 0;
}
#ifdef ok
size_t sb_write(const void *buf, size_t offset, size_t len) {
	int size = io_read(AM_AUDIO_CONFIG).bufsize;
	Area config;
	config.start = (void *)buf;
	int count = 0;
	while (count < len) {
		int real_len = len - count > size ? size : len - count;
		config.end = (config.start + real_len);
		io_write(AM_AUDIO_PLAY,config);
		config.start += real_len;
		count += real_len;
	}
	return count;
}

size_t sbctl_read(void *buf, size_t offset, size_t len) {
	((int *)buf)[0] = io_read(AM_AUDIO_STATUS).count;
	return 1;
}

size_t sbctl_write(const void *buf, size_t offset, size_t len) {
	io_write(AM_AUDIO_CTRL, ((int *)buf)[1],((int *)buf)[2],((int *)buf)[3]);
	return 3;
}

#endif
void init_device() {
  Log("Initializing devices...");
  ioe_init();
	
  screen_width = io_read(AM_GPU_CONFIG).width, screen_height = io_read(AM_GPU_CONFIG).height;
	
}
