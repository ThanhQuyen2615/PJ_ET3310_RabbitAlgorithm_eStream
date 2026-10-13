CXX = g++
CXXFLAGS = -std=c++14 -O3 -Wall -Wextra -I./include

SRCS_CORE = src/rabbit.cpp src/utils.cpp src/file_io.cpp

TARGET_ENCRYPT = encrypt_app
TARGET_DECRYPT = decrypt_app

ifeq ($(OS),Windows_NT)
    TARGET_ENCRYPT := $(TARGET_ENCRYPT).exe
    TARGET_DECRYPT := $(TARGET_DECRYPT).exe
    RM = del /Q /F
else
    RM = rm -f
endif

all: $(TARGET_ENCRYPT) $(TARGET_DECRYPT)

$(TARGET_ENCRYPT): $(SRCS_CORE) encrypt/encrypt.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TARGET_DECRYPT): $(SRCS_CORE) decrypt/decrypt.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	$(RM) $(TARGET_ENCRYPT) $(TARGET_DECRYPT)
