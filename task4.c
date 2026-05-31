// task4.c
int count_digits(char *s) {
    int count = 0;
    int i = 0;
    // Iterate until the null-terminator is reached
    while (s[i] != '\0') {
        // Check if the character is between '0' and '9'
        if (s[i] >= '0' && s[i] <= '9') {
            count++;
        }
        i++;
    }
    return count;
}

// Dummy main so it compiles
int main() {
    return 0;
}