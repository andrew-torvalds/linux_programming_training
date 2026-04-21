#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {

    printf("Truoc fork");   // không có \n → nằm trong stdio buffer
                            // cha có "Truoc fork" trong buffer

    pid_t pid = fork();     // con COPY buffer của cha
                            // con cũng có "Truoc fork" trong buffer!

    if (pid == 0) {
        // ── Test với exit() ──
        // exit(0);
        // → flush buffer → in "Truoc fork" lần 1
        // → cha sau đó flush → in "Truoc fork" lần 2
        // Output: Truoc forkTruoc fork ❌

        // ── Test với _exit() ──
        _exit(0);
        // → KHÔNG flush buffer → con thoát im lặng
        // → cha sau đó flush → in "Truoc fork" 1 lần
        // Output: Truoc fork ✅

    } else {
        waitpid(pid, NULL, 0);
        printf("\nCha tiep tuc\n");
    }

    return 0;
}

/*
    CÁCH FIX ĐẶT NGAY SAU printf("Truoc fork");
    fflush(stdout);      // flush TRƯỚC fork → buffer trống
                         // → con copy buffer trống
                         // → không lo in 2 lần dù dùng exit()

*/