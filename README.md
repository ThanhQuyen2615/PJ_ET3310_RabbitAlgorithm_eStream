# Rabbit Stream Cipher Implementation in C++

![Language](https://img.shields.io/badge/Language-C++11%20or%20higher-blue)
![Type](https://img.shields.io/badge/Type-Stream%20Cipher-red)
![Status](https://img.shields.io/badge/Status-Completed-success)

## 📖 Giới thiệu (Description)

Dự án này là bản triển khai thuật toán mã hóa dòng **Rabbit Stream Cipher** bằng ngôn ngữ **C++**. 

Rabbit là một thuật toán mã hóa luồng tốc độ cao, được thiết kế bởi Martin Boesgaard, Mette Vesterager, Thomas Pedersen, Jesper Christiansen và Ove Scavenius. Thuật toán này nổi bật với hiệu năng tính toán cực nhanh trong môi trường phần mềm và đã xuất sắc lọt vào danh sách các thuật toán chung cuộc của dự án **eSTREAM** (thuộc mạng lưới ECRYPT của Châu Âu).

Dự án được xây dựng với mục đích học thuật, tập trung vào việc mô phỏng chính xác cấu trúc toán học của thuật toán, kiểm chứng tính đúng đắn thông qua các bộ Test Vector chuẩn và đo lường thông lượng mã hóa thực tế trên các tệp dữ liệu.

## 🎯 Mục tiêu dự án
- Triển khai chính xác thuật toán với **Khóa 128-bit** và **Vector khởi tạo (IV) 64-bit**.
- Đảm bảo Keystream sinh ra khớp 100% với Test Vector chuẩn từ bộ tài liệu RFC 4503.
- Cung cấp giao diện dòng lệnh (CLI) để mã hóa và giải mã các tệp dữ liệu thực tế (văn bản, hình ảnh, tài liệu).
- Đo lường và đánh giá tốc độ mã hóa (Throughput Benchmark) trên các kích thước dữ liệu khác nhau.

## 🚀 Tính năng nổi bật (Key Features)
- **Zero-Dependency:** Được viết bằng C++ thuần, không phụ thuộc vào bất kỳ thư viện mật mã hay nền tảng bên thứ ba nào (như OpenSSL).
- **File I/O Support:** Hỗ trợ đọc và xử lý trực tiếp các file nhị phân (Binary stream processing) một cách an toàn mà không làm hỏng cấu trúc file.
- **Tối ưu hóa cơ bản:** Tối ưu các vòng lặp tạo keystream và các phép toán dịch bit (bit-shifting/rotation) cốt lõi của Rabbit.