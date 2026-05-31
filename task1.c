#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Global state variables as specified in Lab 4 */
char debug_mode = 0;               
char file_name[128] = "";          
int unit_size = 1;                 
unsigned char mem_buf[10000];      
size_t mem_count = 0;      
int display_mode = 0;        

/* Updated Struct for the menu, combining Lab 1 logic and Lab 4 constraints */
struct fun_desc {
    char *name;
    char index;
    void (*fun)();                 // Kept as void (*fun)() per Lab 4 instructions
};

/* ========================================= */
/* STUBS FOR UNIMPLEMENTED TASKS             */
/* ========================================= */

void load_into_memory() {
    // 1. Check if the file name is empty
    if (strcmp(file_name, "") == 0) {
        printf("Error: File name is empty. Please set it first using option 'F'.\n");
        return;
    }

    // 2. Open the file for reading in binary mode
    FILE *file = fopen(file_name, "rb");
    if (file == NULL) {
        printf("Error: Could not open file '%s'.\n", file_name);
        return;
    }

    // 3. Prompt the user for location and length
    printf("Please enter <location> <length>\n> ");
    char input[256];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        fclose(file);
        return; // Handle unexpected EOF
    }

    unsigned int location;
    int length;
    
    // Read location in hexadecimal (%x) and length in decimal (%d)
    if (sscanf(input, "%x %d", &location, &length) != 2) {
        printf("Error: Invalid input format. Expected <hex location> <dec length>.\n");
        fclose(file);
        return;
    }

    // 4. Debug output
    if (debug_mode) {
        fprintf(stderr, "Debug: file_name=%s, location=%#x, length=%d\n", file_name, location, length);
    }

    // Calculate total bytes to read based on current unit size
    size_t bytes_to_read = length * unit_size;
    
    // Safety check against buffer overflow (since mem_buf is 10000 bytes)
    if (bytes_to_read > sizeof(mem_buf)) {
        printf("Error: Requested read size (%zu bytes) exceeds memory buffer capacity.\n", bytes_to_read);
        fclose(file);
        return;
    }

    // 5. Seek to the specified location in the file
    if (fseek(file, location, SEEK_SET) != 0) {
        printf("Error: Could not seek to location %#x in file.\n", location);
        fclose(file);
        return;
    }

    // Read the requested bytes directly into our global mem_buf
    size_t bytes_read = fread(mem_buf, 1, bytes_to_read, file);
    
    // Update our global count of how many valid bytes are in our buffer
    mem_count = bytes_read; 

    // 6. Close the file
    fclose(file);

    // 7. Print success message
    printf("Loaded %d units into memory\n", length);
}

void toggle_display_mode() {
    if (display_mode == 0) {
        // Currently off (hexadecimal), so turn it on (decimal)
        display_mode = 1;
        printf("Decimal display flag now on, decimal representation\n");
    } else {
        // Currently on (decimal), so turn it off (hexadecimal)
        display_mode = 0;
        printf("Decimal display flag now off, hexadecimal representation\n");
    }
}

void memory_display() {
    // Format arrays exactly as provided in the Lab 4 instructions
    static char* hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
    static char* dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};

    printf("Enter address and length\n> ");
    char input[256];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return; // Handle EOF safely
    }

    unsigned int addr;
    int length;
    
    // Read address in hexadecimal (%x) and length in decimal (%d)
    if (sscanf(input, "%x %d", &addr, &length) != 2) {
        printf("Error: Invalid input format. Expected <hex addr> <dec length>.\n");
        return;
    }

    if (debug_mode) {
        fprintf(stderr, "Debug: addr=%#x, length=%d\n", addr, length);
    }

    // Determine the starting pointer: 
    // 0 is the special case for mem_buf. Otherwise, cast addr to a physical pointer.
    unsigned char *start_ptr = (addr == 0) ? mem_buf : (unsigned char *)addr;

    // Print the appropriate header based on display_mode from Task 1b
    if (display_mode == 1) {
        printf("Decimal\n=======\n");
    } else {
        printf("Hexadecimal\n===========\n");
    }

    // Iterate through the requested length, fetching the correct unit sizes
    for (int i = 0; i < length; i++) {
        int val = 0;
        
        // Pointer arithmetic and casting to read exactly 1, 2, or 4 bytes
        if (unit_size == 1) {
            val = *((unsigned char *)(start_ptr + (i * unit_size)));
        } else if (unit_size == 2) {
            val = *((unsigned short *)(start_ptr + (i * unit_size)));
        } else if (unit_size == 4) {
            val = *((unsigned int *)(start_ptr + (i * unit_size)));
        } else {
            printf("Invalid unit size.\n");
            return;
        }

        // Print using the predefined arrays. We subtract 1 because unit sizes 
        // are 1, 2, 4 but array indices are 0, 1, 3.
        if (display_mode == 1) {
            printf(dec_formats[unit_size - 1], val);
        } else {
            printf(hex_formats[unit_size - 1], val);
        }
    }
}

