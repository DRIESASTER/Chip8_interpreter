CC = gcc
CFLAGS = -Wall -Wextra -g $(shell pkg-config --cflags sdl2)
LDFLAGS = $(shell pkg-config --libs sdl2)

SRCS = src/main.c src/chip8.c
OBJDIR = build/obj
OBJS = $(patsubst src/%.c,$(OBJDIR)/%.o,$(SRCS))
TARGET = bin

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/%.o: src/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean:
	rm -rf $(OBJDIR) $(TARGET)
