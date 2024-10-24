/*

Kaze Operating System
Version : 0.0.1
Developer : Kurokaze (Zahir Hadi Athallah)

The Kernel of Kaze OS

*/


#include "includes/keyboard_map.h"
#include "includes/itoa.h"
#include "includes/terminal_fg_colors.h"
#include "includes/fs.h"
#include "includes/stdint.h"
#include "includes/string.h"
#include "includes/stdint.h"

#define LINES 25
#define COLUMNS_IN_LINE 80
#define BYTES_FOR_EACH_ELEMENT 2
#define SCREENSIZE BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE * LINES

#define KEYBOARD_DATA_PORT 0x60
#define KEYBOARD_STATUS_PORT 0x64
#define IDT_SIZE 256
#define INTERRUPT_GATE 0x8e
#define KERNEL_CODE_SEGMENT_OFFSET 0x08
#define SECTOR_SIZE 512

#define ENTER_KEY_CODE 0x1C        
#define ARROW_LEFT_KEY_CODE  0x4B  
#define ARROW_RIGHT_KEY_CODE 0x4D  

char buffer[128];

extern unsigned char keyboard_map[128];
extern void keyboard_handler(void);
extern char read_port(unsigned short port);
extern void write_port(unsigned short port, unsigned char data);
extern void load_idt(unsigned long *idt_ptr);
void update_cursor(int x, int y);

unsigned int current_loc = 0;
int current_loc_y = 0;
int current_loc_x = 11;
char current_command[255] = {0};
int current_command_length = 0;

char *vidptr = (char*)0xb8000;

struct IDT_entry {
    unsigned short int offset_lowerbits;
    unsigned short int selector;
    unsigned char zero;
    unsigned char type_attr;
    unsigned short int offset_higherbits;
};

struct IDT_entry IDT[IDT_SIZE];

struct User {
    char username[20];
    int is_root;  
};

struct User users[3] = {
    {"root", 1},  
    {"user", 0}   
};

int user_count = 2;  

struct User* current_user = 0;

void idt_init(void) {
    unsigned long keyboard_address;
    unsigned long idt_address;
    unsigned long idt_ptr[2];

    keyboard_address = (unsigned long)keyboard_handler;
    IDT[0x21].offset_lowerbits = keyboard_address & 0xffff;
    IDT[0x21].selector = KERNEL_CODE_SEGMENT_OFFSET;
    IDT[0x21].zero = 0;
    IDT[0x21].type_attr = INTERRUPT_GATE;
    IDT[0x21].offset_higherbits = (keyboard_address & 0xffff0000) >> 16;

    write_port(0x20, 0x11);
    write_port(0xA0, 0x11);

    write_port(0x21, 0x20);
    write_port(0xA1, 0x28);

    write_port(0x21, 0x00);
    write_port(0xA1, 0x00);

    write_port(0x21, 0x01);
    write_port(0xA1, 0x01);

    write_port(0x21, 0xff);
    write_port(0xA1, 0xff);

    idt_address = (unsigned long)IDT;
    idt_ptr[0] = (sizeof(struct IDT_entry) * IDT_SIZE) + ((idt_address & 0xffff) << 16);
    idt_ptr[1] = idt_address >> 16;

    load_idt(idt_ptr);
}

void kb_init(void) {
    write_port(0x21, 0xFD);
}

void clear_screen(void) {
    unsigned int i = 0;
    while (i < SCREENSIZE) {
        vidptr[i++] = ' ';
        vidptr[i++] = 0x07;
    }
}

void enable_cursor(int cursor_start, int cursor_end) {
    write_port(0x3D4, 0x0A);
    write_port(0x3D5, (read_port(0x3D5) & 0xC0) | cursor_start);

    write_port(0x3D4, 0x0B);
    write_port(0x3D5, (read_port(0x3D5) & 0xE0) | cursor_end);
}

void update_cursor(int x, int y) {
    int pos = y * COLUMNS_IN_LINE + x;

    write_port(0x3D4, 0x0F);
    write_port(0x3D5, (int) (pos & 0xFF));
    write_port(0x3D4, 0x0E);
    write_port(0x3D5, (int) ((pos >> 8) & 0xFF));
}

