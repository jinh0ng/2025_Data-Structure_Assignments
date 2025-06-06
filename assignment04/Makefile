CC = gcc

.c.o: 
	$(CC) -c $<

all: word_count4

word_count4: word_count4.o adt_dlist.o
	$(CC) -o $@ word_count4.o adt_dlist.o
	
clean:
	rm -f *.o
	rm -f word_count4
