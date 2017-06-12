# Makefile
# Copyright (C) 2016 pexeer@gamil.com
# Wed Jan  4 16:00:01 CST 2017

CXXFLAGS=-std=c++11 -Wall -Wrror -g3
CC = gcc
C++ = g++
LINK = g++


test: main.cpp
	$(CC) -c make_pcontext.S -O2
	$(CC) -c jump_pcontext.S	-O2
	$(C++) -c main.cpp -o main.o
	$(LINK) main.o make_pcontext.o  jump_pcontext.o -o test

clean:
	rm -rf *.o test
