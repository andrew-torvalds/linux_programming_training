#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>

int main() {
    pid_t pid  = getpid();   // PID của process hiện tại
    pid_t ppid = getppid();  // PID của process cha
    uid_t uid  = getuid();   // User ID
    gid_t gid  = getgid();   // Group ID

    printf("pid %d\n", pid);
    printf("ppid %d\n", ppid);
    printf("uid %d\n", uid);
    printf("gid %d\n", gid);
    return 0;
}
