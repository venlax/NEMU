#include <NDL.h>
#include <SDL.h>
int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
  //assert(0);
#ifdef ok
	NDL_OpenAudio(desired->freq, desired->channels, desired->samples);
	if (obtained != NULL) {
		memcpy((void *)obtained, (const void *)desired, sizeof(SDL_AudioSpec));
		obtained->size = 0x10000;
		obtained->callback = NULL;
		obtained->userdata = NULL;
	}
#endif	
	return 0;
}

void SDL_CloseAudio() {
  //assert(0);
#ifdef ok
	NDL_CloseAudio();
#endif
}

void SDL_PauseAudio(int pause_on) {
  //assert(0);
#ifdef ok
	int fd = open("/dev/sbctl",0,0);
	int buf[4] = {pause_on,0,0,0};
	write(fd,(void *)buf,4);
#endif
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
  //assert(0);
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
  //assert(0);
  return NULL;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
  //assert(0);
}

void SDL_LockAudio() {
  //assert(0);
}

void SDL_UnlockAudio() {
  //assert(0);
}
