#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Global state variables */
char debug_mode = 0;               
char file_name[128] = "";          
int unit_size = 1;                 
unsigned char mem_buf[10000];      
size_t mem_count = 0;              
int display_mode = 0; /* 0 = hexadecimal, 1 = decimal */

struct fun_desc {
    char *name;
    char index;
    void (*fun)();
};

/* ========================================= */
/* FORMATTING LOGIC FROM units.c             */
/* ========================================= */

char* unit_to_format() {
    // These format arrays match the assignment requirements exactly
    static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
    static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};
    
    if (display_mode == 1) {
        return dec_formats[unit_size - 1];
    } else {
        return hex_formats[unit_size - 1];
    }
}

/* ========================================= */
/* IMPLEMENTED MENU FUNCTIONS                */
/* ========================================= */

void toggle_debug_mode() {
    debug_mode = !debug_mode;
    if (debug_mode) {
        fprintf(stderr, "Debug flag now on\n");                 
    } else {
        fprintf(stderr, "Debug flag now off\n");                
    }
}

void set_file_name() {
    printf("Enter file name: ");
    char input[256];                                            
    if (fgets(input, sizeof(input), stdin) != NULL) {
        input[strcspn(input, "\n")] = 0; 
        strncpy(file_name, input, 128);  
        if (debug_mode) {
            fprintf(stderr, "Debug: file name set to '%s'\n", file_name); 
        }
    }
}

void set_unit_size() {
    printf("Enter unit size (1, 2, or 4): ");                   
    char input[256];
    if (fgets(input, sizeof(input), stdin) != NULL) {
        int new_size;
        if (sscanf(input, "%d", &new_size) == 1) {
            if (new_size == 1 || new_size == 2 || new_size == 4) {  
                unit_size = new_size;
                if (debug_mode) {
                    fprintf(stderr, "Debug: set size to %d\n", unit_size); 
                }
            } else {
                printf("Error: invalid unit size. Size remains %d.\n", unit_size); 
            }
        }
    }
}

void load_into_memory() {
    if (strcmp(file_name, "") == 0) {
        printf("Error: File name is empty.\n");
        return;
    }

    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        printf("Error: Could not open file '%s'.\n", file_name);
        return;
    }

    printf("Please enter <location> <length>\n> ");
    char input[256];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        fclose(file);
        return; 
    }

    unsigned int location;
    int length;
    if (sscanf(input, "%x %d", &location, &length) != 2) {
        printf("Error: Invalid input format.\n");
        fclose(file);
        return;
    }

    if (debug_mode) {
        fprintf(stderr, "Debug: file_name=%s, location=%#x, length=%d\n", file_name, location, length);
    }

    fseek(file, location, SEEK_SET);

    /* Streamlined read using units.c logic: fread reads (length) elements of (unit_size) bytes */
    mem_count = fread(mem_buf, unit_size, length, file); 

    fclose(file);
    printf("Loaded %d units into memory\n", length);
}

void toggle_display_mode() {
    display_mode = !display_mode;
    if (display_mode) {
        printf("Decimal display flag now on, decimal representation\n");
    } else {
        printf("Decimal display flag now off, hexadecimal representation\n");
    }
}

void memory_display() {
    printf("Enter address and length\n> ");
    char input[256];
    if (fgets(input, sizeof(input), stdin) == NULL) return;

    unsigned int addr;
    int length;
    if (sscanf(input, "%x %d", &addr, &length) != 2) {
        printf("Error: Invalid input format.\n");
        return;
    }

    if (debug_mode) {
        fprintf(stderr, "Debug: addr=%#x, length=%d\n", addr, length);
    }

    unsigned char *buffer = (addr == 0) ? mem_buf : (unsigned char *)addr;
    
    /* Pointer limits setup just like units.c */
    unsigned char *end = buffer + (unit_size * length); 

    if (display_mode == 1) {
        printf("Decimal\n=======\n");
    } else {
        printf("Hexadecimal\n===========\n");
    }

    /* Loop mechanism completely simplified via units.c */
    while (buffer < end) {
        // Read 4 bytes natively, then let printf truncate it according to unit_to_format()
        int var = *((int*)(buffer));
        printf(unit_to_format(), var);
        buffer += unit_size;
    }
}

