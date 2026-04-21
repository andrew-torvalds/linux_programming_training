#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork()");
        exit(1);

    } 
    else if (pid == 0) {
        // ── Process con ──
        printf("Con đang chạy, PID: %d\n", getpid());
        sleep(2);
        exit(42);  // kết thúc với code 42
    } 
    else {
        // Cha không block — làm việc khác trong khi chờ con
        int status;
        while (1) {
            pid_t done = waitpid(pid, &status, WNOHANG);

            if (done == -1) {
                perror("waitpid()");
                break;

            } else if (done == 0) {
                // Con chưa xong → cha làm việc khác
                printf("Con chưa xong, cha làm việc khác...\n");
                sleep(1);

            } else {
                // Con đã xong
                if (WIFEXITED(status))
                    printf("Con xong! Exit code: %d\n", WEXITSTATUS(status));
                break;
            }
        }
    }
}