#
# Makefile para programas teste
#

CC=gcc
LIB_DIR=../lib

all: teste1

teste1: teste1.c $(LIB_DIR)/libcthread.a
	$(CC) -o teste teste1.c -L$(LIB_DIR) -lcthread -Wall
clean:
		rm -rf ./*.o
