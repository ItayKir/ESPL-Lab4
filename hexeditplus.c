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

void toggle_display_mode() { printf("Not implemented yet\n"); } 
void memory_display() { printf("Not implemented yet\n"); }      
void save_into_file() { printf("Not implemented yet\n"); }      
void memory_modify() { printf("Not implemented yet\n"); }       

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