void kprint(const char *str, int color) {
    unsigned int i = 0;
    while (str[i] != '\0') {
        vidptr[current_loc++] = str[i++];
        vidptr[current_loc++] = color;
    }

    update_cursor(current_loc_x, current_loc_y);
}

void kprint_newline(void) {
    unsigned int line_size = BYTES_FOR_EACH_ELEMENT * COLUMNS_IN_LINE;
    current_loc = current_loc + (line_size - current_loc % (line_size));
    current_loc_y++;
    current_loc_x = 11;  
    update_cursor(current_loc_x, current_loc_y);
}

void register_user(void) {
    if (user_count >= 3) {
        kprint("Max users reached. Cannot register more users.", VGA_COLOR_RED);
        kprint_newline();
        return;
    }

    char username[20] = {0}; 
    kprint("password>>>>", VGA_COLOR_GREEN);
    update_cursor(current_loc_x, current_loc_y);

    int username_length = 0;
    while (1) {
        unsigned char status = read_port(KEYBOARD_STATUS_PORT);
        if (status & 0x01) {
            char keycode = read_port(KEYBOARD_DATA_PORT);
            char key = keyboard_map[(unsigned char) keycode];

            if (keycode == ENTER_KEY_CODE) {
                username[username_length] = '\0';  
                break;
            } 

            else if (key == '\b') {
                if (username_length > 0) {
                    username_length--;
                    current_loc -= 2;
                    vidptr[current_loc++] = ' ';  
                    vidptr[current_loc++] = 0x07;
                    current_loc -= 2;
                    current_loc_x--; 
                    update_cursor(current_loc_x, current_loc_y);
                }
            }

            else if (key >= ' ' && key <= '~') { 
                if (username_length < sizeof(username) - 1) {
                    username[username_length++] = key;  
                    vidptr[current_loc++] = key;
                    vidptr[current_loc++] = 0x07;
                    current_loc_x++;
                    update_cursor(current_loc_x, current_loc_y);
                }
            }
        }
    }

    for (int i = 0; i < sizeof(username); i++) {
        users[user_count].username[i] = username[i];
    }
    users[user_count].is_root = 0;  
    user_count++;

    kprint_newline();
    kprint("User registered successfully!", VGA_COLOR_GREEN);
    kprint_newline();
}

void login(void) {
    char username[20] = {0};  
    kprint("password>>>>", VGA_COLOR_GREEN);
    update_cursor(current_loc_x, current_loc_y);

    int username_length = 0;
    while (1) {
        unsigned char status = read_port(KEYBOARD_STATUS_PORT);
        if (status & 0x01) {
            char keycode = read_port(KEYBOARD_DATA_PORT);
            char key = keyboard_map[(unsigned char)keycode];

            if (keycode == ENTER_KEY_CODE) {
                username[username_length] = '\0';  
                break;
            } 

            else if (key == '\b') {
                if (username_length > 0) {
                    username_length--;
                    current_loc -= 2;
                    vidptr[current_loc++] = ' ';  
                    vidptr[current_loc++] = 0x07;
                    current_loc -= 2;
                    current_loc_x--; 
                    update_cursor(current_loc_x, current_loc_y);
                }
            }

            else if (key >= ' ' && key <= '~') { 
                if (username_length < sizeof(username) - 1) {
                    username[username_length++] = key;  
                    vidptr[current_loc++] = key;
                    vidptr[current_loc++] = 0x07;
                    current_loc_x++;
                    update_cursor(current_loc_x, current_loc_y);
                }
            }
        }
    }

    if (my_strcmp(username, "register") == 0) {
        kprint_newline();
        register_user();
        login();  
        return;
    }

    for (int i = 0; i < user_count; i++) {
        if (my_strcmp(username, users[i].username) == 0) {
            current_user = &users[i];
            kprint_newline();
            return;
        }
    }

    kprint_newline();
    kprint("Invalid username. Try again or type 'register' to create a new user.", VGA_COLOR_RED);
    kprint_newline();
    login();
}

