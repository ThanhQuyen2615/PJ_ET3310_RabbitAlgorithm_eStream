# ET3310 - Mã hóa dòng Rabbit

Bài tập lớn môn ET3310: cài đặt thuật toán Rabbit bằng C++, theo bài báo FSE 2003 (file PDF trong repo) và RFC 4503.

## Thành viên

| STT | Họ và tên | MSSV |
| --- | --- | --- |
| 1 | Đỗ Đức Huy | 20233441 |
| 2 | Đinh Ngọc Hoàng | 20233408 |
| 3 | Bùi Công Thành | 20233643 |
| 4 | Trịnh Minh Quân | 20233599 |
| 5 | Đỗ Trọng Nam | 20233541 |
| 6 | Ngô Anh Quân | 20233595 |
| 7 | Đinh Quang Vinh | 20233716 |

## Thư mục

```
code/
  include/, src/   thuật toán Rabbit (rabbit), hàm phụ (utils), đọc/ghi file (file_io)
  encrypt/         chương trình mã hóa
  decrypt/         chương trình giải mã
  tests/           các chương trình test
  input/           file mẫu để thử
  output/          kết quả và file log
```

## Build và chạy

Trong thư mục `code/`:

```bash
mingw32-make            # build tất cả
mingw32-make test       # chạy test

./encrypt_app.exe input/sample.txt output/sample.enc 912813292E3D36FE3BFC62F1DC51C3AC C373F575C1267E59
./decrypt_app.exe output/sample.enc output/sample_dec.txt 912813292E3D36FE3BFC62F1DC51C3AC C373F575C1267E59
```

Tham số: `<file vào> <file ra> <khóa 32 ký tự hex> [IV 16 ký tự hex]`.

## Log

Các giá trị tính được ở từng bước (X, C, g, keystream) được ghi vào file `.txt`, không in ra màn hình:

- `output/<tên file ra>.log.txt` khi chạy encrypt/decrypt
- `output/test_vectors_log.txt` khi chạy test

## Test vector

Test lấy từ Phụ lục B của bài báo trong file PDF và Phụ lục A của RFC 4503. Bài báo và RFC viết byte theo thứ tự ngược nhau, nên code có 2 chế độ: `ByteOrder::PDF` và `ByteOrder::RFC` (mặc định).

## Hướng dẫn Git

Xem [GIT_GUIDE.md](./GIT_GUIDE.md).
