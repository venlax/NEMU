#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
static unsigned long int next = 1;
static uint32_t addr_offset = 0;
int rand(void) {
  // RAND_MAX assumed to be 32767
  next = next * 1103515245 + 12345;
  return (unsigned int)(next/65536) % 32768;
}

void srand(unsigned int seed) {
  next = seed;
}

int abs(int x) {
  return (x < 0 ? -x : x);
}

int atoi(const char* nptr) {
  int x = 0;
  while (*nptr == ' ') { nptr ++; }
  while (*nptr >= '0' && *nptr <= '9') {
    x = x * 10 + *nptr - '0';
    nptr ++;
  }
  return x;
}

int my_isalpha(const char char_) {
	return (char_ >= 'a' && char_ <= 'z') || (char_ >= 'A' && 
			char_ <= 'Z');
}

int my_isnum(const char char_) {
	return char_ >= '0' && char_ <= '9';
}


void *malloc(size_t size) {
  // On native, malloc() will be called during initializaion of C runtime.
  // Therefore do not call panic() here, else it will yield a dead recursion:
  //   panic() -> putchar() -> (glibc) -> malloc() -> panic()
#if !(defined(__ISA_NATIVE__) && defined(__NATIVE_USE_KLIB__))
  //panic("Not implemented");
#endif
	while ((addr_offset + (uint32_t)heap.start) % 4 != 0) addr_offset++;
	void *ret = (void *)(addr_offset + (uint32_t)heap.start);
	addr_offset += size;
	return ret;
}

void free(void *ptr) {
}

#endif
