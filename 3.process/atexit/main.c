#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void cleanup1() { printf("atexit: cleanup1 chạy\n"); }
void cleanup2() { printf("atexit: cleanup2 chạy\n"); }

int main() {

    // Đăng ký cleanup handlers
    atexit(cleanup1);
    atexit(cleanup2);

    printf("Chuẩn bị thoát...\n");

    exit(0);
    // Output:
    // Chuẩn bị thoát...
    // atexit: cleanup2 chạy  ← LIFO
    // atexit: cleanup1 chạy
}