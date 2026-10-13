# BÁO CÁO BÀI TẬP LỚN / BÀI TẬP NHÓM
## HỌC PHẦN: ET3310 - LÝ THUYẾT MẬT MÃ
### ĐỀ TÀI: TRIỂN KHAI THUẬT TOÁN MÃ HÓA DÒNG RABBIT STREAM CIPHER (RFC 4503)

---

## 👥 THÀNH VIÊN NHÓM (GROUP MEMBERS)

| STT | Họ và Tên (Full Name) | MSSV (Student ID) |
| :---: | :--- | :---: |
| **01** | ĐỖ  ĐỨC HUY | 20233441 |


---

## 📑 MỤC LỤC
1. [Giới thiệu tổng quan đề tài](#1-giới-thiệu-tổng-quan-đề-tài)
2. [Cơ sở lý thuyết thuật toán Rabbit (RFC 4503)](#2-cơ-sở-lý-thuyết-thuật-toán-rabbit-rfc-4503)
3. [Kiến trúc thư mục và Thiết kế module](#3-kiến-trúc-thư-mục-và-thiết-kế-module)
4. [Các kỹ thuật tối ưu hóa mã nguồn (Optimizations)](#4-các-kỹ-thuật-tối-ưu-hóa-mã-nguồn-optimizations)
5. [Kiểm thử tính đúng đắn & Đối chiếu Test Vectors](#5-kiểm-thử-tính-đúng-đắn--đối-chiếu-test-vectors)
6. [Đánh giá hiệu năng và Thông lượng (Throughput)](#6-đánh-giá-hiệu-năng-và-thông-lượng-throughput)
7. [Hướng dẫn biên dịch và sử dụng](#7-hướng-dẫn-biên-dịch-và-sử-dụng)
8. [Hướng dẫn Git & GitHub cho thành viên (GIT_GUIDE.md)](./GIT_GUIDE.md)
9. [Kết luận](#9-kết-luận)

---

## 1. Giới thiệu tổng quan đề tài

Trong khuôn khổ môn học **ET3310**, bài tập lớn này tập trung nghiên cứu và hiện thực hóa thuật toán mật mã dòng **Rabbit Stream Cipher**, được chuẩn hóa trong tài liệu quốc tế **RFC 4503** và là một trong những thuật toán lọt vào danh mục chung cuộc (Final Portfolio) của dự án **eSTREAM Profile 1** (phần mềm hiệu năng cao) do Mạng lưới Mật mã châu Âu ECRYPT chủ trì.

### Mục tiêu đề tài:
- **Triển khai chuẩn mực**: Khắc phục triệt để các sai lệch logic từ các mã nguồn tham khảo sơ khai, tuân thủ 100% đặc tả toán học theo RFC 4503.
- **Tối ưu hóa hiệu năng**: Kết hợp tối ưu hóa bitwise, giảm thiểu chu kỳ lệnh CPU, loại bỏ phép chia số nguyên 64-bit chậm, hỗ trợ tự động sinh mã máy instruction-level (ROL/ROR).
- **Kiến trúc phần mềm C++ hiện đại**: Phân tách rạch ròi giao diện (.h) và thực thi (.cpp), đóng gói trạng thái loại bỏ biến toàn cục.
- **Bảo toàn dữ liệu File I/O**: Hỗ trợ mã hóa/giải mã mọi định dạng file (văn bản, PDF, hình ảnh, nhị phân) mà không làm biến dạng dữ liệu.

---

## 2. Cơ sở lý thuyết thuật toán Rabbit (RFC 4503)

Rabbit là một mật mã dòng đồng bộ (synchronous stream cipher) xử lý các khối 128-bit với:
- **Độ dài Khóa bí mật (Secret Key)**: 128 bits (16 bytes).
- **Độ dài Vector khởi tạo (IV)**: 64 bits (8 bytes).
- **Không gian trạng thái bên trong (Internal State)**: Tổng cộng 513 bits, gồm:
  - 8 biến trạng thái 32-bit: $X_0, X_1, \dots, X_7$ (256 bits).
  - 8 biến đếm Counter 32-bit: $C_0, C_1, \dots, C_7$ (256 bits).
  - 1 biến nhớ cờ nhớ (Carry bit): $b \in \{0, 1\}$ (1 bit).

### 2.1. Hệ thống bộ đếm (Counter System)
Trước mỗi vòng lặp Next-State, các biến đếm được cập nhật theo chuỗi hằng số bất đối xứng $A_0 \dots A_7$:
$$A_0 = A_3 = A_6 = \text{0x4D34D34D}, \quad A_1 = A_4 = A_7 = \text{0xD34D34D3}, \quad A_2 = A_5 = \text{0x34D34D34}$$
Cơ chế cập nhật:
$$\text{temp} = C_j + A_j + b$$
$$b = \lfloor \text{temp} / 2^{32} \rfloor, \quad C_j = \text{temp} \pmod{2^{32}}$$

### 2.2. Hàm phi tuyến $g(u, v)$
Hàm $g$ nhận hai giá trị 32-bit và ánh xạ thành một giá trị 32-bit có độ khuếch tán cao:
$$g(u, v) = \text{LSW}\left((u + v \pmod{2^{32}})^2\right) \oplus \text{MSW}\left((u + v \pmod{2^{32}})^2\right)$$

### 2.3. Hàm chuyển trạng thái (Next-State Function)
$$G_j = g(X_j, C_j) \quad (0 \le j \le 7)$$
Các biến trạng thái mới được cập nhật thông qua phép quay bit trái (ROL) và cộng mod $2^{32}$:
$$\begin{aligned}
X_0 &= G_0 + (G_7 \lll 16) + (G_6 \lll 16) \pmod{2^{32}} \\
X_1 &= G_1 + (G_0 \lll 8) + G_7 \pmod{2^{32}} \\
X_2 &= G_2 + (G_1 \lll 16) + (G_0 \lll 16) \pmod{2^{32}} \\
X_3 &= G_3 + (G_2 \lll 8) + G_1 \pmod{2^{32}} \\
X_4 &= G_4 + (G_3 \lll 16) + (G_2 \lll 16) \pmod{2^{32}} \\
X_5 &= G_5 + (G_4 \lll 8) + G_3 \pmod{2^{32}} \\
X_6 &= G_6 + (G_5 \lll 16) + (G_4 \lll 16) \pmod{2^{32}} \\
X_7 &= G_7 + (G_6 \lll 8) + G_5 \pmod{2^{32}}
\end{aligned}$$

### 2.4. Khởi tạo Khóa và IV (Key & IV Setup)
- **Key Setup**: Khóa 128-bit được chia thành 8 khóa con 16-bit $K_0 \dots K_7$ theo chuẩn chuyển đổi Big-Endian (OS2IP). Sau đó khởi tạo $X_j, C_j$, chạy 4 vòng lặp Next-State khởi động và tái cấu hình $C_j \leftarrow C_j \oplus X_{(j+4 \bmod 8)}$. Trạng thái này được lưu thành **Master State**.
- **IV Setup**: Khôi phục từ Master State, trộn 64-bit IV vào $C_0 \dots C_7$, sau đó lặp 4 vòng Next-State khởi động.

### 2.5. Trích xuất Keystream (Extraction Scheme)
Mỗi vòng sinh ra khối 128-bit keystream $S[127..0]$ thông qua:
$$\begin{aligned}
S[15..0] &= X_0[15..0] \oplus X_5[31..16] & S[31..16] &= X_0[31..16] \oplus X_3[15..0] \\
S[47..32] &= X_2[15..0] \oplus X_7[31..16] & S[63..48] &= X_2[31..16] \oplus X_5[15..0] \\
S[79..64] &= X_4[15..0] \oplus X_1[31..16] & S[95..80] &= X_4[31..16] \oplus X_7[15..0] \\
S[111..96] &= X_6[15..0] \oplus X_3[31..16] & S[127..112] &= X_6[31..16] \oplus X_1[15..0]
\end{aligned}$$
Sau đó đóng gói thành 16 byte theo thứ tự I2OSP Big-Endian ($S[127..112]$ ở đầu, $S[15..0]$ ở cuối).

---

## 3. Kiến trúc thư mục và Thiết kế module

Dự án được phân cấp chuyên nghiệp nhằm tuân thủ nguyên tắc **Separation of Concerns (SoC)**:

```
code/
├── include/                 # GIAO DIỆN CHUẨN (Chỉ chứa khai báo .h)
│   ├── rabbit.h             # Khai báo lớp RabbitCipher và trạng thái RabbitState
│   ├── utils.h              # Khai báo các tiện ích bitwise (rotl32, gFunc, hex)
│   └── file_io.h            # Khai báo các hàm đọc/ghi tệp nhị phân
│
├── src/                     # HIỆN THỰC THƯ VIỆN LÕI
│   ├── rabbit.cpp           # Hiện thực Key Setup, IV Setup, Next-State, Block Gen
│   ├── utils.cpp            # Hiện thực chi tiết các hàm tính toán tối ưu
│   └── file_io.cpp          # Hiện thực File I/O nhị phân an toàn
│
├── encrypt/                 # ỨNG DỤNG MÃ HÓA
│   └── encrypt.cpp          # Hàm main() CLI cho chương trình mã hóa
│
├── decrypt/                 # ỨNG DỤNG GIẢI MÃ
│   └── decrypt.cpp          # Hàm main() CLI cho chương trình giải mã
│
├── input/                   # Chứa tệp dữ liệu đầu vào cần thử nghiệm
├── output/                  # Chứa tệp kết quả đầu ra sau mã hóa/giải mã
└── Makefile                 # Pipeline biên dịch tự động hóa
```

---

## 4. Các kỹ thuật tối ưu hóa mã nguồn (Optimizations)

| Kỹ thuật tối ưu | Triển khai trong dự án | So sánh với mã nguồn cơ bản |
| :--- | :--- | :--- |
| **Bitwise Rotation Idiom** | `(x << n) \| (x >> (32 - n))` trong `Utils::rotl32` | Trình biên dịch dịch thẳng thành 1 lệnh máy `rol` (x86) thay vì phép dịch chia modulo. |
| **Masking thay cho Modulo** | `(j & 7)` thay cho `(j % 8)`; `(j & 1)` thay cho `(j % 2)` | Giảm từ 15-20 chu kỳ máy của phép chia số nguyên xuống còn đúng 1 chu kỳ của lệnh `and`. |
| **Carry Ripple không chia** | `temp = C[j] + A[j] + carry; carry = temp >> 32;` | Tính toán trong thanh ghi 64-bit `uint64_t`, lấy cờ nhớ bằng phép dịch `>> 32` không cần lệnh `div`. |
| **Bình phương phi tuyến 64-bit** | `uint64_t sq = sum * sum; sq ^ (sq >> 32)` | Bình phương không tràn, gập 64-bit về 32-bit chỉ tốn 2 lệnh `shr` và `xor`. |
| **Bộ đệm Keystream 16-byte** | Cache `buffer_[16]` trong `RabbitCipher` | Cho phép mã hóa dữ liệu độ dài byte tùy ý mà không lãng phí byte thừa, không cấp phát heap liên tục. |
| **Zero-Allocation File I/O** | `file.tellg()` + `buffer.resize(size)` + đọc 1 khối | Tránh re-allocation nhiều lần khi đọc tệp dung lượng lớn, đạt tốc độ xấp xỉ tốc độ đọc đĩa. |
| **Bộ nhớ Master State** | Lưu `masterState_` sau `setupKey()` | Nạp IV mới chỉ tốn 4 vòng lặp khởi động, không cần tính lại quá trình giải mã khóa ban đầu. |

---

## 5. Kiểm thử tính đúng đắn & Đối chiếu Test Vectors

Dự án hỗ trợ và vượt qua **100% (21/21 test cases)** của cả 2 bộ dữ liệu kiểm chuẩn quốc tế:
1. **Bộ Test Vectors trong bài báo gốc FSE 2003 (tệp PDF `978-3-540-39887-5_23.pdf` Appendix B)**
2. **Bộ Test Vectors chuẩn hóa Internet RFC 4503 (Appendix A)**

> 💡 **Lưu ý kiến trúc (Endianness Note)**:  
> - Trong bài báo gốc FSE 2003 (PDF), các tác giả đặc tả mã nguồn ANSI C cho vi kiến trúc **Little-Endian (x86)**: Khóa và dòng keystream 16-byte được trích xuất theo thứ tự byte Little-Endian ($s[0]$ bắt đầu bằng byte có trọng số thấp nhất `02 F7 4A 1C...`).
> - Trong chuẩn RFC 4503, IETF quy chuẩn sang định dạng **Network Big-Endian (OS2IP / I2OSP)**: Byte có trọng số cao nhất nằm ở đầu ($S[0]$ bắt đầu bằng `B1 57 54 F0...`).  
> Lớp `RabbitCipher` trong dự án hỗ trợ cả 2 chế độ (`EndianMode::RFC4503` và `EndianMode::FSE2003_LE`).

### 5.1. Bảng đối chiếu Test Vectors FSE 2003 (Theo tệp PDF Appendix B)

* **Vector 1**: `Key = 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00`
  - $s[0]$: `02 F7 4A 1C 26 45 6B F5 EC D6 A5 36 F0 54 57 B1` *(Khớp 100%)*
  - $s[1]$: `A7 8A C6 89 47 6C 69 7B 39 0C 9C C5 15 D8 E8 88` *(Khớp 100%)*
  - $s[31]$: `EF 9A 69 71 8B 82 49 A1 A7 3C 5A 6E 5B 90 45 95` *(Khớp 100%)*

* **Vector 2**: `Key = C2 1F CF 38 81 CD 5E E8 62 8A CC B0 A9 89 0D F8`
  - $s[0]$: `3D 02 E0 C7 30 55 91 12 B4 73 B7 90 DE E0 18 DF` *(Khớp 100%)*
  - $s[1]$: `CD 6D 73 0C E5 4E 19 F0 C3 5E C4 79 0E B6 C7 4A` *(Khớp 100%)*
  - $s[31]$: `9F B4 92 E1 B5 40 36 3A E3 83 C0 1F 9F A2 26 1A` *(Khớp 100%)*

* **Vector 3**: `Key = 1D 27 2C 6A 2D 8E 3D FC AC 14 05 6B 78 D6 33 A0`
  - $s[0]$: `A3 A9 7A BB 80 39 38 20 B7 E5 0C 4A BB 53 82 3D` *(Khớp 100%)*
  - $s[1]$: `C4 42 37 99 C2 EF C9 FF B3 A4 12 5F 1F 4C 99 A8` *(Khớp 100%)*
  - $s[31]$: `97 C0 73 3F F1 F1 8D 25 6A 59 E2 BA AB C1 F4 F1` *(Khớp 100%)*

### 5.2. Bảng đối chiếu Test Vectors Chuẩn RFC 4503 (Appendix A)

* **Vector A.1.1 (Key = 0, Không IV)**:
  - $S[0]$: `B1 57 54 F0 36 A5 D6 EC F5 6B 45 26 1C 4A F7 02` *(Khớp 100%)*
  - $S[1]$: `88 E8 D8 15 C5 9C 0C 39 7B 69 6C 47 89 C6 8A A7` *(Khớp 100%)*
  - $S[2]$: `F4 16 A1 C3 70 0C D4 51 DA 68 D1 88 16 73 D6 96` *(Khớp 100%)*

* **Vector A.1.2 (Key = 91 28 13 29 2E 3D 36 FE 3B FC 62 F1 DC 51 C3 AC)**:
  - $S[0]$: `3D 2D F3 C8 3E F6 27 A1 E9 7F C3 84 87 E2 51 9C` *(Khớp 100%)*
  - $S[1]$: `F5 76 CD 61 F4 40 5B 88 96 BF 53 AA 85 54 FC 19` *(Khớp 100%)*
  - $S[2]$: `E5 54 74 73 FB DB 43 50 8A E5 3B 20 20 4D 4C 5E` *(Khớp 100%)*

* **Vector A.2.2 (Key = 0, IV = C3 73 F5 75 C1 26 7E 59)**:
  - $S[0]$: `1F CD 4E B9 58 00 12 E2 E0 DC CC 92 22 01 7D 6D` *(Khớp 100%)*
  - $S[1]$: `A7 5F 4E 10 D1 21 25 01 7B 24 99 FF ED 93 6F 2E` *(Khớp 100%)*
  - $S[2]$: `EB C1 12 C3 93 E7 38 39 23 56 BD D0 12 02 9B A7` *(Khớp 100%)*

### 5.3. Chạy Kiểm Thử Tự Động (Automated Test Suite)
Nhóm đã tích hợp chương trình kiểm thử tự động toàn diện:
```bash
cd code
make test
# hoặc chạy trực tiếp:
./test_vectors_app.exe
```
Kết quả chạy: **`21 / 21 TESTS PASSED (100.0%)`**.

---

## 6. Đánh giá hiệu năng và Thông lượng (Throughput)

Thực nghiệm đo đạc mã hóa trên môi trường CPU thực tế:
- **Tệp dữ liệu thử nghiệm**: Tài liệu PDF phức hợp (`233,292 bytes` ~ 227.8 KB).
- **Thời gian xử lý mã hóa**: `~1.37 ms`.
- **Thông lượng đạt được (Throughput)**: **~161.57 MB/s**.
- **Kiểm tra tính toàn vẹn dữ liệu (Data Integrity)**:
  - Mã băm SHA-256 tệp gốc: `1624CF03D9763852BDF78C4CEB2C70D4B7AB42D3234445BDDDEFA096C7F6636F`
  - Mã băm SHA-256 tệp giải mã: `1624CF03D9763852BDF78C4CEB2C70D4B7AB42D3234445BDDDEFA096C7F6636F`
  - **Kết luận**: Tệp phục hồi hoàn hảo 100% đến từng bit nhị phân.

---

## 7. Hướng dẫn biên dịch và sử dụng

### 7.1. Yêu cầu môi trường
- Hệ điều hành: Windows / Linux / macOS.
- Trình biên dịch: `g++` hỗ trợ C++11 trở lên (khuyến nghị C++14).
- Công cụ build: `make` (tùy chọn).

### 7.2. Biên dịch dự án
Tại thư mục `code/`:
```bash
# Sử dụng Makefile:
make

# Hoặc biên dịch trực tiếp bằng lệnh g++:
g++ -std=c++14 -O3 -Wall -Wextra -I./include src/rabbit.cpp src/utils.cpp src/file_io.cpp encrypt/encrypt.cpp -o encrypt_app.exe
g++ -std=c++14 -O3 -Wall -Wextra -I./include src/rabbit.cpp src/utils.cpp src/file_io.cpp decrypt/decrypt.cpp -o decrypt_app.exe
```

### 7.3. Hướng dẫn chạy chương trình

#### A. Mã hóa tệp tin (Encrypt)
```bash
# Cách 1: Sử dụng các cờ tham số (Flags)
./encrypt_app.exe -k 912813292E3D36FE3BFC62F1DC51C3AC -iv C373F575C1267E59 -i input/sample.txt -o output/sample.enc

# Cách 2: Tham số vị trí
./encrypt_app.exe input/sample.txt output/sample.enc 912813292E3D36FE3BFC62F1DC51C3AC C373F575C1267E59
```

#### B. Giải mã tệp tin (Decrypt)
```bash
# Sử dụng cờ tham số:
./decrypt_app.exe -k 912813292E3D36FE3BFC62F1DC51C3AC -iv C373F575C1267E59 -i output/sample.enc -o output/sample_dec.txt
```

---

## 8. Hướng dẫn Git & GitHub cho thành viên

Để phục vụ quá trình làm việc nhóm đồng bộ, không ghi đè mã nguồn của nhau và tránh xung đột code (merge conflict), nhóm đã biên soạn tài liệu chi tiết tại:
👉 **[Xem chi tiết Hướng dẫn sử dụng Git & GitHub tại đây (GIT_GUIDE.md)](./GIT_GUIDE.md)**

Tài liệu này bao gồm:
- Hướng dẫn cấu hình danh tính và Token xác thực GitHub.
- Hướng dẫn clone dự án và cập nhật mã nguồn mới nhất (`git pull origin main`).
- Quy trình tạo nhánh tính năng (`feature/branch`), commit chuẩn và tạo Pull Request (PR).
- Các bước xử lý khi xảy ra xung đột mã nguồn (Merge Conflict).
- Bảng lệnh tắt cứu nguy (Git Cheat Sheet).

---

## 9. Kết luận

Dự án đã hoàn thành các mục tiêu đề ra cho bài tập môn học ET3310:
1. Triển khai được thuật toán mã hóa dòng Rabbit tuân thủ chuẩn xác **RFC 4503**.
2. Triển khai kiến trúc thư mục chuẩn mực, cấu trúc mã nguồn C++ sạch và phân tầng rõ ràng.
3. Ứng dụng các kỹ thuật tối ưu hóa bitwise bậc cao giúp chương trình đạt thông lượng tính toán vượt trội trên phần mềm.
4. Đảm bảo độ an toàn tuyệt đối trong việc bảo toàn tính toàn vẹn của các tệp tin dữ liệu nhị phân thực tế.
