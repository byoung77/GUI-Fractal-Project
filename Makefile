CC := gcc
TARGET := Fractals
SRC := Fractals.c

CFLAGS  := -std=gnu99 -O3 -Wall -Wextra -Wno-deprecated-declarations \
           $(shell pkg-config --cflags gtk+-3.0)

LDLIBS  := $(shell pkg-config --libs gtk+-3.0) -lm -ldl -lX11

all : $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $< -o $@ $(LDLIBS)

clean:
	rm -f $(TARGET) *.o function.c function.so

.PHONY: all clean

