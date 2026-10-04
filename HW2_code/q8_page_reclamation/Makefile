CC=gcc
CFLAGS=-Wall -O2 -pthread
N?=64

all: hw2_page_reclamation

hw2_page_reclamation: hw2_page_reclamation.c
	$(CC) $(CFLAGS) -o $@ $<

run: all
	./hw2_page_reclamation $(N) 20
	./hw2_page_reclamation $(N) 200
	./hw2_page_reclamation $(N) 2000

clean:
	rm -f hw2_page_reclamation
