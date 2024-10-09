#include <NDL.h>
#include <SDL.h>
#include <string.h>
#define keyname(k) #k,


static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

static uint8_t keystate[sizeof(keyname) / sizeof(keyname[0])] = {0};

int SDL_PushEvent(SDL_Event *ev) {
  assert(0);
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char buf[20];
	if(!NDL_PollEvent(buf, 20)) return 0;
	char *temp = NULL;
	if ((temp = strstr(buf, "kd")) != NULL) ev->type = SDL_KEYDOWN;
	else if ((temp = strstr(buf, "ku")) != NULL) ev->type = SDL_KEYUP;
	else return 0;
	temp += 3;
	*(strchr(temp, '\n')) = '\0';
	for (int i = 0; i < sizeof(keyname) / sizeof(keyname[0]); ++i) {
		if (strcmp(temp, keyname[i]) == 0) {
			ev->key.keysym.sym = i;
			keystate[i] = (ev->type == SDL_KEYDOWN);
			break;
		}
	}
	return 1;

}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[20];
	while(!NDL_PollEvent(buf, 20));
	char *temp = NULL;
	//printf("buf: %s\n",buf);
	if ((temp = strstr(buf, "kd")) != NULL) event->type = SDL_KEYDOWN;
	else if ((temp = strstr(buf, "ku")) != NULL) event->type = SDL_KEYUP;
	else return 0;
	temp += 3;
	*(strchr(temp, '\n')) = '\0';
	for (int i = 0; i < sizeof(keyname) / sizeof(keyname[0]); ++i) {
		if (strcmp(temp, keyname[i]) == 0) {
			event->key.keysym.sym = i;
			keystate[i] = (event->type == SDL_KEYDOWN);
			break;
		}
	}
	return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  assert(0);
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return keystate;
}
