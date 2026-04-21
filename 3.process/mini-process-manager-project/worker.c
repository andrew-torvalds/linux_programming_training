#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // thiếu truyền vào đối số ID cho worker
    if(argc < 2) return 0;

    int worker_id = atoi(argv[1]);

    printf("[Worker %d] Đang chạy, PID = %d\n", worker_id, getpid());
    
    // worker sleep
    // int worker_sleep = argv[1] - 48;
    sleep(worker_id);
    
    exit(worker_id);
}