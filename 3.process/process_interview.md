# Processes — Câu hỏi phỏng vấn

> Tổng hợp 8 câu hỏi phỏng vấn về Processes trong Linux Programming
> Bao gồm câu trả lời của bạn và câu trả lời hoàn chỉnh

---

## Mục lục

| # | Chủ đề | Kết quả |
|---|---|---|
| 1 | [Process vs Program](#câu-1-process-vs-program) | ✅ Tốt |
| 2 | [Memory Layout](#câu-2-memory-layout) | ✅ Tốt |
| 3 | [Copy-on-Write](#câu-3-copy-on-write) | ✅ Tốt |
| 4 | [fork() return 2 lần](#câu-4-fork-return-2-lần) | ✅ Tốt |
| 5 | [exec()](#câu-5-exec) | ✅ Tốt nhất |
| 6 | [exit() vs _exit()](#câu-6-exit-vs-_exit) | ✅ Tốt |
| 7 | [Zombie process](#câu-7-zombie-process) | ✅ Tốt |
| 8 | [Process Credentials](#câu-8-process-credentials) | ✅ Tốt |

---

## Câu 1: Process vs Program

**Câu hỏi:**
> "Anh/chị có thể giải thích sự khác nhau giữa process và program không? Và tại sao Linux cần chạy nhiều process cùng lúc?"

---

**Câu trả lời của bạn:**
> Process là tiến trình đang chạy, tiêu tốn tài nguyên. Program chỉ là chương trình được build ra từ source code, nằm trên disk và không tiêu tốn tài nguyên. Linux cần đa tác vụ nên cần nhiều process. Khác với thiết bị nhúng không OS — không chạy đa tác vụ, không linh hoạt.

**Nhận xét:**
- ✅ Process = đang chạy, tốn tài nguyên
- ✅ Program = trên disk, không tốn tài nguyên
- ✅ Nhắc đến đa tác vụ
- ✅ So sánh với bare metal
- ❌ Chưa nhắc đến **cô lập** — lý do quan trọng thứ 2
- ❌ Chưa nhắc **memory riêng** — điểm phân biệt với FreeRTOS task

---

**Câu trả lời hoàn chỉnh:**
> Program là file thực thi trên disk — không tốn RAM, không tốn CPU. Process là program đang chạy — có memory riêng, PID, đang tiêu tốn tài nguyên.
>
> Linux cần nhiều process vì 2 lý do:
> - **Đa tác vụ** — nhiều việc chạy cùng lúc (sensor reader, web server, SSH...)
> - **Cô lập** — process A bug chỉ kill process A, không ảnh hưởng process B
>
> Khác với FreeRTOS — tasks share cùng RAM nên bug một task có thể crash toàn hệ thống.

---

## Câu 2: Memory Layout

**Câu hỏi:**
> "Anh/chị hãy giải thích 5 vùng nhớ trong memory layout của một process. Vùng nào nằm ở địa chỉ cao nhất, vùng nào thấp nhất?"

---

**Câu trả lời của bạn:**
> Vùng thấp nhất là Text, cao nhất là Stack. 5 vùng: Stack, Heap, BSS, Data, Text. Stack chứa các biến và hàm trong main(). Heap dùng cấp phát bộ nhớ động. Stack phình xuống, Heap phình lên. BSS lưu biến khai báo không có giá trị. Data lưu biến có giá trị. Text chứa source code.

**Nhận xét:**
- ✅ Thứ tự đúng: Text thấp, Stack cao
- ✅ Liệt kê đủ 5 vùng
- ✅ Heap grows up, Stack grows down
- ✅ BSS = chưa khởi tạo, Data = đã khởi tạo
- ❌ "Stack chứa biến và hàm trong main()" — Stack chứa **biến local** và **function call frames** ở mọi function, không chỉ main(). Hàm (code) nằm trong Text!
- ❌ BSS và Data chỉ áp dụng cho **biến global và static**
- ❌ Text là **read-only** — chưa nhắc

---

**Câu trả lời hoàn chỉnh:**
> 5 vùng từ địa chỉ thấp lên cao: Text → Data → BSS → Heap → Stack.
>
> - **Text**: code của program — **read only**, kernel không cho ghi vào
> - **Data**: biến **global/static đã khởi tạo** — `int x = 10;` ở global
> - **BSS**: biến **global/static chưa khởi tạo** — kernel tự set = 0
> - **Heap**: cấp phát động bằng `malloc()` — grows up, developer tự quản lý
> - **Stack**: biến **local** và function call frames ở mọi function — grows down, tự động quản lý
>
> Stack và Heap grow về phía nhau — đệ quy quá sâu hoặc malloc quá nhiều → Stack Overflow!

---

## Câu 3: Copy-on-Write

**Câu hỏi:**
> "Anh/chị giải thích cơ chế Copy-on-Write sau fork(). Tại sao Linux không copy toàn bộ memory ngay lập tức?"

---

**Câu trả lời của bạn:**
> Nếu copy toàn bộ ngay lập tức sẽ tốn thời gian và làm nặng chương trình.

**Nhận xét:**
- ✅ Tốn thời gian — đúng
- ✅ Tốn tài nguyên — đúng hướng
- ❌ Chưa nói cụ thể tốn **RAM**
- ❌ Chưa nêu tình huống fork() + exec() — lý do CoW quan trọng nhất

---

**Câu trả lời hoàn chỉnh:**
> Nếu copy toàn bộ ngay — process cha 100MB → fork() copy 100MB → tốn thời gian và RAM.
>
> Thực tế hầu hết trường hợp con exec() ngay sau fork() → toàn bộ memory bị thay thế → 100MB vừa copy bị xóa → lãng phí hoàn toàn!
>
> CoW giải quyết:
> - fork() xong: cha và con **share cùng trang nhớ vật lý** (read-only)
> - Khi có ai **ghi vào** → kernel mới copy đúng trang đó
> - Nếu con exec() ngay → không copy gì cả → tiết kiệm hoàn toàn!

---

## Câu 4: fork() return 2 lần

**Câu hỏi:**
> "Tại sao cha nhận PID của con, còn con nhận 0? Thiết kế như vậy có ý nghĩa gì thực tế?"

---

**Câu trả lời của bạn:**
> Process cha cần biết PID của con để quản lý. Process con nhận 0 để biết vai trò của mình là con.

**Nhận xét:**
- ✅ Cha cần PID để quản lý con — đúng
- ✅ Con nhận 0 để biết vai trò — đúng
- ❌ Chưa nêu cụ thể "quản lý" là làm gì — waitpid(), kill()

---

**Câu trả lời hoàn chỉnh:**
> fork() là syscall — kernel thiết lập sẵn return value cho cả 2 bằng cách ghi thẳng vào register %eax trước khi cho chạy.
>
> **Cha nhận PID con** vì cần quản lý con:
> ```c
> waitpid(pid, &status, 0);  // chờ đúng con này
> kill(pid, SIGTERM);         // kill đúng con này
> ```
>
> **Con nhận 0** vì cần biết vai trò để chạy đúng code:
> ```c
> if (pid == 0) {
>     execl("/bin/ls", "ls", NULL);  // con làm việc của con
> }
> ```
>
> Nếu cha nhận 0 → không biết PID con → không quản lý được. Thiết kế này tách biệt rõ ràng vai trò cha/con.

---

## Câu 5: exec()

**Câu hỏi:**
> "Anh/chị giải thích exec() làm gì với memory layout? Tại sao exec() không bao giờ return khi thành công?"

---

**Câu trả lời của bạn:**
> exec() dùng để gọi chương trình bên ngoài theo đường dẫn và tham số. exec() thay thế toàn bộ memory layout bằng memory layout của chương trình được gọi. exec() không return vì chương trình mới đã có exit() riêng. exec() chỉ return khi thất bại.

**Nhận xét:**
- ✅ Thay thế toàn bộ memory layout — đúng
- ✅ Chương trình mới có exit() riêng — suy luận rất hay!
- ✅ Chỉ return khi thất bại — đúng
- ❌ Chưa nhắc **PID không đổi** sau exec()
- ❌ Chưa nhắc **fd table được kế thừa**

---

**Câu trả lời hoàn chỉnh:**
> exec() thay thế toàn bộ memory layout — Text, Data, BSS, Heap, Stack đều mới. Nhưng **PID không đổi** — vẫn là cùng process, chỉ khác chương trình.
>
> exec() không return vì không còn stack frame để return về — stack cũ đã bị xóa sạch.
>
> **fd table được kế thừa** — đó là lý do có thể redirect stdout trước exec():
> ```c
> dup2(fd, 1);              // redirect stdout → file
> execl("/bin/ls", "ls");   // ls kế thừa fd table → ghi vào file!
> ```

---

## Câu 6: exit() vs _exit()

**Câu hỏi:**
> "Sự khác nhau giữa exit() và _exit()? Trong tình huống nào bắt buộc phải dùng _exit()?"

---

**Câu trả lời của bạn:**
> exit() dọn dẹp buffer trên user rồi mới xuống kernel. _exit() chạy thẳng xuống kernel và flush buffer kernel 1 lần.

**Nhận xét:**
- ✅ exit() dọn dẹp user space trước — đúng
- ✅ _exit() thẳng xuống kernel — đúng
- ❌ "_exit() flush kernel buffer" — SAI! _exit() không flush kernel buffer. Kernel buffer được flush bởi fsync(), không phải _exit()

---

**Câu trả lời hoàn chỉnh:**
> ```
> exit(0):
>     ① Gọi atexit() handlers
>     ② fflush() stdio buffer (user space)
>     ③ Gọi _exit()
>
> _exit(0):
>     Thẳng vào kernel — KHÔNG flush gì cả
>     Kernel đóng fd, giải phóng memory
> ```
>
> **Bắt buộc dùng _exit() khi process con sau fork():**
> ```c
> printf("Hello");   // nằm trong stdio buffer của cha
> fork();            // con copy stdio buffer → con cũng có "Hello"
>
> if (pid == 0) {
>     exit(0);   // ❌ flush → in "Hello" 2 lần!
>     _exit(0);  // ✅ không flush → chỉ in 1 lần
> }
> ```

---

## Câu 7: Zombie Process

**Câu hỏi:**
> "Zombie process là gì? Tại sao nguy hiểm với embedded Linux chạy 24/7? Bạn xử lý như thế nào?"

---

**Câu trả lời của bạn:**
> Zombie là process con đã chạy xong nhưng cha không thu thập dọn dẹp. Nguy hiểm vì chiếm PID và PID có giới hạn. Xử lý bằng waitpid().

**Nhận xét:**
- ✅ Zombie = con xong, cha không collect — đúng
- ✅ Chiếm PID, PID có giới hạn — đúng
- ✅ waitpid() để xử lý — đúng
- ❌ "tiêu tốn data chương trình" — không chính xác, zombie chỉ giữ 1 entry PCB rất nhỏ
- ❌ Chưa nhắc 3 cách xử lý cụ thể

---

**Câu trả lời hoàn chỉnh:**
> Zombie = process con đã kết thúc nhưng cha chưa gọi waitpid(). Kernel giữ lại PID và exit status chờ cha collect.
>
> Zombie không tốn RAM hay CPU — chỉ tốn **1 entry trong process table**. Nguy hiểm vì chiếm PID, tối đa 32768. Embedded device fork() liên tục không waitpid() → hết PID → fork() thất bại → crash!
>
> **3 cách xử lý:**
> ```c
> // ① Blocking
> waitpid(pid, &status, 0);
>
> // ② Nonblocking trong vòng lặp
> waitpid(-1, &status, WNOHANG);
>
> // ③ Kernel tự xóa
> signal(SIGCHLD, SIG_IGN);
> ```

---

## Câu 8: Process Credentials

**Câu hỏi:**
> "Process Credentials là gì? Tại sao `./sensor_reader` bị Permission denied khi mở `/dev/i2c-1`, nhưng `sudo ./sensor_reader` thì chạy được?"

---

**Câu trả lời của bạn:**
> User chạy ./sensor_reader không có quyền sử dụng. Thêm sudo thì hệ thống chạy với root.

**Nhận xét:**
- ✅ Đúng hướng
- ❌ Chưa giải thích **cơ chế cụ thể** — UID/GID, kernel check permissions thế nào
- ❌ Chưa nhắc **fix đúng chuẩn** — không dùng sudo mà thêm user vào group

---

**Câu trả lời hoàn chỉnh:**
> Process credentials gồm UID và GID — kernel dùng để kiểm tra quyền truy cập.
>
> `./sensor_reader` thất bại vì:
> ```
> Process UID=1000 (user thường)
> /dev/i2c-1: group=i2c, permissions=660
> UID=1000 không phải owner, không thuộc group i2c
> → kernel check "other" permissions = 0 → Permission denied!
> ```
>
> `sudo ./sensor_reader` thành công vì:
> ```
> sudo có SUID bit → eUID=0 (root)
> → kernel check eUID=0 → root bỏ qua mọi permissions!
> ```
>
> **Fix đúng chuẩn embedded Linux — không dùng sudo:**
> ```bash
> sudo usermod -aG i2c giang  # thêm user vào group i2c
> # Process có GID=i2c → match group /dev/i2c-1 → cho phép!
> ```

---

## Tổng kết

| Câu | Chủ đề | Điểm mạnh | Cần cải thiện |
|---|---|---|---|
| 1 | Process vs Program | So sánh bare metal tốt | Thêm cô lập, memory riêng |
| 2 | Memory Layout | Thứ tự đúng | Stack chứa local mọi function |
| 3 | Copy-on-Write | Hiểu cơ bản | Tình huống fork+exec |
| 4 | fork() return 2 lần | Hiểu sâu cơ chế kernel | Nêu cụ thể waitpid(), kill() |
| 5 | exec() | **Tốt nhất** — suy luận hay | PID không đổi, fd kế thừa |
| 6 | exit() vs _exit() | Đúng hướng | _exit() không flush kernel buffer |
| 7 | Zombie | Xác định đúng nguyên nhân | 3 cách xử lý cụ thể |
| 8 | Process Credentials | Đúng hướng | Cơ chế UID/GID, fix đúng chuẩn |

**Nhận xét chung:**
- Tiến bộ rõ rệt so với File I/O
- Câu 5 về exec() — tự suy luận rất tốt
- Cần cải thiện: diễn đạt **cơ chế cụ thể** thay vì nói chung chung

---

*Tổng hợp từ buổi phỏng vấn thử — Processes Linux Programming*
