#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;

uint32_t NDL_GetTicks() {
  struct timeval temp;
	gettimeofday(&temp,NULL);
	
	return temp.tv_sec * 1000 + temp.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) {
	int fd = open("/dev/events",0,0);
	//printf("\n%d\n",fd);
	int ret = read(fd, buf, len);
	close(fd);
	//if (strstr(buf, "kb:") == NULL && strstr(buf,"ku:") == NULL) return 0;
	return !(ret == 0);
}

void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
	//printf("\nok\n");
	int fd = open("/proc/dispinfo",0,0);
	char buf2[32];
	read(fd, buf2, 32);
	//memcpy(&screen_w, buf2, 4);
	//memcpy(&screen_h, buf2 + 4, 4);
	sscanf(buf2,"WIDTH: %d\nHEIGHT: %d\n",&screen_w,&screen_h);
	if (*w == 0 && *h == 0) {
		*w = screen_w;
		*h = screen_h;
	}
	//printf("screen_w:%dscreen_h%d\n",screen_w,screen_h);
	//for (int i = 0; i < 30; ++i) putchar(buf2[i]);
	//printf("buf2:%s w:%d h:%d\n",buf2,*w,*h);
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
	if (fbdev == -1) fbdev = open("/dev/fb",0,0);
	//printf("w: %d h: %d fbdev: %d\n",w,h,lseek(fbdev,0,SEEK_END));
	if (w == 0 && h == 0) {
	 w = screen_w;
	 h = screen_h;
	}
	for (int r = 0; r < h; ++r) {
		lseek(fbdev, (  x + (y + r ) * screen_w ) * 4, SEEK_SET);
		write(fbdev, pixels + r * w , w * 4  );	
	}
}

void NDL_OpenAudio(int freq, int channels, int samples) {
#ifdef ok
	int fd = open("dev/sbctl",0,0);
	int buf[4] = {0, freq, channels, samples};
	write(fd, (void *)buf, 4);
#endif
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
#ifdef ok
	int fd = open("/dev/sb",0,0);
	return write(fd,buf,len);
#endif
	return 0;
}

int NDL_QueryAudio() {
#ifdef ok
	int ret = 0;
		int fd = open("/dev/sbctl",0,0);
		read(fd,&ret,1);
		return ret;
#endif
		return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit() {
}

