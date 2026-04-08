# System Calls — Tổng kết kiến thức

> Tài liệu tổng kết toàn bộ kiến thức về System Calls từ *The Linux Programming Interface* (TLPI) — Ch3

---

## 1. Tại sao cần System Call?

### 1.1 Vấn đề

Nếu user program có thể truy cập kernel trực tiếp:

- **Stability**: Process A có thể corrupt memory của process B hoặc kernel
- **Security**: Attacker có thể ghi vào kernel memory để chiếm quyền hệ thống
- **Isolation**: Bug trong 1 process có thể crash toàn bộ hệ thống

### 1.2 Giải pháp — 2 CPU modes

CPU có 2 modes hoàn toàn tách biệt:

```
┌─────────────────────────────────────────────┐
│              USER MODE                       │
│  Chỉ truy cập được memory của process đó    │
│  Không được tác động vào hardware trực tiếp  │
├─────────────────────────────────────────────┤
│              KERNEL MODE                     │
│  Truy cập được mọi thứ                      │
│  Kiểm soát toàn bộ hardware                 │
└─────────────────────────────────────────────┘
```

**System call = cơ chế chuyển đổi CÓ KIỂM SOÁT từ user mode → kernel mode**

> **So sánh STM32 bare metal:**
> - STM32: Program tác động thẳng vào thanh ghi — không có sự phân tách
> - Bug có thể crash toàn bộ MCU, phải reset board
> - Linux: Bug chỉ kill process đó, hệ thống vẫn chạy bình thường

---

## 2. Flow của System Call (x86)

<svg width="100%" viewBox="0 0 680 520" xmlns="http://www.w3.org/2000/svg">
<defs>
  <marker id="arr1" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse">
    <path d="M2 1L8 5L2 9" fill="none" stroke="context-stroke" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/>
  </marker>
