CXX = g++
CXXFLAGS = -std=c++14 -O3 -Wall -Wextra -I./include

SRCS_CORE = src/rabbit.cpp src/utils.cpp src/file_io.cpp

# Cac chuong trinh test trong tests/. Them ten vao day khi file .cpp da co ham main().
TESTS = test_vectors test_g_function test_iv_setup test_utils

ifeq ($(OS),Windows_NT)
    EXE = .exe
    RM  = del /Q /F
else
    EXE =
    RM  = rm -f
endif

TARGET_ENCRYPT = encrypt_app$(EXE)
TARGET_DECRYPT = decrypt_app$(EXE)

# Windows coi chuong trinh co chu "setup" trong ten la trinh cai dat va doi quyen
# admin, nen ten chuong trinh test dung "init" thay the (test_iv_init_app).
TEST_APPS = $(addsuffix _app$(EXE),$(subst setup,init,$(TESTS)))

all: $(TARGET_ENCRYPT) $(TARGET_DECRYPT) $(TEST_APPS)

$(TARGET_ENCRYPT): $(SRCS_CORE) encrypt/encrypt.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

$(TARGET_DECRYPT): $(SRCS_CORE) decrypt/decrypt.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

# tests/<name>_setup.cpp -> <name>_init_app
%_init_app$(EXE): $(SRCS_CORE) tests/%_setup.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

# tests/<name>.cpp -> <name>_app
%_app$(EXE): $(SRCS_CORE) tests/%.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

# Build va chay tat ca chuong trinh test; dung lai o loi dau tien.
test: $(addprefix run_,$(TEST_APPS))

run_%: %
	./$<

clean:
	$(RM) $(TARGET_ENCRYPT) $(TARGET_DECRYPT) $(TEST_APPS)

.PHONY: all test clean
