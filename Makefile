# Default target to compile everything
all: task1 task4

# Compiling Task 1 (hexeditplus)
task1: task1.c
	gcc -m32 -g -Wall -o task1 task1.c

# Compiling Task 4 (The digit counter patch)
# Using the exact flags required by the lab instructions
task4: task4.c
	gcc -m32 -fno-pie -fno-stack-protector task4.c -o task4

# Clean target to remove generated files
clean:
	rm -f task1 task4 *.o