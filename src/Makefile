CFLAGS = -Wall -Wextra -O3 -mavx -msse2

mandelEXE : mandelbrot.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY : clean
