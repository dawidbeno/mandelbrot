CFLAGS = -Wall -Wextra -O3

mandelEXE : mandel.c mandel_avx.o mandel_sse2.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

mandel_avx.o : mandel_avx.c
	$(CC) -c $(CFLAGS) -mavx -o $@ $^

mandel_sse2.o : mandel_sse2.c
	$(CC) -c $(CFLAGS) -msse2 -o $@ $^

clean :
	$(RM) mandel.x86 mandel_avx.o mandel_sse2.o

.PHONY : clean