</defs>
<!-- USER SPACE -->
<rect x="20" y="20" width="640" height="180" rx="12" fill="#E6F1FB" stroke="#378ADD" stroke-width="1" stroke-dasharray="6 3" opacity="0.6"/>
<text font-size="12" font-weight="500" x="36" y="40" fill="#185FA5" font-family="sans-serif">User space</text>
<!-- User program -->
<rect x="40" y="50" width="180" height="130" rx="8" fill="#B5D4F4" stroke="#378ADD" stroke-width="0.5"/>
<text font-size="13" font-weight="500" x="130" y="73" text-anchor="middle" fill="#0C447C" font-family="sans-serif">User program</text>
<text font-size="11" x="130" y="95" text-anchor="middle" fill="#185FA5" font-family="sans-serif">write(1, "Hi", 2)</text>
<text font-size="10" x="130" y="115" text-anchor="middle" fill="#378ADD" font-family="sans-serif">gọi hàm bình thường</text>
<!-- glibc wrapper -->
<rect x="280" y="50" width="220" height="130" rx="8" fill="#EEEDFE" stroke="#7F77DD" stroke-width="0.5"/>
<text font-size="13" font-weight="500" x="390" y="73" text-anchor="middle" fill="#26215C" font-family="sans-serif">glibc wrapper</text>
<text font-size="11" x="390" y="97" text-anchor="middle" fill="#534AB7" font-family="sans-serif">① %eax = 4 (write)</text>
<text font-size="11" x="390" y="115" text-anchor="middle" fill="#534AB7" font-family="sans-serif">② args → registers</text>
<text font-size="11" x="390" y="133" text-anchor="middle" fill="#534AB7" font-family="sans-serif">③ int 0x80</text>
<text font-size="10" x="390" y="168" text-anchor="middle" fill="#7F77DD" font-family="sans-serif">return kết quả cho user</text>
<!-- Arrows user ↔ glibc -->
<line x1="220" y1="108" x2="278" y2="108" stroke="#378ADD" stroke-width="1.5" marker-end="url(#arr1)"/>
<text font-size="10" x="249" y="101" text-anchor="middle" fill="#185FA5" font-family="sans-serif">call</text>
<path d="M280 148 L250 148 L250 128 L220 128" fill="none" stroke="#1D9E75" stroke-width="1.5" marker-end="url(#arr1)"/>
<text font-size="10" x="235" y="143" text-anchor="middle" fill="#0F6E56" font-family="sans-serif">return</text>
<!-- Mode switch -->
<line x1="20" y1="210" x2="660" y2="210" stroke="#888780" stroke-width="0.8" stroke-dasharray="5 3"/>
<text font-size="11" x="340" y="226" text-anchor="middle" fill="#888780" font-family="sans-serif">── CPU switches to kernel mode (int 0x80 → IDT[128]) ──</text>
<line x1="390" y1="180" x2="390" y2="240" stroke="#BA7517" stroke-width="1.5" stroke-dasharray="4 2" marker-end="url(#arr1)"/>
<!-- KERNEL SPACE -->
<rect x="20" y="240" width="640" height="260" rx="12" fill="#E1F5EE" stroke="#1D9E75" stroke-width="1" stroke-dasharray="6 3" opacity="0.6"/>
<text font-size="12" font-weight="500" x="36" y="260" fill="#0F6E56" font-family="sans-serif">Kernel space</text>
<!-- system_call() handler -->
<rect x="40" y="272" width="200" height="100" rx="8" fill="#9FE1CB" stroke="#1D9E75" stroke-width="0.5"/>
<text font-size="13" font-weight="500" x="140" y="293" text-anchor="middle" fill="#04342C" font-family="sans-serif">system_call()</text>
<text font-size="11" x="140" y="313" text-anchor="middle" fill="#085041" font-family="sans-serif">④ lưu registers</text>
<text font-size="11" x="140" y="331" text-anchor="middle" fill="#085041" font-family="sans-serif">⑤ kiểm tra eax</text>
<text font-size="11" x="140" y="349" text-anchor="middle" fill="#085041" font-family="sans-serif">⑥ tra sys_call_table</text>
<!-- sys_call_table -->
<rect x="298" y="272" width="160" height="100" rx="8" fill="#FAEEDA" stroke="#BA7517" stroke-width="0.5"/>
<text font-size="12" font-weight="500" x="378" y="293" text-anchor="middle" fill="#633806" font-family="sans-serif">sys_call_table[]</text>
<text font-size="11" x="378" y="313" text-anchor="middle" fill="#854F0B" font-family="sans-serif">[0] → sys_read</text>
<text font-size="11" x="378" y="331" text-anchor="middle" fill="#854F0B" font-family="sans-serif">[1] → sys_write</text>
<text font-size="11" x="378" y="349" text-anchor="middle" fill="#D85A30" font-family="sans-serif">[4] → sys_write ←</text>
<!-- sys_write -->
<rect x="518" y="272" width="122" height="100" rx="8" fill="#FAECE7" stroke="#D85A30" stroke-width="0.5"/>
<text font-size="12" font-weight="500" x="579" y="293" text-anchor="middle" fill="#4A1B0C" font-family="sans-serif">sys_write()</text>
<text font-size="11" x="579" y="315" text-anchor="middle" fill="#993C1D" font-family="sans-serif">thực thi!</text>
<text font-size="11" x="579" y="333" text-anchor="middle" fill="#993C1D" font-family="sans-serif">ghi vào</text>
<text font-size="11" x="579" y="351" text-anchor="middle" fill="#993C1D" font-family="sans-serif">buffer cache</text>
<!-- Arrows inside kernel -->
<line x1="240" y1="322" x2="296" y2="322" stroke="#1D9E75" stroke-width="1.5" marker-end="url(#arr1)"/>
<line x1="458" y1="322" x2="516" y2="322" stroke="#BA7517" stroke-width="1.5" marker-end="url(#arr1)"/>
<!-- Return path -->
<path d="M579 372 L579 415 L140 415 L140 374" fill="none" stroke="#534AB7" stroke-width="1.5" stroke-dasharray="5 3" marker-end="url(#arr1)"/>
<text font-size="10" x="360" y="432" text-anchor="middle" fill="#534AB7" font-family="sans-serif">return value → glibc wrapper → user program</text>
<!-- IDT note -->
<rect x="40" y="444" width="200" height="44" rx="8" fill="#F1EFE8" stroke="#888780" stroke-width="0.5"/>
<text font-size="11" font-weight="500" x="140" y="462" text-anchor="middle" fill="#2C2C2A" font-family="sans-serif">IDT[128] = syscall entry</text>
<text font-size="10" x="140" y="480" text-anchor="middle" fill="#5F5E5A" font-family="sans-serif">0x80 = 128 → system_call() address</text>
</svg>

