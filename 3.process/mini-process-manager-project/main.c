#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define NUMBER_OF_WORKERS 3
pid_t worker_pid[NUMBER_OF_WORKERS];
pid_t ret_pid[NUMBER_OF_WORKERS];
int   ret_status[NUMBER_OF_WORKERS];

int main() {
    // tạo worker process
    for(int i = 0 ; i < NUMBER_OF_WORKERS; i++) {
        worker_pid[i] = fork();
        int worker_id = i + 1;

        // fail khi fork thất bại
        if(worker_pid[i] < 0) {
            perror("fork()");
            exit(1);
        }
        
        // child process
        // gọi trong 1 vòng lặp này luôn, nếu tách ra thì mỗi lần vào process con thì cái i nó phải bị bắt đầu lại, vì mỗi process nó được copy code ra mà
        if(worker_pid[i] == 0) {
            char id_str[10];
            snprintf(id_str, sizeof(id_str), "%d", worker_id);

            char *args[] = {"./worker", id_str, NULL};
            execvp("./worker", args);  // tự tìm trong PATH, không cần đường dẫn đầy đủ

            // fail khi exec thất bại
            perror("execvp()");
            exit(1);
        }

        else {
            printf("[Parent] Tạo worker %d, PID = %d\n", worker_id, worker_pid[i]);
        }
    }

    // process cha wait process con
    int cnt = 0;
    while(1) {
        for(int i = 0 ; i < NUMBER_OF_WORKERS ; i++) {
            // check xem worker_pid đã được collect rồi
            if(worker_pid[i] != 0) {
                ret_pid[i] = waitpid(worker_pid[i], &ret_status[i], WNOHANG);
            
                if(ret_pid[i] < 0) {
                    perror("waitpid()");
                    exit(1);
                }

                else if(ret_pid[i] > 0) {
                    if (WIFEXITED(ret_status[i])) {
                        printf("[Parent] Worker %d đã xong, exit code = %d\n", WEXITSTATUS(ret_status[i]), WEXITSTATUS(ret_status[i]));
                        worker_pid[i] = 0;  // đánh dấu worker đã collect xong
                        cnt++;
                    }               
                }
            }
        }
        if(cnt == 3) break;
    }

    printf("[Parent] Tất cả workers đã xong!\n");

    return 0;
}