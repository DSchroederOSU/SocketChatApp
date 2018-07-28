# Makefile for CS 372 Oregon State University Summer 2018

TORM = chatclient
CC = gcc

make:
	$(CC) -o chatclient chatclient.c

clean:
	rm -f chatserve
	rm -f chatclient
	rm -f multi