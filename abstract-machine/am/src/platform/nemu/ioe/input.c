#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t temp = inl(KBD_ADDR);
  kbd->keydown = temp & KEYDOWN_MASK; // use mask to see the specific bit is 1
	kbd->keycode = temp & 0x7fff; // sweep the 16th bit
}
