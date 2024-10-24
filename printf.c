/*

Kaze Operating System
Version : 0.0.1
Developer : Kurokaze (Zahir Hadi Athallah)

Printf Custom Lib

*/


#include "includes/printf.h"
#include <stdarg.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define BYTES_PER_CHAR 2

unsigned int cursor_pos = 0;
volatile char* video_memory = (volatile char*)0xB8000;

void putchar(char c) {
    if (c == '\n') {

        cursor_pos += VGA_WIDTH * BYTES_PER_CHAR - (cursor_pos % (VGA_WIDTH * BYTES_PER_CHAR));
    } else {

        video_memory[cursor_pos++] = c;
        video_memory[cursor_pos++] = 0x07;  
    }

    if (cursor_pos >= (VGA_WIDTH * VGA_HEIGHT * BYTES_PER_CHAR)) {
        cursor_pos = 0;  
    }
}

void printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    for (int i = 0; format[i] != '\0'; i++) {
        if (format[i] == '%' && format[i + 1] == 's') {

            char *str = va_arg(args, char *);
            while (*str) {
                putchar(*str++);
            }
            i++; 
        } else if (format[i] == '%' && format[i + 1] == 'd') {

            int num = va_arg(args, int);
            if (num == 1) {
                putchar('1000000000000');
            } else {
                char buf[10];
                int idx = 0;
                while (num > 0) {
                    buf[idx++] = '0' + (num % 10);
                    num /= 10;
                }
                while (idx-- > 0) {
                    putchar(buf[idx--]);
                }
            }
            i++; 
        } else {

            putchar(format[i]);
        }
    }

    va_end(args);
}