void logout(void) {
    current_user = 0;  
    kprint("Logout successfully", VGA_COLOR_RED);
    kprint_newline();
}

void handle_command(void) {
    char command[32];  
    char filename[32]; 
    char content[256]; 

    strcpy(command, ""); 
    strcpy(filename, "");
    strcpy(content, "");

    int i = 0;
    while (current_command[i] != '\0' && current_command[i] != ' ' && i < 31) {
        command[i] = current_command[i];
        i++;
    }
    command[i] = '\0'; 

    if (current_command[i] == ' ') {
        i++; 
        int j = 0;

        while (current_command[i] != '\0' && current_command[i] != ' ' && j < 31) {
            filename[j] = current_command[i];
            i++;
            j++;
        }
        filename[j] = '\0'; 

        if (current_command[i] == ' ') {
            i++; 
            int k = 0;
            while (current_command[i] != '\0' && k < 255) {
                content[k] = current_command[i];
                i++;
                k++;
            }
            content[k] = '\0';
        }
    }

    if (my_strcmp(command, "delete") == 0) {
        if (current_user == 0) {
            kprint("Please register & login first.", VGA_COLOR_RED);
            kprint_newline();
        } else if (filename[0] == '\0') {
            kprint("Please provide a filename to delete.", VGA_COLOR_RED);
            kprint_newline();
        } else {

            int result = fs_delete(filename);
            if (result == 0) {
                kprint("File deleted: ", VGA_COLOR_GREEN);
                kprint_newline();
                kprint(filename, VGA_COLOR_WHITE);
                kprint_newline();
            } else {
                kprint("Failed to delete file or file not found.", VGA_COLOR_RED);
                kprint_newline();
            }
        }
        return; 

    } else if (my_strcmp(command, "view") == 0) {
        if (current_user == 0) {
            kprint("Please register & login first.", VGA_COLOR_RED);
            kprint_newline();
        } else if (filename[0] == '\0') { 
            kprint("Please provide a filename.", VGA_COLOR_RED);
            kprint_newline();
        } else {
            if (current_user == 0) {
                kprint("Please register & login first.", VGA_COLOR_RED);
                kprint_newline();
            } else {

                int fd = fs_open(filename);
                if (fd < 0) {
                    kprint("File not found.", VGA_COLOR_RED);
                    kprint_newline();
                } else {

                    kprint("Your file is opened: ", VGA_COLOR_GREEN);
                    kprint(filename, VGA_COLOR_WHITE); 
                    kprint_newline();

                    char buffer[256]; 
                    int bytes_read = fs_read(fd, buffer, sizeof(buffer) - 1); 
                    buffer[bytes_read] = '\0'; 

                    kprint("File content: ", VGA_COLOR_GREEN);
                    kprint_newline();
                    kprint(buffer, VGA_COLOR_WHITE); 
                    kprint_newline();
                }
            }
        }
        return; 
    } else if (my_strcmp(command, "create") == 0) {
        if (current_user == 0) {
            kprint("Please register & login first.", VGA_COLOR_RED);
            kprint_newline();
        } else if (filename[0] == '\0') {
            kprint("Please provide a filename.", VGA_COLOR_RED);
            kprint_newline();
        } else if (content[0] == '\0') {
            kprint("Please provide content to write to the file.", VGA_COLOR_RED);
            kprint_newline();
        } else {

            int fd = fs_create(filename);
            if (fd > 0) {
                fs_write(fd, content, strlen(content)); 
                kprint("File created and content written: ", VGA_COLOR_GREEN);
                kprint(filename, VGA_COLOR_WHITE);
                kprint_newline();
            } else {
                kprint("Failed to create file.", VGA_COLOR_RED);
                kprint_newline();
            }
        }
        return; 

    } else if (my_strcmp(current_command, "clear") == 0) {
        clear_screen();
        current_loc_y = 0;
        current_loc_x = 11;
        current_loc = 0;  
        update_cursor(current_loc_x, current_loc_y);
    } else if (my_strcmp(current_command, "fetch") == 0) {
        kprint(" ____  __.  _____  _____________________         ________    _________", VGA_COLOR_CYAN);
        kprint_newline();  
        kprint("|    |/ _| /  _  \\ \\____    /\\_   _____/         \\_____  \\  /   _____/", VGA_COLOR_CYAN);
        kprint_newline();  
        kprint("|      <  /  /_\\  \\  /     /  |    __)_   ______  /   |   \\ \\_____  \\ ", VGA_COLOR_CYAN);
        kprint_newline();  
        kprint("|    |  \\/    |    \\/     /_  |        \\ /_____/ /    |    \\/        \\", VGA_COLOR_CYAN);
        kprint_newline();  
        kprint("|____|__ \\____|__  /_______ \\/_______  /         \\_______  /_______  /", VGA_COLOR_CYAN);
        kprint_newline();  
        kprint("        \\/       \\/        \\/        \\/                  \\/        \\/ ", VGA_COLOR_CYAN);

        kprint_newline();
        kprint_newline();
        kprint("Version: 0.0.1", VGA_COLOR_GREEN);
        kprint_newline();
        kprint("Developer: Kurokaze", VGA_COLOR_GREEN);
        kprint_newline();
    } else if (my_strcmp(current_command, "shutdown") == 0) {
    if (current_user == 0) {
        kprint("Please register & login first.", VGA_COLOR_RED);
        kprint_newline();
    } else if (current_user->is_root) {
        kprint("Shutting down system...", VGA_COLOR_RED);
        kprint_newline();
        asm volatile("hlt");
    } else {
        kprint("Permission denied: Only root can execute this command.", VGA_COLOR_RED);
        kprint_newline();
    }
} else if (my_strcmp(current_command, "help") == 0) {
    kprint("Available Commands:", VGA_COLOR_CYAN);
        kprint_newline();
kprint("- clear: Clears the screen", VGA_COLOR_WHITE);
kprint_newline();
kprint("- fetch: Displays system information", VGA_COLOR_WHITE);
kprint_newline();
kprint("- register: Registers a user in the system", VGA_COLOR_WHITE);
kprint_newline();
kprint("- login: Logs into the user system", VGA_COLOR_WHITE);
kprint_newline();
kprint("- logout: Logs out from the user system", VGA_COLOR_WHITE);
kprint_newline();
kprint("- switch: Switches to the root user", VGA_COLOR_WHITE);
kprint_newline();
kprint("- unswitch: Switches back to a regular user", VGA_COLOR_WHITE);
kprint_newline();
kprint("- shutdown: Shuts down the system", VGA_COLOR_WHITE);
kprint_newline();
kprint("- ls: Lists all files in the current directory", VGA_COLOR_WHITE);
kprint_newline();
kprint("- view <filename>: Displays the content of a specified file", VGA_COLOR_WHITE);
kprint_newline();
kprint("- create <filename> <content>: Creates a file and writes the specified content", VGA_COLOR_WHITE);
kprint_newline();
kprint("- delete <filename>: Deletes the specified file", VGA_COLOR_WHITE);
kprint_newline();

    } else if (my_strcmp(current_command, "login") == 0) {
        login();
    } else if (my_strcmp(current_command, "register") == 0) {
        register_user();
    } else if (my_strcmp(current_command, "switch") == 0) {
        if (current_user == 0) {
            kprint("Please register & login first.", VGA_COLOR_RED);
            kprint_newline();
        } else {
            current_user = &users[0];  
            kprint_newline();
            kprint("Switched to root user.", VGA_COLOR_GREEN);
            kprint_newline();
        }
    } else if (my_strcmp(current_command, "unswitch") == 0) {
        if (current_user == 0) {
            kprint("Please register & login first.", VGA_COLOR_RED);
            kprint_newline();
        } else {
            current_user = &users[1];  
            kprint_newline();
            kprint("Switched to user.", VGA_COLOR_GREEN);
            kprint_newline();
        }
    } else if (my_strcmp(current_command, "logout") == 0) {
        if (current_user == 0) {
            kprint("Please register & login first.", VGA_COLOR_RED);
            kprint_newline();
        } else {
            logout();
        }
    } else if (my_strcmp(current_command, "ls") == 0) {
        if (current_user == 0) {
            kprint("Please register & login first.", VGA_COLOR_RED);
            kprint_newline();
        } else {
            fs_ls();
        }
    } else {
        kprint("Invalid Command", VGA_COLOR_RED);
        kprint_newline();
    }
}

