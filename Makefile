CC=clang
CFLAGS=-Wall -Werror
LCUNIT=-lcunit

matrix.o: malloc.c
	$(CC) $(CFLAGS) -o malloc -c $<

test: test.c malloc.o
	$(CC) $(CFLAGS) -o test test.c malloc.o $(LCUNIT) -lm
	./test

resume: test_resume.c malloc.o
	$(CC) $(CFLAGS) -o resume test_resume.c malloc.o $(LCUNIT) -lm
	./resume

.PHONY: clean test resume

clean:
	rm -f test malloc malloc.o resume