# Makefile for libmrc 

CC = gcc # C compiler
CFLAGS = -Wall -Wextra -O3 -pipe # C flags
#LDFLAGS = -shared  # linking flags
RM = rm -f  # rm command
TARGET_LIB = libmrc.a # target lib

SRCS = src/mrc.c  # source files
OBJS = $(SRCS:.c=.o)

.PHONY: all
all: ${TARGET_LIB}

$(TARGET_LIB): $(OBJS)
	echo "[Link (Static)]"
	ar rcs $@ $^
	#$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	echo "Compile $<"
	$(CC) -c $(CFLAGS) $< -o $@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d)