void keyboard_handler_main(void) {
    unsigned char status;
    char keycode;

    write_port(0x20, 0x20); 

    status = read_port(KEYBOARD_STATUS_PORT);
    if (status & 0x01) { 
        keycode = read_port(KEYBOARD_DATA_PORT);
        if (keycode < 0) return;

        if (keycode == ENTER_KEY_CODE) {
            current_command[current_command_length] = '\0';
            kprint_newline();
            handle_command();
            current_command_length = 0;
            if (current_user == 0) {
    kprint("null@kaze>> ", VGA_COLOR_GREEN);  
} else {
    kprint(current_user->is_root ? "root@kaze>> " : "user@kaze>>", VGA_COLOR_GREEN);
}
            return;
        }

        if (keyboard_map[(unsigned char)keycode] == '\b') {
            if (current_command_length > 0) {
                current_loc -= 2;
                vidptr[current_loc++] = ' ';
                vidptr[current_loc++] = 0x07;
                current_loc -= 2; 
                current_command[--current_command_length] = '\0';
                current_loc_x--; 
                update_cursor(current_loc_x, current_loc_y);
            }
            return;
        }

        if (keycode == ARROW_LEFT_KEY_CODE) { 
            if (current_loc_x > 0) {
                current_loc_x--; 
                update_cursor(current_loc_x, current_loc_y);
            }
            return;
        }

        if (keycode == ARROW_RIGHT_KEY_CODE) { 
            if (current_loc_x < current_command_length) {
                current_loc_x++; 
                update_cursor(current_loc_x, current_loc_y);
            }
            return;
        }

        vidptr[current_loc++] = keyboard_map[(unsigned char)keycode];
        vidptr[current_loc++] = 0x07;
        current_command[current_command_length++] = keyboard_map[(unsigned char)keycode];
        current_loc_x++; 
        update_cursor(current_loc_x, current_loc_y);
    }
}