void save_into_file() {
    if (strcmp(file_name, "") == 0) {
        printf("Error: File name is empty.\n");
        return;
    }

    FILE *file = fopen(file_name, "r+");
    if (file == NULL) {
        printf("Error: Could not open file.\n");
        return;
    }

    printf("Please enter <source-address> <target-location> <length>\n> ");
    char input[256];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        fclose(file);
        return; 
    }

    unsigned int source_address;
    unsigned int target_location;
    int length;
    if (sscanf(input, "%x %x %d", &source_address, &target_location, &length) != 3) {
        printf("Error: Invalid input format.\n");
        fclose(file);
        return;
    }

    if (debug_mode) {
        fprintf(stderr, "Debug: file_name=%s, source_address=%#x, target_location=%#x, length=%d\n", 
                file_name, source_address, target_location, length);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    if (target_location > file_size) {
        printf("Error: target-location exceeds file size.\n");
        fclose(file);
        return;
    }

    fseek(file, target_location, SEEK_SET);

    unsigned char *buffer = (source_address == 0) ? mem_buf : (unsigned char *)source_address;

    /* Streamlined write using units.c logic */
    fwrite(buffer, unit_size, length, file);

    fclose(file);
}

void memory_modify() {
    printf("Please enter <location> <val>\n> ");
    char input[256];
    if (fgets(input, sizeof(input), stdin) == NULL) return;

    unsigned int location;
    unsigned int val;
    if (sscanf(input, "%x %x", &location, &val) != 2) {
        printf("Error: Invalid input format.\n");
        return;
    }

    if (debug_mode) {
        fprintf(stderr, "Debug: location=%#x, val=%#x\n", location, val);
    }

    if (location + unit_size > sizeof(mem_buf)) {
        printf("Error: Bounds exceeded.\n");
        return;
    }

    /* Modifying memory still requires typecasting to prevent overwriting adjacent bytes */
    if (unit_size == 1) {
        *((unsigned char *)(&mem_buf[location])) = (unsigned char)val;
    } else if (unit_size == 2) {
        *((unsigned short *)(&mem_buf[location])) = (unsigned short)val;
    } else if (unit_size == 4) {
        *((unsigned int *)(&mem_buf[location])) = (unsigned int)val;
    } else {
        printf("Error: Invalid unit size.\n");
    }
}

void quit() {
    if (debug_mode) {
        fprintf(stderr, "quitting\n");                          
    }
    exit(0);                                                    
}

/* ========================================= */
/* MAIN PROGRAM                              */
/* ========================================= */

int main(int argc, char **argv) {
    // Menu mapping array
    struct fun_desc menu[] = {
        {"Toggle <D>ebug Mode", 'D', toggle_debug_mode},             
        {"Set <F>ile Name", 'F', set_file_name},                     
        {"Set <U>nit Size", 'U', set_unit_size},                     
        {"<L>oad Into Memory", 'L', load_into_memory},               
        {"<T>oggle Display Mode", 'T', toggle_display_mode},         
        {"<M>emory Display", 'M', memory_display},                   
        {"<S>ave Into File", 'S', save_into_file},                   
        {"Memory Modif<y>", 'Y', memory_modify},                     
        {"<Q>uit", 'Q', quit},                                       
        {NULL, '\0', NULL}                                            
    };

    char input[10];
    
    // Infinite loop processing
    while (1) {
        if (debug_mode) {
            fprintf(stderr, "\n--- Debug Information ---\n");
            fprintf(stderr, "unit_size: %d\n", unit_size);
            fprintf(stderr, "file_name: %s\n", file_name);
            fprintf(stderr, "mem_count: %zu\n", mem_count);
            fprintf(stderr, "-------------------------\n\n");
        }

        printf("Choose action:\n");                             
        for (int i = 0; menu[i].name != NULL; i++) {
            printf("%s\n", menu[i].name);                 
        }

        printf("> "); 
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; 
        }

        char choice;
        if (sscanf(input, " %c", &choice) != 1) {
            continue; 
        }

        choice = toupper((unsigned char)choice);

        int found = 0;
        for (int i = 0; menu[i].name != NULL; i++) {
            if (toupper((unsigned char)menu[i].index) == choice) {
                menu[i].fun();
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("Invalid option.\n");
        }
        printf("\n");
    }

    return 0;
}