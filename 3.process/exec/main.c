#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        // Process con — exec program của mình
        printf("before execl\n\n");

        // execl — truyền từng argument riêng lẻ
        execl("/bin/ls", "ls", "-la", "/home", NULL);
        

        /* execvp — truyền bằng array (tiện hơn khi có nhiều args) */
        // char *args[] = {"ls", "-la", "/home", NULL};
        // execvp("ls", args);  // tự tìm trong PATH, không cần đường dẫn đầy đủ

        // nếu exec thất bại sẽ in ra dòng này
        printf("after execl");

        perror("execl()");
        exit(1);
    } 
    else {
        waitpid(pid, NULL, 0);
        printf("\nđã xong!\n");
    }
    return 0;
}