void kmain(void) {
    clear_screen();

    fs_format();  
    fs_init();    

    kprint(" ____  __.  _____  _____________________         ________    _________", VGA_COLOR_CYAN);
    kprint_newline();  
    kprint("|    |/ _| /  _  \\ \\____    /\\_   _____/         \\_____  \\  /   _____/", VGA_COLOR_CYAN);
    kprint_newline();  
    kprint("|      <  /  /_\\  \\  /     /  |    __)_   ______  /   |   \\ \\_____  \\ ", VGA_COLOR_CYAN);
    kprint_newline();  
    kprint("|    |  \\/    |    \\/     /_  |        \\ /_____/ /    |    \\/        \\", VGA_COLOR_CYAN);
    kprint_newline();  
    kprint("|____|__ \\____|__  /_______ \\/_______  /         \\_______  /_______  /", VGA_COLOR_CYAN);
    kprint_newline();  
    kprint("        \\/       \\/        \\/        \\/                  \\/        \\/ ", VGA_COLOR_CYAN);

    kprint_newline();
    kprint_newline();

    idt_init();
    kprint("[0] Developed by Kurokaze", VGA_COLOR_CYAN);
    kprint_newline();
    kprint_newline();

    kb_init();
    enable_cursor(0, 15);
    if (current_user == 0) {
    kprint("null@kaze>> ", VGA_COLOR_GREEN);  
} else {
    kprint(current_user->is_root ? "root@kaze>> " : "user@kaze>>", VGA_COLOR_GREEN);
}
    while (1);
}