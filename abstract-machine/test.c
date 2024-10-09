#include <stdarg.h>
#include <stdio.h>

int _sprintf(char *out, const char *fmt, ...) {
	
	rtc_io_handler();
	va_list args;
	int count = 0;
	va_start(args , fmt);	
	while (*fmt != '\0') {
		if (*fmt == '%') {
			fmt++;
			switch (*fmt) {
				case 's':
					char *str = va_arg(args, char*);
					while ( *str != '\0') {
						*out++ = *str++;
						count++;
					}
					break;
				case 'd':
					int num = va_arg(args, int);
					if (num < 0) {
						*out++ = '-';
						count++;
						num = -num;
					}
					char arr[10] = {'0'};
					int len = 0;
					
					do {
						arr[len++] = num % 10 + '0';
						num /= 10;
					}while (num != 0);
					
					for (int i = len - 1; i >= 0 ; --i) {
						*out++ = arr[i];
						count++;
					}
					break;
				default :
					//panic("wrong format string");	
			}
		} else {
			*out++ = *fmt;
			count++;
		}
		fmt++;
	}
	*out = '\0';//convert to C string
	va_end(args);
	return count;
}


int main() {
	char str[50];
	_sprintf(str, "%s %d %s","xixi", -5, "xxx");
	printf("%s\n", str);
	return 0;
}

