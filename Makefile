CC=gcc
CFLAGS_LIB=-shared -fPIC -Wall -Wextra
CFLAGS_COMMON=-Wall -Wextra
LDFLAGS=-ldl

SRC_LIB=logger.c
TARGET_LIB=logger.so

SRC_TEST=test.c
TARGET_TEST=test

all: $(TARGET_LIB) $(TARGET_TEST)

$(TARGET_LIB): $(SRC_LIB)
	$(CC) $(CFLAGS_LIB) -o $@ $< $(LDFLAGS)

$(TARGET_TEST): $(SRC_TEST)
	$(CC) $(CFLAGS_COMMON) -o $@ $<

run: all
	@echo "--- Running test with LD_PRELOAD ---"
	LD_PRELOAD=./$(TARGET_LIB) ./$(TARGET_TEST)

clean:
	rm -f $(TARGET_LIB) $(TARGET_TEST) *.o

.PHONY: all run clean