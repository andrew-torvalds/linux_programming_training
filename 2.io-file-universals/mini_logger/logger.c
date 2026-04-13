#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>  // rand()

#define FILE_LOGGER "sensor.log"
#define FILE_ERROR  "error.txt"
#define MSG         "Logger started\n"

int main() {
    // lay thong tin file
    struct stat file_info;
    if(stat(FILE_LOGGER, &file_info) == -1) {
        if(errno == ENOENT) {
            // Error NO ENTry => Tao file moi
            printf("open() system call create new file\n");
        }
        else {
            perror("stat()");
            return -1;
        }
    }

    // group khong duoc khi, other khong duoc doc ghi va thuc thi
    mode_t old_mask = umask(027);

    // open file
    int fd = open(FILE_LOGGER, O_WRONLY | O_CREAT | O_APPEND, 0666);
    if(fd == -1) {
        perror("open()");
        return -1;
    }

    // open file log error, 
    int fderr = open(FILE_ERROR, O_WRONLY | O_CREAT | O_APPEND, 0666);


    // check xem thong tin file error
    struct stat file_error;
    if(stat(FILE_ERROR, &file_error) == -1) {
        perror("stat()");
        close(fderr);
        return -1;
    }
    else {
        if(file_error.st_size > 200) {
            // doi thanh ten khac
            rename(FILE_ERROR, "last_err.txt");
            close(fderr);

            // mo file
            fderr = open(FILE_ERROR, O_WRONLY | O_CREAT | O_APPEND, 0666);
        }
    }

    // redirect stderr > log error file
    dup2(fderr, 2);    // so 2 tuc la fd = 2 => stderr cua he thong
    close(fderr);       // fd=2 đã trỏ vào file rồi, fderr không cần nữa

    if(flock(fd, LOCK_EX | LOCK_NB) == 0) {
        // write intro
        ssize_t len = write(fd, MSG, strlen(MSG));
        if(len == -1) {
            perror("write()");
            close(fd);
            return -1;
        }

        // loop
        while(1) {
            float sensor_value = 20.0 + (rand() % 200) / 10.0;  // gia lap nhiet do tu 20 - 40 do

            // lay thoi gian hien tai
            time_t now = time(NULL);

            // convert to string
            struct tm *t = localtime(&now);
            char timestamp[32];
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", t);

            // combine temp & time
            char log_line[128];
            snprintf(log_line, sizeof(log_line), "[%s] sensor=%.1f\n", timestamp, sensor_value);
            
            ssize_t len = write(fd, log_line, strlen(log_line));
            if(len == -1) {
                perror("write()");
                close(fd);
                return -1;
            }

            // dam bao data xuong disk
            if (fsync(fd) == -1) {
                perror("fsync()");
                close(fd);
                return -1;
            }
                
            sleep(1);  // cho 1 giay
        }
        flock(fd, LOCK_UN);  // unlock
    }
    else {
        fprintf(stderr, "Program đã chạy rồi! Chỉ cho phép 1 instance.\n");
        close(fd);
        umask(old_mask);    // set umask lai nhu cu
        return -1;
    }
    return 0;
}