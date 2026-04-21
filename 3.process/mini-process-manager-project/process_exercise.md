# Bài tập: Mini Process Manager

> Bài tập thực hành tổng hợp kiến thức Processes — Linux Programming
> Mức độ: 7/10

---

## Mô tả bài toán

Viết chương trình C mô phỏng một **process manager đơn giản** trên embedded Linux. Chương trình tạo và quản lý nhiều worker process, mỗi worker làm việc độc lập và báo cáo kết quả về cha.

---

## Yêu cầu chức năng

### ① Parent process — quản lý workers
- Tạo **3 worker process** bằng `fork()`
- Mỗi worker nhận **worker_id** riêng (1, 2, 3)
- Cha **không block** — dùng `WNOHANG` để collect worker khi xong
- Cha in ra exit status của từng worker khi kết thúc
- Cha thoát khi **tất cả workers** đã xong

### ② Worker process — làm việc độc lập
- Mỗi worker `exec()` program `./worker`
- Worker nhận `worker_id` qua `argv[1]`
- Worker giả lập công việc bằng `sleep(worker_id)`
- Worker in ra: `[Worker ID] Đang chạy, PID = X`
- Worker kết thúc với `exit code = worker_id`

### ③ Xử lý lỗi đúng chuẩn
- Kiểm tra `fork()` thất bại
- Kiểm tra `exec()` thất bại
- Phân biệt `WIFEXITED` và `WIFSIGNALED`
- Dùng `_exit()` hoặc `exit()` đúng chỗ

### ④ Tránh các vấn đề phổ biến
- Không để process con tiếp tục fork() trong vòng lặp
- Không gọi `waitpid()` 2 lần cho cùng 1 worker
- Đảm bảo không có zombie process

---

## Kết quả mong đợi

```
[Parent] Tạo worker 1, PID = 29268
[Parent] Tạo worker 2, PID = 29269
[Parent] Tạo worker 3, PID = 29270
[Worker 1] Đang chạy, PID = 29268
[Worker 2] Đang chạy, PID = 29269
[Worker 3] Đang chạy, PID = 29270
[Parent] Worker 1 đã xong, exit code = 1
[Parent] Worker 2 đã xong, exit code = 2
[Parent] Worker 3 đã xong, exit code = 3
[Parent] Tất cả workers đã xong!
```

---

## Cấu trúc project

```
process_manager/
├── main.c      ← parent process manager
└── worker.c    ← worker program
```

---

## Compile và chạy

```bash
gcc -o main main.c
gcc -o worker worker.c
./main
```

---

## Concepts được luyện tập

| Concept | Syscall/Function | Chỗ áp dụng |
|---|---|---|
| Tạo process | `fork()` | Tạo 3 worker |
| Thay chương trình | `exec()` | Worker chạy program riêng |
| Chờ con không block | `waitpid()` + `WNOHANG` | Cha collect worker |
| Kiểm tra exit status | `WIFEXITED` + `WEXITSTATUS` | Kiểm tra worker xong |
| Truyền argument | `argv[]` + `atoi()` | Truyền worker_id |
| Tránh zombie | Đánh dấu đã collect | Không gọi waitpid() 2 lần |

---

*Bài tập tổng hợp Processes — TLPI Ch6, 24, 25, 26*
