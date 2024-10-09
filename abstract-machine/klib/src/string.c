#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
	size_t result = 0;
	if (s == NULL) return result;
	while (*s++ != '\0') ++result;
	return result;
}

char *strcpy(char *dst, const char *src) {
  assert(dst != NULL && src != NULL);
	char *res = dst;
	while((*dst++ = *src++) != '\0');
	return res;
}

char *strncpy(char *dst, const char *src, size_t n){
  assert(dst != NULL && src != NULL);
	char *res = dst;
	while (n-- > 0) {
		*dst++ = (*src == '\0') ? '\0' : *src++;
	}
	return res;
}

char *strcat(char *dst, const char *src) {
  assert(dst != NULL && src != NULL);
	char *begin = dst + strlen(dst);
	while ((*begin++ = *src++) != '\0');
	return dst;
}

int strcmp(const char *s1, const char *s2) {
  assert(s1 != NULL && s2 != NULL);
	for (;*s1 == *s2; s1++, s2++) {
		if (*s1 == '\0') {
			return 0;
		}
	}
	return (*(const unsigned char *) s1)
		< (*(const unsigned char *) s2) ? -1 : 1;
}

int strncmp(const char *s1, const char *s2, size_t n) {
	for (;0 < n; n--, s1++, s2++) {
		if (*s1 != *s2) {
			return (*(const unsigned char *) s1) < (*(const unsigned char *) s2) ? -1 : 1;
		} else if (*s1 == '\0') return 0;
	}
	return 0;
}

void *memset(void *s, int c, size_t n) {
  assert(s != NULL);
	unsigned char *res =(unsigned char *) s;
	while (n-- > 0) {
		*res++ = (unsigned char)c;
	}
	return s;
}


void *memmove(void *dst, const void *src, size_t n) {
  assert(dst != NULL && src != NULL);
	unsigned char* dst1 = (unsigned char*) dst;
	const unsigned char* src1 = (const unsigned char*) src;
	if (dst <= src || dst >= src + n) {
		while (n-- > 0) *dst1++ = *src1++;
	} else {
		dst += n - 1;
		src += n - 1;
		while (n-- > 0) *dst1-- = *src1--;
	}
	return dst;
}
void *memcpy(void *out, const void *in, size_t n) {
	assert(out != NULL && in != NULL);
	unsigned char* out1 = (unsigned char*) out;
	const unsigned char* in1 = (const unsigned char*) in;
	while (n-- > 0) *out1++ = *in1++;
	return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  assert(s1 != NULL && s2 != NULL);
	const unsigned char* mem1 = (const unsigned char*)s1;

	const unsigned char* mem2 = (const unsigned char*)s2;

	while (n-- > 0) {
		if (*mem1 > *mem2)return 1;
		else if (*mem1 < *mem2)return -1;
		mem1++;
		mem2++;
	}
	return 0;
}
char *strchr(const char *str, int chr) {
	assert(str != NULL);
	char *ret = (char *)str;
	while (*ret != '\0') {
		if (*ret == chr) return ret;
		ret++;
	}
	return NULL;
}
#endif
