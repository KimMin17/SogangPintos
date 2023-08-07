#include <stdio.h>
#include <syscall.h>
#include <string.h>

int string_to_int(char*);
int char_to_int(char);

int main(int argc, char* argv[]) {
    if(argc != 5) return EXIT_FAILURE;
    int n1 = string_to_int(argv[1]);
    int n2 = string_to_int(argv[2]);
    int n3 = string_to_int(argv[3]);
    int n4 = string_to_int(argv[4]);
    printf("%d %d\n", fibonacci(n1), max_of_four_int(n1, n2, n3, n4));
    return EXIT_SUCCESS;
}

int string_to_int(char* s) {
    int len = strlen(s);
    int result = 0;
    int exp = 1;
    int i;
    for(i = len-1; i >= 0; i--) {
        result += exp * char_to_int(s[i]);
        exp *= 10;
    }
    return result;
}

int char_to_int(char c) {
    return c - 48;
}