# Hướng dẫn giải: Mini Process Manager

> Tổng hợp các vấn đề gặp phải và cách giải quyết

---

## Bước 1 — Viết worker.c trước

Worker đơn giản hơn — bắt đầu từ đây để hiểu cách nhận argument từ exec().

**Vấn đề hay gặp:**

**① exec() chỉ truyền được string — không truyền được int:**
```c
// ❌ Sai — exec() không nhận int
execl("./worker", "worker", worker_id, NULL);

// ✅ Đúng — convert int → string trước
char id_str[10];
snprintf(id_str, sizeof(id_str), "%d", worker_id);
execl("./worker", "worker", id_str, NULL);
```

Worker nhận lại bằng `atoi()`:
```c
int worker_id = atoi(argv[1]);  // string → int
```

**② Thiếu `\n` trong printf:**
```c
// ❌ Sai — nằm trong stdio buffer mãi không hiển thị
printf("[Worker %d] Đang chạy, PID = %d", worker_id, getpid());

// ✅ Đúng
printf("[Worker %d] Đang chạy, PID = %d\n", worker_id, getpid());
```

**③ argv[0] là tên program, argv[1] mới là argument đầu tiên:**
```c
// exec() truyền vào
char *args[] = {"./worker", id_str, NULL};
//              ↑           ↑
//            argv[0]     argv[1]  ← worker_id ở đây!

// worker.c nhận
int worker_id = atoi(argv[1]);  // không phải argv[0]!
```

---

## Bước 2 — Viết main.c — fork() trong vòng lặp

**Vấn đề quan trọng nhất: process con tiếp tục fork() trong vòng lặp!**

```c
// ❌ Bug — process con vẫn ở trong vòng lặp và fork() tiếp!
for(int i = 0; i < 3; i++) {
    worker_pid[i] = fork();
    // Con được tạo ở i=0 → tiếp tục vòng lặp → i=1 → fork() tiếp!
}
```

Hậu quả:
```
i=0: Cha tạo Con1
i=1: Cha tạo Con2, Con1 tạo Con1a  ← không mong muốn!
i=2: Cha tạo Con3, Con2 tạo Con2a, Con1 tạo Con1b, Con1a tạo Con1a1...
→ Tạo ra rất nhiều process không mong muốn!
```

**Fix — gộp fork() và exec() trong cùng 1 vòng lặp:**
```c
for(int i = 0; i < NUMBER_OF_WORKERS; i++) {
    worker_pid[i] = fork();
    int worker_id = i + 1;

    if(worker_pid[i] < 0) {
        perror("fork()");
        exit(1);
    }

    else if(worker_pid[i] == 0) {
        // Con biết worker_id = i+1 ngay tại đây!
        // → exec() luôn, không cần vòng lặp thứ 2
        char id_str[10];
        snprintf(id_str, sizeof(id_str), "%d", worker_id);
        char *args[] = {"./worker", id_str, NULL};
        execvp("./worker", args);
        perror("execvp()");
        exit(1);  // exec thất bại → thoát, không chạy xuống waitpid()!
    }

    else {
        // Cha tiếp tục vòng lặp
        printf("[Parent] Tạo worker %d, PID = %d\n", worker_id, worker_pid[i]);
    }
}
```

**Tại sao không tách thành 2 vòng lặp riêng?**
```
Vòng lặp 1: fork() — Con1 break ra với i=0
Vòng lặp 2: exec() — i reset về 0
    → Con1 check worker_pid[0] == 0 → đúng → exec worker 1 ✅
    → Con2 check worker_pid[0] == 0?
       worker_pid[0] trong Con2 = PID của Con1 (khác 0!) ← BUG!
    → Con2 không exec được đúng worker_id!
```

---

## Bước 3 — waitpid() với WNOHANG

**Vấn đề: gọi waitpid() 2 lần cho cùng 1 worker:**

```
Lần 1: waitpid(worker1) → return pid → collect ✅
Lần 2: waitpid(worker1) → return -1, errno=ECHILD ❌
        worker1 đã collect rồi, không còn child nữa!
```

**Fix — đánh dấu worker đã collect:**
```c
// Sau khi collect xong → đánh dấu pid = 0
if(ret_pid[i] > 0) {
    if(WIFEXITED(ret_status[i])) {
        printf("[Parent] Worker %d đã xong, exit code = %d\n",
            i + 1, WEXITSTATUS(ret_status[i]));
        worker_pid[i] = 0;  // đánh dấu đã collect!
        cnt++;
    }
}

// Trước khi gọi waitpid() — check đã collect chưa
if(worker_pid[i] != 0) {
    ret_pid[i] = waitpid(worker_pid[i], &ret_status[i], WNOHANG);
    ...
}
```

---

## Bước 4 — Điều kiện dừng vòng lặp

**Vấn đề: while(1) không có điều kiện dừng → chạy mãi mãi:**

```c
// ✅ Đếm số worker đã xong
int cnt = 0;
while(1) {
    for(int i = 0; i < NUMBER_OF_WORKERS; i++) {
        // collect worker...
        cnt++;
    }
    if(cnt == NUMBER_OF_WORKERS) break;  // tất cả xong → thoát!
}
```

---

## Tóm tắt các bugs theo thứ tự gặp phải

| # | Bug | Nguyên nhân | Fix |
|---|---|---|---|
| 1 | Thiếu `\n` trong printf | stdio buffer không flush | Thêm `\n` |
| 2 | Process con fork() tiếp | Con vẫn trong vòng lặp | Gộp fork+exec 1 vòng lặp |
| 3 | argv format sai | argv[0] = tên program | args[] = {"./worker", id_str, NULL} |
| 4 | In Worker ID sai | Dùng WEXITSTATUS thay vì i+1 | Dùng i+1 cho Worker ID |
| 5 | waitpid() gọi 2 lần | Không đánh dấu đã collect | worker_pid[i] = 0 sau collect |
| 6 | while(1) không dừng | Không có điều kiện thoát | Đếm cnt == NUMBER_OF_WORKERS |

---

## Bài học rút ra

**① fork() trong vòng lặp — luôn gộp exec() vào cùng vòng lặp:**
```
Process con được tạo ra tại i=N
→ Nó biết worker_id = N+1 ngay tại thời điểm đó
→ exec() luôn trong nhánh if(pid==0)
→ Không cần vòng lặp thứ 2
```

**② exec() không bao giờ return khi thành công:**
```
→ Process con chỉ đến được exit(1) khi exec() thất bại
→ Đảm bảo process con không chạy xuống code của cha
```

**③ Luôn đánh dấu worker đã collect:**
```
→ Tránh waitpid() gọi 2 lần → ECHILD error
→ Dùng worker_pid[i] = 0 hoặc mảng boolean riêng
```

**④ WEXITSTATUS và Worker ID là 2 thứ khác nhau:**
```
Worker ID = thứ tự worker (1, 2, 3) — dùng i+1
Exit code = giá trị worker truyền vào exit() — dùng WEXITSTATUS
Trong bài này 2 thứ trùng nhau — nhưng thực tế production nên tách biệt!
```

---

*Hướng dẫn giải bài tập Mini Process Manager — Processes Linux Programming*
