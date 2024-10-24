/*

Kaze Operating System
Version : 0.0.1
Developer : Kurokaze (Zahir Hadi Athallah)

Kprint Lib

*/

#include "includes/kprint.h"
#include "includes/stdint.h" 

char *vidptr = (char*)0xb8000; 

unsigned int current_loc = 0;  
int current_loc_x = 0;         
int current_loc_y = 0;         

void kprint(const char *str, int color) {
    unsigned int i = 0;
    while (str[i] != '\0') {
        vidptr[current_loc++] = str[i++];
        vidptr[current_loc++] = color;
    }
    update_cursor(current_loc_x, current_loc_y); 
}

void kprint_newline(const char *str, int color) {
    kprint(str, color);          
    kprint("\n", color);        
}