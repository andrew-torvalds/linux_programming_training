# File I/O — Câu hỏi phỏng vấn

> Tổng hợp 7 câu hỏi phỏng vấn về File I/O trong Linux Programming
> Bao gồm câu trả lời của bạn và câu trả lời hoàn chỉnh

---

## Mục lục

| # | Chủ đề | Kết quả |
|---|---|---|
| 1 | [Universal I/O Model](#câu-1-universal-io-model) | ✅ Đúng hướng |
| 2 | [3 tầng bảng khi open()](#câu-2-3-tầng-bảng-khi-open) | ✅ Tốt |
| 3 | [Blocking vs Nonblocking](#câu-3-blocking-vs-nonblocking) | ✅ Tốt |
| 4 | [Buffering + fsync()](#câu-4-buffering--fsync) | ✅ Đúng concept |
| 5 | [Permissions + chmod](#câu-5-permissions--chmod) | ✅ Đúng hướng |
| 6 | [fd leak debug](#câu-6-fd-leak-debug) | ✅ Tốt |
| 7 | [File Locking](#câu-7-file-locking) | ✅ Tốt nhất |

---

## Câu 1: Universal I/O Model

**Câu hỏi:**
> "Anh/chị có thể giải thích tại sao trong Linux, mọi thứ đều được trừu tượng hóa thành file không? Điều này mang lại lợi ích gì cho lập trình viên embedded Linux?"

---

**Câu trả lời của bạn:**
> Bởi vì Linux có nhiều thiết bị drivers khác nhau, sử dụng với mục đích khác nhau chẳng hạn như network, disk... vì vậy Linux được trừu tượng hóa thành 1 cơ chế gọi là Virtual File System.

**Nhận xét:**
- ✅ Nhắc đến VFS — đúng
- ✅ Biết có nhiều loại thiết bị khác nhau
- ❌ Chưa nói rõ lợi ích cụ thể cho embedded Linux
- ❌ Chưa đề cập uniformity — 1 API cho tất cả

---

**Câu trả lời hoàn chỉnh:**
> Linux trừu tượng hóa mọi thứ thành file vì hệ thống có nhiều loại tài nguyên khác nhau — disk, UART, I2C, network socket... Thay vì mỗi loại có API riêng, Linux dùng VFS làm tầng trung gian, cho phép dùng cùng 1 API `open()`/`read()`/`write()` cho tất cả.
>
> Lợi ích cho embedded Linux:
> - Developer không cần học API riêng cho từng peripheral
> - `/dev/ttyUSB0` (UART), `/dev/i2c-1` (I2C) đều đọc/ghi bằng `read()`/`write()`
> - Process không tác động thẳng vào hardware — kernel kiểm soát qua VFS, tránh conflict giữa các process

---

## Câu 2: 3 tầng bảng khi open()

**Câu hỏi:**
> "Khi một process gọi `open()` để mở file, kernel làm những gì bên dưới? Hãy mô tả liên quan đến các cấu trúc dữ liệu trong kernel."

---

**Câu trả lời của bạn:**
> Khi một process gọi `open()` thì kernel sẽ tìm file xem nó có tồn tại không, tùy vào flag mình dùng mà nó sẽ có trường hợp không tồn tại mà khởi tạo file mới. Tiếp theo khi kernel tìm được file trên disk nó sẽ dựa vào số inode tạo ra 1 entry nằm ở file table, rồi trả về một số nguyên file descriptor lớn hơn 3 cho process đã gọi `open()`.

**Nhận xét:**
- ✅ Biết kernel kiểm tra file tồn tại
- ✅ Nhắc đến O_CREAT — đúng
- ✅ Biết inode → entry trong file table
- ❌ Chưa nhắc **pathname resolution** — kernel duyệt từng directory
- ❌ Chưa nói rõ **3 tầng bảng** đầy đủ
- ❌ Chưa nhắc **kiểm tra permissions**
- ❌ fd không nhất thiết > 3 — là số nhỏ nhất available

---

**Câu trả lời hoàn chỉnh:**
> Khi `open()` được gọi, kernel thực hiện các bước:
>
> 1. **Pathname resolution** — duyệt từng directory từ root để tìm inode của file
> 2. **Kiểm tra permissions** — xem process có quyền mở theo flags không
> 3. **Tạo open file entry** trong open file table (tầng 2) — lưu offset=0 và flags
> 4. Open file entry trỏ đến **inode** (tầng 3) trên disk
> 5. Tạo **fd** trong fd table (tầng 1) của process — trỏ đến open file entry
> 6. Return fd — số nguyên **nhỏ nhất available** (không nhất thiết > 3)

---

## Câu 3: Blocking vs Nonblocking

**Câu hỏi:**
> "Sự khác nhau giữa `read()` trên regular file và `read()` trên UART device trong embedded Linux là gì? Trong trường hợp nào bạn sẽ dùng nonblocking I/O?"

---

**Câu trả lời của bạn:**
> Khi `read()` trên regular file thì nó sẽ dựa vào size của file mà trả về kích thước và chương trình sẽ không bị treo vì nó đã biết trước kích thước. Tuy nhiên đối với device file thì khác, nó sẽ bị block khi thỏa mãn hai điều kiện là không đọc được data từ process hoặc thiết bị khác gửi đến và nó ở chế độ blocking. Trường hợp không muốn process bị treo tại `read()` thì dùng chế độ nonblocking để chạy các function bên dưới.

**Nhận xét:**
- ✅ Phân biệt đúng regular file không block, device file có thể block
- ✅ Hiểu đúng nonblocking để tránh process bị treo
- ❌ "dựa vào size của file" — chưa chính xác, `read()` đọc từ **offset hiện tại**, return 0 khi EOF
- ❌ Diễn đạt điều kiện block hơi lộn xộn — đơn giản hơn: **không có data trong kernel buffer**

---

**Câu trả lời hoàn chỉnh:**
> `read()` trên **regular file**: đọc từ offset hiện tại, nếu hết file thì return 0 (EOF) ngay — không bao giờ block vì data đã có sẵn trên disk.
>
> `read()` trên **UART device**: nếu không có data trong kernel buffer, process bị block — đứng chờ cho đến khi hardware gửi data đến.
>
> Dùng **nonblocking I/O** khi cần đọc nhiều fd cùng lúc — ví dụ vừa đọc UART vừa đọc I2C. Với `O_NONBLOCK`, `read()` return -1 và errno=EAGAIN ngay nếu không có data, process tiếp tục làm việc khác thay vì bị treo.
>
> ```c
> int flags = fcntl(fd, F_GETFL);
> fcntl(fd, F_SETFL, flags | O_NONBLOCK);
>
> ssize_t n = read(fd_uart, buf, sizeof(buf));
> if (n == -1 && errno == EAGAIN) {
>     // Không có data → làm việc khác
>     read(fd_i2c, buf2, sizeof(buf2));
> }
> ```

---

## Câu 4: Buffering + fsync()

**Câu hỏi:**
> "Bạn đang viết chương trình chạy nền trên embedded Linux, liên tục ghi log sensor vào file. Sau khi `write()` thành công và return, thiết bị mất điện đột ngột. Data có được lưu không? Bạn xử lý tình huống này như thế nào?"

---

**Câu trả lời của bạn:**
> Nếu mất điện đột ngột thì khả năng rất cao data sẽ không được ghi vào disk, bởi vì trước khi vào disk nó phải chạy qua 2 tầng RAM — một là RAM user space và hai là RAM kernel space. Nếu cẩn thận sẽ có hai hàm để đảm bảo việc đẩy data xuống.

**Nhận xét:**
- ✅ Biết data qua 2 tầng RAM trước khi xuống disk
- ✅ Biết có 2 hàm để đảm bảo
- ❌ Chưa nói tên 2 hàm: `fflush()` và `fsync()`
- ❌ Chưa giải thích tại sao cần cả 2
- ❌ Chưa nhắc khi nào nên dùng trong embedded

---

**Câu trả lời hoàn chỉnh:**
> Sau khi `write()` return, data chỉ nằm trong **kernel buffer cache** (RAM) — chưa xuống disk. Nếu mất điện lúc này, data mất hoàn toàn.
>
> Để đảm bảo data xuống disk cần gọi cả 2:
> - `fflush(fp)` — flush stdio buffer (user space RAM) xuống kernel
> - `fsync(fd)` — flush kernel buffer cache xuống disk thật sự
>
> Cần cả 2 vì `fflush()` không biết kernel buffer, `fsync()` không biết stdio buffer.
>
> ```c
> fprintf(fp, "sensor=25.5\n");
> fflush(fp);              // stdio → kernel
> fsync(fileno(fp));       // kernel → disk
> ```
>
> Trong embedded Linux, chỉ dùng `fsync()` cho data quan trọng như config file hay firmware update — không dùng cho mọi log vì sẽ làm chậm và tốn wear cycles của flash storage.

---

## Câu 5: Permissions + chmod

**Câu hỏi:**
> "Bạn có file `/etc/app.conf` trên embedded device. Bạn muốn đảm bảo chỉ có root mới đọc được file này vì nó chứa database password. Bạn sẽ làm gì? Và tại sao không dùng `chmod 000`?"

---

**Câu trả lời của bạn:**
> Để đảm bảo mình sẽ đưa quyền sở hữu file cho root, và ngăn chặn các hành vi read/write/execute của user, group và other. Không dùng `chmod 000` thì mình chưa biết.

**Nhận xét:**
- ✅ Biết cần đưa ownership cho root
- ✅ Biết cần ngăn chặn user/group/other
- ❌ Chưa nói command cụ thể: `chown` và `chmod`
- ❌ Chưa giải thích tại sao không dùng `chmod 000`

---

**Câu trả lời hoàn chỉnh:**
> Mình sẽ làm 2 bước:
> ```bash
> chown root:root /etc/app.conf  # đưa ownership cho root
> chmod 600 /etc/app.conf        # chỉ root đọc/ghi, group và other không có gì
> ```
>
> Không dùng `chmod 000` vì dù root vẫn đọc được, nhưng program thường chạy với quyền user thường — sẽ không đọc được config của chính nó và bị crash khi khởi động.
>
> Nếu muốn program đọc được mà user khác không đọc được:
> ```bash
> chown root:appgroup /etc/app.conf
> chmod 640 /etc/app.conf  # root đọc/ghi, appgroup chỉ đọc, other không có gì
> ```

---

## Câu 6: fd leak debug

**Câu hỏi:**
> "Bạn đang debug một embedded Linux device chạy 24/7. Sau 2 tiếng hoạt động, mọi `open()` đều thất bại với errno=EMFILE. Nguyên nhân là gì và bạn fix như thế nào?"

---

**Câu trả lời của bạn:**
> Trong 2 tiếng đó mình đã `open()` rất nhiều và chắc chắn là không có `close()` lại, vì vậy file descriptor và file table bị quá giới hạn. Fix thì chỉ cần `close()` file thôi.

**Nhận xét:**
- ✅ Xác định đúng nguyên nhân: fd leak
- ✅ Biết fix bằng cách `close()` đúng chỗ
- ❌ Chưa nhắc tên giới hạn: **RLIMIT_NOFILE = 1024**
- ❌ Chưa nói cách **debug tìm chỗ bị leak** trong production

---

**Câu trả lời hoàn chỉnh:**
> Đây là **fd leak** — `open()` được gọi trong vòng lặp nhưng không có `close()` tương ứng. Linux mặc định giới hạn `RLIMIT_NOFILE = 1024` fd per process. Sau 2 tiếng → hết 1024 fd → mọi `open()` thất bại với `EMFILE`.
>
> **Debug:**
> ```bash
> ls /proc/PID/fd | wc -l    # đếm số fd đang mở
> ls -la /proc/PID/fd        # xem chi tiết fd nào đang mở
> ```
>
> **Fix:**
> ```c
> // ❌ Bug
> while (1) {
>     int fd = open("/dev/sensor", O_RDONLY);
>     read(fd, buf, 10);
>     // quên close!
>     sleep(1);
> }
>
> // ✅ Fix
> while (1) {
>     int fd = open("/dev/sensor", O_RDONLY);
>     read(fd, buf, 10);
>     close(fd);  // close ngay sau khi dùng xong!
>     sleep(1);
> }
> ```

---

## Câu 7: File Locking

**Câu hỏi:**
> "Trong embedded Linux, bạn có 2 process cùng ghi vào file log `/var/log/sensor.log`. Vấn đề gì có thể xảy ra và bạn giải quyết như thế nào? Khi nào dùng `flock()`, khi nào dùng `fcntl()` record locking?"

---

**Câu trả lời của bạn:**
> Có một khái niệm gọi là race condition, tức là 2 process cùng ghi vào 1 file mà lại cùng 1 offset, điều này khiến xảy ra hiện tượng ghi đè dữ liệu. Để giải quyết vấn đề này mình có thể `open()` file log ra nhưng ở 2 offset khác nhau. Mình có thể dùng `flock()` để lock toàn bộ file khi đang sử dụng và flock có cơ chế chờ hoặc không chờ khi đọc trúng file lock. `fcntl()` thì đặc biệt hơn, nó có thể lock phân vùng file, tức là khóa tại byte 100-200 hoặc bất kỳ vị trí nào.

**Nhận xét:**
- ✅ Xác định đúng race condition và ghi đè dữ liệu
- ✅ Biết `flock()` lock toàn bộ file
- ✅ Biết `fcntl()` lock từng vùng bytes
- ✅ Nhắc đến blocking/nonblocking của flock
- ❌ "open() file ở 2 offset khác nhau" — không giải quyết được race condition
- ❌ Chưa nói rõ khi nào dùng cái nào

---

**Câu trả lời hoàn chỉnh:**
> 2 process cùng ghi vào 1 file có thể xảy ra **race condition** — ghi đè lên nhau nếu cùng offset, hoặc data bị xen kẽ lộn xộn.
>
> Dùng **`flock()`** khi cần lock đơn giản toàn bộ file:
> ```c
> flock(fd, LOCK_EX);           // blocking — chờ đến khi lock được
> flock(fd, LOCK_EX | LOCK_NB); // nonblocking — return ngay nếu không lock được
> // ghi log...
> flock(fd, LOCK_UN);           // unlock
> ```
>
> Dùng **`fcntl()` record locking** khi nhiều process làm việc trên các vùng khác nhau — ví dụ database với nhiều records:
> ```c
> struct flock fl = {
>     .l_type   = F_WRLCK,
>     .l_whence = SEEK_SET,
>     .l_start  = 0,
>     .l_len    = 100   // chỉ lock bytes 0-100
> };
> fcntl(fd, F_SETLKW, &fl);  // blocking
> ```
>
> **Quy tắc:** File log đơn giản → `flock()`. Database nhiều records → `fcntl()` record locking.

---

## Tổng kết

| Câu | Chủ đề | Điểm mạnh | Cần cải thiện |
|---|---|---|---|
| 1 | Universal I/O Model | Nhắc VFS | Nêu lợi ích cụ thể |
| 2 | 3 tầng bảng | Biết flow cơ bản | Pathname resolution, permissions |
| 3 | Blocking/Nonblocking | Phân biệt đúng | Diễn đạt chính xác hơn |
| 4 | Buffering + fsync | Đúng concept 2 tầng RAM | Nêu tên hàm cụ thể |
| 5 | Permissions | Đúng hướng | Tại sao không dùng chmod 000 |
| 6 | fd leak | Xác định đúng nguyên nhân | Debug bằng /proc |
| 7 | File Locking | Câu tốt nhất | Khi nào dùng cái nào |

**Nhận xét chung:**
- Hiểu **concept** tốt — không có câu nào sai hoàn toàn
- Cần cải thiện **diễn đạt chính xác** hơn
- Cần nhớ **tên hàm và command cụ thể** thay vì nói chung chung

---

*Tổng hợp từ buổi phỏng vấn thử — File I/O Linux Programming*
