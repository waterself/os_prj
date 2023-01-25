CC = gcc
CFLAGS = -g -pthread
SRCS = server.c client.c
OBJS = $(SRCS:.c=.o)
TARGET = $(SRCS:%.c=%)

all: $(TARGET)

clean:
	        rm -f $(TARGET)
dist:
	        tar cvfz chat.tar.gz Makefile $(SRCS)