### 2.1 Tại sao dùng `int 0x80` (128)?

```
IDT (Interrupt Descriptor Table):
┌─────┬──────────────────────────────┐
│  0  │ Divide by zero               │
│  1  │ Debug                        │
│ ... │ CPU exceptions (0x00-0x1F)   │ ← Intel reserved
│ 32  │ Hardware interrupts           │
│ ... │ (keyboard, disk, network...) │ ← 0x20-0x7F
│ 128 │ system_call() ◄──────────────│ ← 0x80 = Linux syscall entry
│ ... │ ...                          │
│ 255 │ ...                          │
└─────┴──────────────────────────────┘

→ 0x80 = 128: không conflict với CPU exceptions hay hardware interrupts
→ Linus Torvalds chọn từ đầu, giữ nguyên vì backward compatibility
```

> **Lưu ý:** x86-64 dùng lệnh `syscall` thay vì `int 0x80` — nhanh hơn vì không cần tra IDT.

### 2.2 Tại sao dùng syscall number thay vì địa chỉ hàm?

```
❌ Nếu dùng địa chỉ hàm:
   User truyền địa chỉ bất kỳ → có thể trỏ đến code độc hại trong kernel
   → Attacker chiếm quyền hệ thống!

✅ Dùng syscall number:
   User chỉ truyền số nguyên (ví dụ: 4)
   Kernel tự tra sys_call_table[4] → sys_write()
   User KHÔNG THỂ tự chỉ định địa chỉ kernel
   → Kernel kiểm soát hoàn toàn entry point
```

**Pattern "index vào bảng" — Linux dùng ở nhiều chỗ:**

```
syscall number → sys_call_table[]  → kernel function
fd             → fd table[]        → open file entry
inode number   → inode table[]     → file metadata
```

---

## 3. errno và Error Handling

### 3.1 Cơ chế

Khi syscall thất bại:
- Return **-1**
- Lưu mã lỗi cụ thể vào biến toàn cục **`errno`**

```
open() thất bại
    ↓
kernel set errno = 2  (ENOENT = No such file)
    ↓
open() return -1
    ↓
bạn check: if (fd == -1) → đọc errno để biết lý do
```

### 3.2 2 Rules quan trọng

```
Rule 1: Chỉ đọc errno sau khi syscall return -1
Rule 2: Đọc errno NGAY LẬP TỨC — function call tiếp theo có thể ghi đè!
```

### 3.3 Code đúng chuẩn

```c
// ❌ Sai — printf() có thể ghi đè errno!
int fd = open("file.txt", O_RDONLY);
if (fd == -1) {
    printf("Có lỗi!\n");
    printf("%s\n", strerror(errno));  // errno có thể đã bị ghi đè!
}

// ✅ Đúng — lưu errno ngay lập tức
int fd = open("file.txt", O_RDONLY);
if (fd == -1) {
    int saved_errno = errno;     // lưu ngay!
    printf("Có lỗi!\n");
    printf("%s\n", strerror(saved_errno));
}

// ✅ Đơn giản nhất — dùng perror()
int fd = open("file.txt", O_RDONLY);
if (fd == -1) {
    perror("open()");  // tự đọc và in errno luôn, an toàn!
}
```

### 3.4 Một số errno phổ biến

| Giá trị | Tên | Ý nghĩa |
|---|---|---|
| 2 | ENOENT | File không tồn tại |
| 13 | EACCES | Không có quyền |
| 9 | EBADF | Bad file descriptor |
| 24 | EMFILE | Quá nhiều file đang mở |
| 11 | EAGAIN | Resource temporarily unavailable |
| 4 | EINTR | Interrupted by signal |

### 3.5 errno trong multi-thread

```
Thread A set errno = ENOENT
Thread B set errno = EACCES  ← ghi đè errno của Thread A!

→ Giải pháp: Modern Linux dùng thread-local storage
→ Mỗi thread có errno RIÊNG — không ảnh hưởng nhau
```

