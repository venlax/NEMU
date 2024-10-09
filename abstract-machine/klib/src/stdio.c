#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

void format(const char *fmt,char *out,  va_list args, int* count, int flag, size_t size) {
	if (flag && (size == 0)) return;
	char char_;
	char *str;
	short snum;
	int num;
	unsigned int unum;
	char *aux_str;
	int aux_int;
	int width = 0;
	char aux_arr[10];
	char pad_char = ' ';
	while (*fmt != '\0') {
		
		if (*fmt == '%') {
			fmt++;

			//init
			width = 0;
			pad_char = ' ';
			aux_int = 0;
			//

			if (my_isnum(*fmt)) {
				char temp[12];
				int count = 0;
				if (*fmt == '0') pad_char = '0';
				else {
					temp[count++] = *fmt;
				}
				fmt++;
				while (my_isnum(*fmt)) {
					temp[count++] = *fmt++;
				}
				temp[count++] = '\0';
				width = atoi(temp);
			}		
			switch (*fmt) {
				case 'c':
						char_ = va_arg(args, int);
						*out++ = char_;
						(*count)++;
						break;
				case 's':
					str = va_arg(args, char*);
					while ( *str != '\0') {
						*out++ = *str++;
						(*count)++;
					}					
					break;
				case 'h':
					fmt++;
					switch (*fmt) {
						case 'd' :
							snum = va_arg(args,int);
							if (snum < 0) {
								*out++ = '-';
								(*count)++;
								if (snum == 0x8000) {
									aux_str = "32768";
									for (aux_int = 0; aux_int < 5; ++aux_int) {
										*out++ = aux_str[aux_int];
										(*count)++;
									}
									break;
								}else {
									snum = -snum;
								}
							}
							aux_int= 0;

							do {
								aux_arr[aux_int++] = snum % 10 + '0';
								snum /= 10;
							}while (snum != 0);
					
							for (aux_int -= 1; aux_int >= 0 ; --aux_int) {
								*out++ = aux_arr[aux_int];
								(*count)++;
							}
							break;
					
					}
				break;
				case 'd':
					num = va_arg(args, int);
					if (num < 0) {
						*out++ = '-';
						(*count)++;
						width -= 1; // cause -
						if (num == 0x80000000) {
							aux_str = "2147483648";
							if (strlen(aux_str) < width) {
								for (aux_int = width - strlen(aux_str);aux_int > 0;aux_int--){
								*out++ = pad_char;
								(*count)++;
								}
							}
							for (aux_int = 0; aux_int < 10; ++aux_int){
								*out++ = aux_str[aux_int];
								(*count)++;
							}
							break;
						}else {
							num = -num;
						}
					}
					aux_int= 0;

					do {
						aux_arr[aux_int++] = num % 10 + '0';
						num /= 10;
					}while (num != 0);
					while (aux_int < width) {
						*out++ = aux_arr[aux_int];
						(*count)++;
						width--;
					}
					for (aux_int -= 1; aux_int >= 0 ; --aux_int) {
						*out++ = aux_arr[aux_int];
						(*count)++;
					}
					break;
				case 'u':
					unum = va_arg(args, unsigned int);
					aux_int= 0;

					do {
						aux_arr[aux_int++] = unum % 10 + '0';
						unum /= 10;
					}while (unum != 0);
					
					for (aux_int -= 1; aux_int >= 0 ; --aux_int) {
						*out++ = aux_arr[aux_int];
						(*count)++;
					}
					break;
	
				case 'f':
					//TODO
					break;
				case 'o':
					//TODO

					break;	
				case 'x':
				case 'p':
					//TODO
					break;	
				default :
					panic("wrong format string");	
			}
		
		} else {
			*out++ = *fmt;
			(*count)++;
		}
		fmt++;
	if (flag && *count == size - 1) break;
	}
	*out = '\0';
	while (flag && *count > size - 1) {
		*(--out) = '\0';
		(*count)--;
	}
	va_end(args);
}

	int printf(const char *fmt, ...) {
		va_list args;
		int count = 0;
		char out[10000]; // buffer
		char *res = out;
		va_start(args, fmt);
		format(fmt, out, args, &count, 0, 0);
		while (*res != '\0') putch(*res++);
		return count;	
	}

	int vsprintf(char *out, const char *fmt, va_list ap) {
		int count = 0;
		//va_start(ap,fmt);
		format(fmt, out, ap, &count, 0, 0);
		return count;
	}

int sprintf(char *out, const char *fmt, ...) {
	va_list args;
	int count = 0;
	va_start(args , fmt);	
	format(fmt, out, args, &count,0 , 0);
	return count;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list args;
	int count = 0;
	va_start(args, fmt);
	format(fmt, out, args, &count,1, n);
	return count;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  int count = 0;
	//va_start(ap, fmt);
	format(fmt, out, ap, &count,1, n);
	return count;

}

#endif
