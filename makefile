CC=g++

default: make1

make1:
	$(CC) Assignment1.c -o assignment1
	$(CC) S.c -o S
	$(CC) G.c -o G

clean:
	rm -f assignment1
	rm -f S
	rm -f G
