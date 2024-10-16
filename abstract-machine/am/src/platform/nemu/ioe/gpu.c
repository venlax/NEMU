#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)
void __am_gpu_init() {
//	int i;
//	int w = io_read(AM_GPU_CONFIG).width;
//	int h = io_read(AM_GPU_CONFIG).height;
//	uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
//	for (i = 0; i < w * h; i++) fb[i] = i;
//	outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = 0, .height = 0,
    .vmemsz = 0
  };
	// get the height and the width
	cfg->width = inw(VGACTL_ADDR + 2);
	cfg->height = inw(VGACTL_ADDR);
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  uint32_t *pixels = ctl->pixels;
	int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
	uint32_t screen_width = inw(VGACTL_ADDR + 2);
	// refer to display test
	// fb means the true pos
	uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
	for (int i = y;i <y + h; i++) {
		for (int j = x;j < x + w;j++) {
			fb[i * screen_width + j] = pixels[(i - y) * w + (j - x)];
		}
	}
	
	if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
