# Bài tập: Mini Logger System cho Embedded Linux

> Bài tập thực hành tổng hợp kiến thức File I/O — Linux Programming
> Mức độ: 7/10

---

## Mô tả bài toán

Viết một chương trình C mô phỏng hệ thống ghi log sensor trên embedded Linux device. Chương trình đọc giá trị sensor (giả lập), ghi vào file log với timestamp, và đảm bảo an toàn dữ liệu khi mất điện đột ngột.

---

## Yêu cầu chức năng

### ① Kiểm tra file trước khi mở
- Dùng `stat()` kiểm tra file log có tồn tại không
- Nếu chưa tồn tại (`errno == ENOENT`) → tiếp tục tạo mới, không báo lỗi
- Nếu tồn tại → kiểm tra có phải regular file không (`S_ISREG`)
- Nếu không phải regular file → báo lỗi và thoát

### ② Permissions đúng chuẩn
- Set `umask(027)` trước khi tạo file
- Tạo file log với mode `0666` — sau khi qua umask sẽ là `0640`
- Kết quả: owner đọc/ghi, group chỉ đọc, other không có quyền gì

### ③ Chống chạy nhiều instance
- Dùng `flock()` với `LOCK_EX | LOCK_NB` để lock file log
- Nếu đã có instance đang chạy → in thông báo rõ ràng và thoát
- Không dùng blocking lock — tránh chương trình bị treo

### ④ Redirect stderr vào error log
- Mở file `error.txt` để ghi error log
- Dùng `dup2()` redirect `fd=2` (stderr) vào `error.txt`
- Sau khi redirect → mọi `perror()` và `fprintf(stderr,...)` đều ghi vào file
- Đóng fd gốc sau khi `dup2()`

### ⑤ Log rotation cho error.txt
- Kiểm tra size của `error.txt` trước khi redirect
- Nếu size > 200 bytes → đổi tên thành `last_err.txt`
- Tạo `error.txt` mới trống

### ⑥ Ghi log sensor liên tục
- Vòng lặp vô tận, mỗi 1 giây ghi 1 dòng
- Giả lập giá trị nhiệt độ sensor: 20.0 - 40.0 độ (dùng `rand()`)
- Format mỗi dòng log: `[YYYY-MM-DD HH:MM:SS] sensor=XX.X`
- Dùng `time()` + `strftime()` để lấy timestamp

### ⑦ Đảm bảo data an toàn
- Sau mỗi lần `write()` → gọi `fsync()` để đảm bảo data xuống disk
- Nếu `fsync()` thất bại → báo lỗi và thoát

### ⑧ Error handling đầy đủ
- Kiểm tra return value của tất cả syscalls
- Dùng `perror()` để in lỗi
- `close()` fd trước khi thoát trong mọi trường hợp lỗi

---

## Kết quả mong đợi

**Cấu trúc files sau khi chạy:**
```
mini_logger/
├── logger.c
├── sensor.log       ← log sensor, permissions: rw-r-----
└── error.txt        ← error log, permissions: rw-r-----
```

**Nội dung sensor.log:**
```
Logger started
[2026-04-12 20:58:39] sensor=38.3
[2026-04-12 20:58:40] sensor=28.6
[2026-04-12 20:58:41] sensor=37.7
...
```

**Khi chạy instance thứ 2:**
```
$ ./logger
Program đã chạy rồi! Chỉ cho phép 1 instance.
```

---

## Compile và chạy

```bash
gcc -o logger logger.c
./logger

# Kiểm tra permissions
ls -la sensor.log error.txt

# Xem log đang ghi
tail -f sensor.log

# Test chạy nhiều instance
./logger &   # chạy nền
./logger     # thử chạy lần 2 → báo lỗi
```

---

## Concepts được luyện tập

| Concept | Syscall/Function | Chỗ áp dụng |
|---|---|---|
| Kiểm tra file | `stat()` + `S_ISREG` | Trước khi open |
| Mở file | `open()` + flags | Mở log file |
| Ghi file | `write()` | Ghi log mỗi giây |
| Đảm bảo xuống disk | `fsync()` | Sau mỗi write |
| Permissions | `umask()` + mode | Khi tạo file |
| File locking | `flock()` | Chống nhiều instance |
| Redirect fd | `dup2()` | Redirect stderr |
| Log rotation | `stat()` + `rename()` | Khi file quá lớn |
| Error handling | `perror()` + `errno` | Mọi syscall |
| Timestamp | `time()` + `strftime()` | Format dòng log |

---

## Hướng làm bài

### Bước 1 — Khung cơ bản
Viết chương trình đơn giản nhất trước:
- Set `umask(027)`
- `open()` file log với `O_WRONLY | O_CREAT | O_APPEND`
- `write()` 1 dòng "Logger started\n"
- `fsync()` đảm bảo xuống disk
- `close()`

Compile và kiểm tra file được tạo đúng permissions chưa.

### Bước 2 — Thêm stat() và vòng lặp
- Thêm `stat()` kiểm tra file trước khi `open()`
- Xử lý `errno == ENOENT` — file chưa tồn tại là bình thường
- Kiểm tra `S_ISREG` nếu file đã tồn tại
- Thêm vòng lặp `while(1)` + `sleep(1)`
- Giả lập sensor bằng `rand()`
- Format dòng log với timestamp bằng `time()` + `strftime()`

### Bước 3 — flock() và dup2()
- Thêm `flock(fd, LOCK_EX | LOCK_NB)` sau khi `open()`
- Xử lý trường hợp lock thất bại — in thông báo rõ ràng
- Mở `error.txt` cho error log
- Kiểm tra rotation error.txt (`stat()` + `rename()`)
- `dup2(fderr, 2)` redirect stderr
- `close(fderr)` ngay sau `dup2()`

### Lưu ý quan trọng
```
① Thứ tự đúng:
   stat() error.txt → xử lý rotation → dup2() → close(fderr)
   Không dup2() trước rồi mới stat() — vì sau dup2()
   perror() ghi vào file, không hiển thị terminal!

② close() fd gốc sau dup2():
   dup2(fderr, 2);
   close(fderr);  ← bắt buộc!

③ flock() không bao giờ unlock trong vòng lặp vô tận:
   → kernel tự unlock khi process kết thúc
   → đây là behavior đúng cho daemon

④ fsync() thất bại phải thoát:
   → không bỏ qua, không tiếp tục
   → data integrity quan trọng hơn uptime
```

---

*Bài tập tổng hợp File I/O — TLPI Ch4, 5, 13, 14, 15, 18*