void save_into_file() {
    // 1. Check if the file name is empty
    if (strcmp(file_name, "") == 0) {
        printf("Error: File name is empty. Please set it first using option 'F'.\n");
        return;
    }

    // 2. Open the file for writing (without truncating)
    FILE *file = fopen(file_name, "r+");
    if (file == NULL) {
        printf("Error: Could not open file '%s'.\n", file_name);
        return;
    }

    // 3. Prompt the user for addresses and length
    printf("Please enter <source-address> <target-location> <length>\n> ");
    char input[256];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        fclose(file);
        return; 
    }

    unsigned int source_address;
    unsigned int target_location;
    int length;
    
    // Read source and target in hexadecimal (%x) and length in decimal (%d)
    if (sscanf(input, "%x %x %d", &source_address, &target_location, &length) != 3) {
        printf("Error: Invalid input format. Expected <hex> <hex> <dec>.\n");
        fclose(file);
        return;
    }

    // 4. Debug output
    if (debug_mode) {
        fprintf(stderr, "Debug: file_name=%s, source_address=%#x, target_location=%#x, length=%d\n", 
                file_name, source_address, target_location, length);
    }

    // 5. File size validation
    fseek(file, 0, SEEK_END);        // Move cursor to the end of the file
    long file_size = ftell(file);    // Get the exact byte count
    
    if (target_location > file_size) {
        printf("Error: target-location (%#x) exceeds file size (%ld bytes).\n", target_location, file_size);
        fclose(file);
        return;
    }

    // 6. Navigate to the target location for writing
    if (fseek(file, target_location, SEEK_SET) != 0) {
        printf("Error: Could not seek to target-location %#x in file.\n", target_location);
        fclose(file);
        return;
    }

    // 7. Determine the source pointer based on the special '0' case
    unsigned char *start_ptr = (source_address == 0) ? mem_buf : (unsigned char *)source_address;

    // 8. Write the exact number of units to the file
    fwrite(start_ptr, unit_size, length, file);

    // 9. Close the file
    fclose(file);
}

void memory_modify() {
    // 1. Prompt the user for location and value
    printf("Please enter <location> <val>\n> ");
    char input[256];
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return; // Handle EOF safely
    }

    unsigned int location;
    unsigned int val;
    
    // 2. Read both location and val in hexadecimal (%x)
    if (sscanf(input, "%x %x", &location, &val) != 2) {
        printf("Error: Invalid input format. Expected <hex location> <hex val>.\n");
        return;
    }

    // 3. Debug output
    if (debug_mode) {
        fprintf(stderr, "Debug: location=%#x, val=%#x\n", location, val);
    }

    // 4. Validate that the location and unit size fit within our buffer
    if (location + unit_size > sizeof(mem_buf)) {
        printf("Error: Location %#x with unit size %d exceeds memory buffer bounds.\n", location, unit_size);
        return;
    }

    // 5. Replace a unit in the memory buffer with the new value
    unsigned char *target_ptr = &mem_buf[location];

    if (unit_size == 1) {
        *((unsigned char *)target_ptr) = (unsigned char)val;
    } else if (unit_size == 2) {
        *((unsigned short *)target_ptr) = (unsigned short)val;
    } else if (unit_size == 4) {
        *((unsigned int *)target_ptr) = (unsigned int)val;
    } else {
        printf("Error: Invalid unit size.\n");
    }
}      

/* ========================================= */
/* IMPLEMENTED FUNCTIONS                     */
/* ========================================= */

void toggle_debug_mode() {
    if (debug_mode == 0) {
        debug_mode = 1;
        fprintf(stderr, "Debug flag now on\n");                 
    } else {
        debug_mode = 0;
        fprintf(stderr, "Debug flag now off\n");                
    }
}

void set_file_name() {
    printf("Enter file name: ");
    char input[100];                                            
    if (fgets(input, sizeof(input), stdin) != NULL) {
        input[strcspn(input, "\n")] = 0; // Strip newline character
        strncpy(file_name, input, 128);  // Store in file_name
        if (debug_mode) {
            fprintf(stderr, "Debug: file name set to '%s'\n", file_name); 
        }
    }
}

void set_unit_size() {
    printf("Enter unit size (1, 2, or 4): ");                   
    char input[10];
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
    // Menu array using the character index logic
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
        // If debug mode is on, print state variables before menu
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

        printf("> "); // Matches the example in the Lab 4 Word doc
        
        // This is where EOF is caught. If you pipe an empty file or press Ctrl+D, 
        // fgets returns NULL, and the loop breaks instantly.
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; 
        }

        char choice;
        if (sscanf(input, " %c", &choice) != 1) {
            continue; // Ignore empty lines
        }

        // Convert the user's input to uppercase to handle 'd' or 'D' seamlessly
        choice = toupper((unsigned char)choice);

        // Search the menu array for the corresponding index
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