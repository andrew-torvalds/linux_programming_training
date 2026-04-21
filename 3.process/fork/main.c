#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char const *argv[])                      /* Cấp phát stack frame cho hàm main() */
{
    pid_t child_pid;                                        /* Lưu trong stack frame của main() */
    int counter = 2;                                        /* Lưu trong frame của main() */
    printf("Gia tri khoi tao cua counter: %d\n", counter);
    child_pid = fork();         
    if (child_pid >= 0) {
        if (0 == child_pid) {                               /* Process con */
            printf("\nIm the child process, counter: %d\n", ++counter);
            printf("My PID is: %d, my parent PID is: %d\n", getpid(), getppid());
            
        } else {                    /* Process cha */
            printf("\nIm the parent process, counter: %d\n", ++counter);
            printf("My PID is: %d\n", getpid());
        while (1);
        }
    } else {
        printf("fork() unsuccessfully\n");                   // fork() return -1 nếu lỗi.
    }
    return 0;
}

/**
Khi ta chạy chương trình trên, biến counter đang bằng 2 khi này ta gọi fork để tạo thêm tiến trình con, 
    khi này biến counter đã được nhân bản thành 2, một bản thuộc process cha và 1 bản thuộc process con. 

    Giá trị của counter đều bằng 2 ở mỗi tiến trình. 

Nên khi ta cộng thêm 1 thì cả 2 process đều tằng biến counter lên 3.
*/