---

## 4. Overhead của System Call

### 4.1 Tại sao syscall chậm hơn function call?

```
Function call thường:
┌──────────┐     ┌──────────┐
│ caller   │────▶│ function │  ~0.1 microseconds
└──────────┘     └──────────┘

System call:
┌──────────┐     ┌──────────┐     ┌──────────┐     ┌──────────┐
│ user     │────▶│ context  │────▶│ kernel   │────▶│ context  │
│ program  │     │ save     │     │ handler  │     │ restore  │
└──────────┘     └──────────┘     └──────────┘     └──────────┘
                                                    ~0.3 microseconds
```

**4 nguyên nhân gây overhead:**

```
① Context save/restore  → lưu toàn bộ registers vào kernel stack
② Mode switch           → CPU chuyển privilege level, kiểm tra bảo mật
③ Stack switch          → user và kernel dùng stack riêng
④ Cache flush           → TLB cache có thể bị invalidate khi switch
```

### 4.2 Tối ưu — Minimize syscalls

```c
// ❌ Tệ: 1 triệu syscalls → rất chậm
for (int i = 0; i < 1000000; i++)
    write(fd, &buf[i], 1);

// ✅ Tốt: 1 syscall → nhanh hơn ~1000x
write(fd, buf, 1000000);
```

> **So sánh STM32:** Không có syscall overhead — function gọi thẳng vào hardware. Nhưng cũng không có isolation và security.

---

## 5. System Call vs Library Function

<svg width="100%" viewBox="0 0 680 380" xmlns="http://www.w3.org/2000/svg">
<defs>
  <marker id="arr2" viewBox="0 0 10 10" refX="8" refY="5" markerWidth="6" markerHeight="6" orient="auto-start-reverse">
    <path d="M2 1L8 5L2 9" fill="none" stroke="context-stroke" stroke-width="1.5" stroke-linecap="round" stroke-linejoin="round"/>
  </marker>
</defs>
<!-- USER SPACE -->
<rect x="20" y="20" width="640" height="150" rx="12" fill="#E6F1FB" stroke="#378ADD" stroke-width="1" opacity="0.6"/>
<text font-size="12" font-weight="500" x="36" y="40" fill="#185FA5" font-family="sans-serif">User space</text>
<!-- Library functions -->
<rect x="40" y="50" width="580" height="55" rx="8" fill="#B5D4F4" stroke="#378ADD" stroke-width="0.5"/>
<text font-size="12" font-weight="500" x="330" y="71" text-anchor="middle" fill="#0C447C" font-family="sans-serif">Library Functions — chạy hoàn toàn trong user space</text>
<text font-size="11" x="330" y="93" text-anchor="middle" fill="#185FA5" font-family="sans-serif">printf()     fread()     malloc()     strlen()     strcpy()     fopen()</text>
<!-- Arrows down -->
<line x1="150" y1="105" x2="150" y2="178" stroke="#378ADD" stroke-width="1.5" stroke-dasharray="4 2" marker-end="url(#arr2)"/>
<text font-size="10" x="190" y="145" fill="#185FA5" font-family="sans-serif">gọi syscall</text>
<text font-size="10" x="190" y="158" fill="#185FA5" font-family="sans-serif">(printf→write)</text>
<line x1="450" y1="105" x2="450" y2="140" stroke="#D3D1C7" stroke-width="1" stroke-dasharray="3 3"/>
<text font-size="10" x="480" y="135" fill="#888780" font-family="sans-serif">không gọi syscall</text>
<text font-size="10" x="480" y="148" fill="#888780" font-family="sans-serif">(strlen, strcpy...)</text>
<!-- Boundary line -->
<line x1="20" y1="182" x2="660" y2="182" stroke="#E24B4A" stroke-width="1.5" stroke-dasharray="6 3"/>
<text font-size="11" font-weight="500" x="340" y="198" text-anchor="middle" fill="#A32D2D" font-family="sans-serif">── user mode / kernel mode boundary ──</text>
<!-- KERNEL SPACE -->
<rect x="20" y="208" width="640" height="90" rx="12" fill="#E1F5EE" stroke="#1D9E75" stroke-width="1" opacity="0.6"/>
<text font-size="12" font-weight="500" x="36" y="228" fill="#0F6E56" font-family="sans-serif">Kernel space</text>
<!-- System calls -->
<rect x="40" y="235" width="580" height="50" rx="8" fill="#9FE1CB" stroke="#1D9E75" stroke-width="0.5"/>
<text font-size="12" font-weight="500" x="330" y="256" text-anchor="middle" fill="#04342C" font-family="sans-serif">System Calls — entry point vào kernel</text>
<text font-size="11" x="330" y="275" text-anchor="middle" fill="#085041" font-family="sans-serif">write()     read()     open()     close()     fork()     mmap()     stat()</text>
<!-- Comparison table -->
<rect x="20" y="318" width="640" height="50" rx="8" fill="none" stroke="#888780" stroke-width="0.5"/>
<rect x="20" y="318" width="640" height="22" rx="8" fill="#D3D1C7" stroke="#888780" stroke-width="0.5"/>
<text font-size="11" font-weight="500" x="130" y="333" text-anchor="middle" fill="#2C2C2A" font-family="sans-serif">Đặc điểm</text>
<text font-size="11" font-weight="500" x="340" y="333" text-anchor="middle" fill="#185FA5" font-family="sans-serif">Library Function</text>
<text font-size="11" font-weight="500" x="560" y="333" text-anchor="middle" fill="#0F6E56" font-family="sans-serif">System Call</text>
<text font-size="10" x="40" y="355" fill="#444441" font-family="sans-serif">Chạy ở / Tốc độ</text>
<text font-size="10" x="220" y="355" fill="#185FA5" font-family="sans-serif">User space / Nhanh</text>
<text font-size="10" x="430" y="355" fill="#0F6E56" font-family="sans-serif">Kernel space / ~3x chậm hơn</text>
<line x1="210" y1="318" x2="210" y2="368" stroke="#888780" stroke-width="0.5"/>
<line x1="420" y1="318" x2="420" y2="368" stroke="#888780" stroke-width="0.5"/>
</svg>

