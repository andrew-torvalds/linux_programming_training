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
        // ── Process cha ──
        printf("Cha đang chờ con PID: %d\n", pid);

        int status;
        pid_t done = waitpid(pid, &status, 0);  // blocking

        if (done == -1) {
            perror("waitpid()");
            exit(1);
        }

        // Kiểm tra con kết thúc thế nào
        if (WIFEXITED(status)) {
            printf("Con kết thúc bình thường\n");
            printf("Exit code: %d\n", WEXITSTATUS(status));

        } 
        else if (WIFSIGNALED(status)) {
            printf("Con bị kill bởi signal: %d\n", WTERMSIG(status));
        }
    }

    return 0;
}