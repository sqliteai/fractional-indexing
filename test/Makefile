CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra
SRC_DIR  = ..
TARGET   = test_fractional_indexing

$(TARGET): test.c $(SRC_DIR)/fractional_indexing.c $(SRC_DIR)/fractional_indexing.h
	$(CC) $(CFLAGS) -I$(SRC_DIR) -o $@ test.c $(SRC_DIR)/fractional_indexing.c

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: run clean