**Ví dụ liên kết:**

```
printf("Hello")
    ↓ glibc buffer (stdio buffer)
    ↓ khi buffer đầy hoặc gặp \n
write(1, "Hello", 5)    ← system call
    ↓
kernel buffer cache
    ↓
terminal/disk
```

| | Library Function | System Call |
|---|---|---|
| Ví dụ | `printf()`, `strlen()`, `malloc()` | `write()`, `open()`, `read()` |
| Chạy ở | User space | Kernel space |
| Tốc độ | Nhanh | Chậm hơn ~3x |
| Gọi syscall? | Có thể có hoặc không | Luôn luôn là syscall |

---

## 6. Checklist câu hỏi phỏng vấn

- [ ] System call là gì và tại sao cần thiết?
- [ ] Giải thích flow đầy đủ của system call từ user space đến kernel
- [ ] Tại sao dùng `int 0x80`? Số 128 có ý nghĩa gì?
- [ ] Tại sao dùng syscall number thay vì địa chỉ hàm trực tiếp?
- [ ] errno hoạt động như thế nào? Những rule nào khi dùng errno?
- [ ] Sự khác nhau giữa system call và library function?
- [ ] Tại sao system call chậm hơn function call thường?
- [ ] Làm thế nào để tối ưu khi phải gọi nhiều syscalls?
- [ ] errno trong multi-thread hoạt động thế nào?

---

## 7. So sánh Linux vs STM32 Bare Metal

| | STM32 Bare Metal | Linux |
|---|---|---|
| Truy cập hardware | Thẳng vào thanh ghi | Qua kernel (syscall) |
| Bug ghi sai địa chỉ | Crash toàn MCU | Chỉ kill process đó |
| Isolation | Không có | User/kernel mode |
| Security | Không có | Kernel kiểm soát entry point |
| Overhead | Không có | ~0.3 microseconds/syscall |
| Nhiều chương trình | Khó, conflict | Kernel quản lý, an toàn |

---

*Tài liệu này được tổng kết từ TLPI Chapter 3 — System Programming Concepts*
